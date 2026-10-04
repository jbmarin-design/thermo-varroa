// =============================================================================
// hal_esp32.cpp — Implémentation matérielle (Arduino-ESP32, cœur 2.0.x ou 3.x)
// Voir brochage.h et hardware/cablage-phase1.md.
// =============================================================================
#include "hal_esp32.h"

#include <OneWire.h>
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_timer.h>
#include <driver/gpio.h>

#include <cmath>
#include <cstring>

#include "brochage.h"
#include "conversions.h"
#include "enable_ssr.h"
#include "parametres_defaut.h"
#include "validation_sonde.h"

namespace hal {

namespace {

// ---------------------------------------------------------------------------------
// Enable dynamique du SSR : esp_timer périodique 1 ms (tâche esp_timer, haute priorité).
// ---------------------------------------------------------------------------------
tv::EnableDynamique g_enable;
esp_timer_handle_t g_timer_enable = nullptr;

void IRAM_ATTR tic_enable(void*) {
    gpio_set_level(static_cast<gpio_num_t>(broche::SSR_ENABLE), g_enable.tic() ? 1 : 0);
}

// ---------------------------------------------------------------------------------
// Tachymètres : compteurs d'impulsions sur front descendant.
// ---------------------------------------------------------------------------------
volatile uint32_t g_impulsions[tv::NB_VENTILOS] = {0, 0, 0};
uint32_t g_tachy_dernier_ms = 0;

void IRAM_ATTR isr_tachy_toit() { ++g_impulsions[tv::VENTILO_TOIT]; }
void IRAM_ATTR isr_tachy_pla() { ++g_impulsions[tv::VENTILO_PLANCHER_A]; }
void IRAM_ATTR isr_tachy_plb() { ++g_impulsions[tv::VENTILO_PLANCHER_B]; }

// ---------------------------------------------------------------------------------
// PWM ventilateurs (LEDC, 25 kHz, 8 bits).
// ---------------------------------------------------------------------------------
const uint8_t PINS_PWM[tv::NB_VENTILOS] = {broche::PWM_TOIT, broche::PWM_PLANCHER_A, broche::PWM_PLANCHER_B};
constexpr uint8_t RESOLUTION_PWM = 8;

void pwm_init() {
    for (uint8_t v = 0; v < tv::NB_VENTILOS; ++v) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcAttach(PINS_PWM[v], defauts::FREQ_PWM_VENTILO_HZ, RESOLUTION_PWM);
#else
        ledcSetup(v, defauts::FREQ_PWM_VENTILO_HZ, RESOLUTION_PWM);
        ledcAttachPin(PINS_PWM[v], v);
#endif
    }
}

void pwm_ecrire(uint8_t v, uint8_t pct) {
    if (pct > 100) pct = 100;
    uint32_t rc = static_cast<uint32_t>(pct) * 255u / 100u;
    if (broche::PWM_VENTILO_INVERSE) rc = 255u - rc;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(PINS_PWM[v], rc);
#else
    ledcWrite(v, rc);
#endif
}

// ---------------------------------------------------------------------------------
// 1-Wire : 3 bus de peignes + 1 bus toit (T_air + T_retour).
// ---------------------------------------------------------------------------------
constexpr uint8_t NB_BUS = 4;
constexpr uint8_t BUS_TOIT = 3;
constexpr uint8_t MAX_SONDES_TOIT = 2;
OneWire g_bus[NB_BUS] = {OneWire(broche::OW_P1), OneWire(broche::OW_P2), OneWire(broche::OW_P3),
                         OneWire(broche::OW_TOIT)};
tv::LectureSonde g_toit[MAX_SONDES_TOIT + 1];
uint8_t g_nb_toit = 0;
uint8_t g_trouvees_toit = 0;

/// Balaye un bus ; retourne le nombre de ROM DS18B20 valides trouvées (max cap).
uint8_t balayer(OneWire& ow, uint8_t (*roms)[8], uint8_t cap, bool& presence) {
    presence = ow.reset() == 1;
    if (!presence) return 0;
    uint8_t n = 0;
    uint8_t rom[8];
    ow.reset_search();
    // Limite dure de 8 itérations : un bus perturbé ne doit pas bloquer la boucle.
    for (uint8_t garde = 0; garde < 8 && ow.search(rom); ++garde) {
        if (OneWire::crc8(rom, 7) != rom[7]) continue;
        if (rom[0] != 0x28) continue;  // famille DS18B20
        if (n < cap) std::memcpy(roms[n], rom, 8);
        ++n;
    }
    return n;
}

void appliquer_etalonnage(tv::LectureSonde& s, const tv::TableEtalonnage& etal) {
    const int8_t i = etal.chercher(s.rom);
    if (i >= 0) {
        s.etalonnee = true;
        s.offset = etal.e[i].offset;
        s.position = etal.e[i].position;
    } else {
        s.etalonnee = false;
        s.offset = 0.0f;
        s.position = tv::Position::INCONNUE;
    }
}

bool lire_scratchpad(OneWire& ow, const uint8_t rom[8], float& t) {
    if (ow.reset() != 1) return false;
    ow.select(rom);
    ow.write(0xBE);
    uint8_t sp[9];
    for (uint8_t i = 0; i < 9; ++i) sp[i] = ow.read();
    return tv::ds18b20_decoder(sp, t);
}

void lancer_conversion(OneWire& ow) {
    if (ow.reset() != 1) return;
    ow.skip();
    ow.write(0x44, 0);  // alimentation externe 3,3 V (pas de mode parasite)
}

// ---------------------------------------------------------------------------------
// I2C : SHT45 (0x44), DS3231 (0x68).
// ---------------------------------------------------------------------------------
constexpr uint8_t ADR_SHT45 = 0x44;
constexpr uint8_t ADR_DS3231 = 0x68;

uint8_t bcd2dec(uint8_t v) { return static_cast<uint8_t>((v >> 4) * 10 + (v & 0x0F)); }
uint8_t dec2bcd(uint8_t v) { return static_cast<uint8_t>(((v / 10) << 4) | (v % 10)); }

// ---------------------------------------------------------------------------------
// µSD
// ---------------------------------------------------------------------------------
bool g_sd_ok = false;
File g_fichier;
char g_nom_fichier[32] = "";
uint16_t g_lignes_depuis_flush = 0;
constexpr uint16_t FLUSH_TOUTES_LES = 6;  // ~1 min en cycle (1 ligne / 10 s)

Preferences g_prefs;

}  // namespace

// =================================================================================
void initialiser() {
    // 1) Sorties de sécurité d'abord : SSR coupé, C4 non forcée, ventilateurs hors tension.
    pinMode(broche::SSR_ENABLE, OUTPUT);
    digitalWrite(broche::SSR_ENABLE, LOW);
    pinMode(broche::RELAIS_TRIP, OUTPUT);
    digitalWrite(broche::RELAIS_TRIP, LOW);
    pinMode(broche::ALIM_VENTILOS, OUTPUT);
    digitalWrite(broche::ALIM_VENTILOS, LOW);

    pinMode(broche::C4_ETAT, INPUT);  // pull-up externe 10 kΩ
    pinMode(broche::BOUTON, INPUT_PULLUP);

    // 2) Enable dynamique.
    esp_timer_create_args_t args = {};
    args.callback = &tic_enable;
    args.name = "enable_ssr";
    args.dispatch_method = ESP_TIMER_TASK;
    if (esp_timer_create(&args, &g_timer_enable) == ESP_OK) esp_timer_start_periodic(g_timer_enable, 1000);

    // 3) Ventilateurs.
    pwm_init();
    for (uint8_t v = 0; v < tv::NB_VENTILOS; ++v) pwm_ecrire(v, 0);
    pinMode(broche::TACH_TOIT, INPUT);
    pinMode(broche::TACH_PLANCHER_A, INPUT);
    pinMode(broche::TACH_PLANCHER_B, INPUT);
    attachInterrupt(digitalPinToInterrupt(broche::TACH_TOIT), isr_tachy_toit, FALLING);
    attachInterrupt(digitalPinToInterrupt(broche::TACH_PLANCHER_A), isr_tachy_pla, FALLING);
    attachInterrupt(digitalPinToInterrupt(broche::TACH_PLANCHER_B), isr_tachy_plb, FALLING);
    g_tachy_dernier_ms = millis();

    // 4) Bus I2C (100 kHz : câbles internes au toit, marge).
    Wire.begin(broche::I2C_SDA, broche::I2C_SCL, 100000);
    Wire.setTimeOut(20);

    // 5) ADC NTC élément.
    analogSetPinAttenuation(broche::NTC_ELEMENT, ADC_11db);
}

// --- Chauffe --------------------------------------------------------------------------
void commande_chauffe(bool chauffe) { g_enable.commande(chauffe); }

void ouvrir_c4(bool ouvrir) { digitalWrite(broche::RELAIS_TRIP, ouvrir ? HIGH : LOW); }

bool c4_fermee() { return digitalRead(broche::C4_ETAT) == LOW; }

// --- Ventilation ------------------------------------------------------------------------
void pwm_ventilos(const uint8_t pct[tv::NB_VENTILOS]) {
    bool un_actif = false;
    for (uint8_t v = 0; v < tv::NB_VENTILOS; ++v) {
        pwm_ecrire(v, pct[v]);
        if (pct[v] > 0) un_actif = true;
    }
    digitalWrite(broche::ALIM_VENTILOS, un_actif ? HIGH : LOW);
}

void lire_tachymetres(uint16_t rpm[tv::NB_VENTILOS]) {
    const uint32_t maintenant = millis();
    const uint32_t duree = maintenant - g_tachy_dernier_ms;
    g_tachy_dernier_ms = maintenant;
    for (uint8_t v = 0; v < tv::NB_VENTILOS; ++v) {
        noInterrupts();
        const uint32_t n = g_impulsions[v];
        g_impulsions[v] = 0;
        interrupts();
        rpm[v] = tv::tachy_rpm(n, duree, defauts::IMPULSIONS_PAR_TOUR);
    }
}

// --- Sondes ---------------------------------------------------------------------------------
void demarrer_conversion(tv::Mesures& m, const tv::TableEtalonnage& etal) {
    uint8_t roms[tv::MAX_SONDES_PAR_EMBASE + 1][8];
    for (uint8_t e = 0; e < tv::NB_EMBASES; ++e) {
        tv::EtatEmbase& eb = m.embases[e];
        bool presence = false;
        const uint8_t n = balayer(g_bus[e], roms, tv::MAX_SONDES_PAR_EMBASE + 1, presence);
        eb.presence = presence;
        const uint8_t n_copies = n < tv::MAX_SONDES_PAR_EMBASE + 1 ? n : tv::MAX_SONDES_PAR_EMBASE + 1;
        eb.nb_trouvees = tv::fusionner_sondes(eb.sondes, eb.nb_sondes, tv::MAX_SONDES_PAR_EMBASE, roms, n_copies);
        eb.nb_trouvees = n;
        for (uint8_t s = 0; s < eb.nb_sondes; ++s) appliquer_etalonnage(eb.sondes[s], etal);
        tv::ordonner_sondes(eb.sondes, eb.nb_sondes);
        lancer_conversion(g_bus[e]);
    }
    bool presence = false;
    const uint8_t n = balayer(g_bus[BUS_TOIT], roms, MAX_SONDES_TOIT + 1, presence);
    const uint8_t n_copies = n < MAX_SONDES_TOIT + 1 ? n : MAX_SONDES_TOIT + 1;
    g_trouvees_toit = n;
    tv::fusionner_sondes(g_toit, g_nb_toit, MAX_SONDES_TOIT, roms, n_copies);
    for (uint8_t s = 0; s < g_nb_toit; ++s) appliquer_etalonnage(g_toit[s], etal);
    lancer_conversion(g_bus[BUS_TOIT]);
}

void lire_conversion(tv::Mesures& m) {
    for (uint8_t e = 0; e < tv::NB_EMBASES; ++e) {
        tv::EtatEmbase& eb = m.embases[e];
        for (uint8_t s = 0; s < eb.nb_sondes; ++s) {
            tv::LectureSonde& l = eb.sondes[s];
            float t = NAN;
            const bool ok = l.presente && lire_scratchpad(g_bus[e], l.rom, t);
            tv::appliquer_lecture(l, ok, t);
        }
    }
    for (uint8_t s = 0; s < g_nb_toit; ++s) {
        tv::LectureSonde& l = g_toit[s];
        float t = NAN;
        const bool ok = l.presente && lire_scratchpad(g_bus[BUS_TOIT], l.rom, t);
        tv::appliquer_lecture(l, ok, t);
    }
    // Affectation air / retour par position d'étalonnage. Une sonde du toit non étalonnée
    // n'est JAMAIS prise comme sonde d'air (l'auto-test refusera le départ : AT_SONDE_AIR).
    m.t_air = tv::LectureSonde();
    m.t_retour = tv::LectureSonde();
    for (uint8_t s = 0; s < g_nb_toit; ++s) {
        if (g_toit[s].position == tv::Position::AIR) m.t_air = g_toit[s];
        if (g_toit[s].position == tv::Position::RETOUR) m.t_retour = g_toit[s];
    }
}

const tv::LectureSonde* sondes_toit(uint8_t& n, uint8_t& trouvees) {
    n = g_nb_toit;
    trouvees = g_trouvees_toit;
    return g_toit;
}

// --- Autres capteurs -------------------------------------------------------------------------
bool lire_sht45(float& t, float& hr) {
    Wire.beginTransmission(ADR_SHT45);
    Wire.write(0xFD);  // mesure haute précision, sans chauffage
    if (Wire.endTransmission() != 0) return false;
    delay(10);
    if (Wire.requestFrom(ADR_SHT45, static_cast<uint8_t>(6)) != 6) return false;
    uint8_t r[6];
    for (uint8_t i = 0; i < 6; ++i) r[i] = static_cast<uint8_t>(Wire.read());
    return tv::sht45_decoder(r, t, hr);
}

bool lire_ntc_element(float& t) {
    uint32_t somme = 0;
    for (uint8_t i = 0; i < 8; ++i) somme += analogReadMilliVolts(broche::NTC_ELEMENT);
    const float mv = static_cast<float>(somme) / 8.0f;
    t = tv::ntc_temperature(mv, ntc_element::VCC_MV, ntc_element::R_SERIE_OHM, ntc_element::R0_OHM,
                            ntc_element::BETA);
    return !std::isnan(t);
}

// --- Horloge -----------------------------------------------------------------------------------
bool rtc_presente() {
    Wire.beginTransmission(ADR_DS3231);
    return Wire.endTransmission() == 0;
}

bool lire_rtc(tv::Horodatage& h) {
    h.valide = false;
    // Registre d'état 0x0F, bit 7 OSF : oscillateur arrêté depuis le dernier réglage -> invalide.
    Wire.beginTransmission(ADR_DS3231);
    Wire.write(0x0F);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(ADR_DS3231, static_cast<uint8_t>(1)) != 1) return false;
    const uint8_t statut = static_cast<uint8_t>(Wire.read());
    Wire.beginTransmission(ADR_DS3231);
    Wire.write(0x00);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(ADR_DS3231, static_cast<uint8_t>(7)) != 7) return false;
    uint8_t r[7];
    for (uint8_t i = 0; i < 7; ++i) r[i] = static_cast<uint8_t>(Wire.read());
    h.seconde = bcd2dec(r[0] & 0x7F);
    h.minute = bcd2dec(r[1] & 0x7F);
    h.heure = bcd2dec(r[2] & 0x3F);  // mode 24 h imposé par regler_rtc()
    h.jour = bcd2dec(r[4] & 0x3F);
    h.mois = bcd2dec(r[5] & 0x1F);
    h.annee = static_cast<uint16_t>(2000 + bcd2dec(r[6]));
    h.valide = (statut & 0x80) == 0 && h.annee >= 2025 && h.mois >= 1 && h.mois <= 12;
    return h.valide;
}

bool regler_rtc(const tv::Horodatage& h) {
    Wire.beginTransmission(ADR_DS3231);
    Wire.write(0x00);
    Wire.write(dec2bcd(h.seconde));
    Wire.write(dec2bcd(h.minute));
    Wire.write(dec2bcd(h.heure));  // bit 6 = 0 : mode 24 h
    Wire.write(1);                 // jour de semaine (non utilisé)
    Wire.write(dec2bcd(h.jour));
    Wire.write(dec2bcd(h.mois));
    Wire.write(dec2bcd(static_cast<uint8_t>(h.annee - 2000)));
    if (Wire.endTransmission() != 0) return false;
    // Efface OSF.
    Wire.beginTransmission(ADR_DS3231);
    Wire.write(0x0F);
    Wire.write(0x00);
    return Wire.endTransmission() == 0;
}

// --- IHM -----------------------------------------------------------------------------------------
bool bouton_appuye() { return digitalRead(broche::BOUTON) == LOW; }

void led(uint8_t r, uint8_t g, uint8_t b) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    rgbLedWrite(broche::LED_ETAT, r, g, b);
#else
    neopixelWrite(broche::LED_ETAT, r, g, b);
#endif
}

// --- µSD -------------------------------------------------------------------------------------------
bool sd_initialiser() {
    SPI.begin(broche::SD_SCK, broche::SD_MISO, broche::SD_MOSI, broche::SD_CS);
    g_sd_ok = SD.begin(broche::SD_CS, SPI, 10000000);
    return g_sd_ok;
}

bool sd_presente() { return g_sd_ok; }

bool sd_ouvrir_fichier(const tv::Horodatage& h) {
    if (!g_sd_ok) return false;
    if (g_fichier) g_fichier.close();
    if (h.valide) {
        snprintf(g_nom_fichier, sizeof g_nom_fichier, "/tv_%04u%02u%02u_%02u%02u%02u.csv", h.annee, h.mois, h.jour,
                 h.heure, h.minute, h.seconde);
    } else {
        // Horloge invalide : numéro séquentiel (le départ est de toute façon refusé, AT_RTC).
        for (uint16_t i = 0; i < 9999; ++i) {
            snprintf(g_nom_fichier, sizeof g_nom_fichier, "/tv_sansrtc_%04u.csv", i);
            if (!SD.exists(g_nom_fichier)) break;
        }
    }
    g_fichier = SD.open(g_nom_fichier, FILE_APPEND);
    g_lignes_depuis_flush = 0;
    if (!g_fichier) g_sd_ok = false;
    return static_cast<bool>(g_fichier);
}

bool sd_ecrire_ligne(const char* ligne, bool forcer_flush) {
    if (!g_sd_ok || !g_fichier) return false;
    const size_t n = std::strlen(ligne);
    bool ok = g_fichier.write(reinterpret_cast<const uint8_t*>(ligne), n) == n;
    ok = ok && g_fichier.write(static_cast<uint8_t>('\n')) == 1;
    if (forcer_flush || ++g_lignes_depuis_flush >= FLUSH_TOUTES_LES) {
        g_fichier.flush();
        g_lignes_depuis_flush = 0;
    }
    if (!ok) {
        g_fichier.close();
        g_sd_ok = false;  // carte retirée ou pleine : avertissement côté main
    }
    return ok;
}

const char* sd_nom_fichier() { return g_nom_fichier; }

// --- NVS -------------------------------------------------------------------------------------------
bool nvs_lire_parametres(tv::Parametres& p) {
    if (!g_prefs.begin("tv", true)) return false;
    const size_t n = g_prefs.getBytes("param", &p, sizeof p);
    g_prefs.end();
    return n == sizeof p && tv::parametres_valides(p);
}

bool nvs_ecrire_parametres(const tv::Parametres& p) {
    if (!g_prefs.begin("tv", false)) return false;
    const size_t n = g_prefs.putBytes("param", &p, sizeof p);
    g_prefs.end();
    return n == sizeof p;
}

bool nvs_lire_etalonnage(tv::TableEtalonnage& t) {
    if (!g_prefs.begin("tv", true)) return false;
    const size_t n = g_prefs.getBytes("etal", &t, sizeof t);
    g_prefs.end();
    return n == sizeof t && t.valide();
}

bool nvs_ecrire_etalonnage(const tv::TableEtalonnage& t) {
    if (!g_prefs.begin("tv", false)) return false;
    const size_t n = g_prefs.putBytes("etal", &t, sizeof t);
    g_prefs.end();
    return n == sizeof t;
}

}  // namespace hal
