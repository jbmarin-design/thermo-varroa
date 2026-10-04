// =============================================================================
// main.cpp — Nœud ruche, Phase 1 (prototype mono-ruche, banc À VIDE)
//
// Boucle coopérative non bloquante :
//   - toutes les 2 s : balayage + conversion 1-Wire (4 bus), lecture 800 ms plus tard,
//     SHT45, NTC élément, tachymètres, état C4 -> consolider() -> machine.pas() ;
//   - en continu : bouton, console série, enable dynamique du SSR (timer 1 ms) ;
//   - journal CSV : µSD + recopie série (10 s en cycle, 60 s sinon) + événements.
//
// Sécurité : la commande SSR effective = machine (régulation ET sécurité ET état)
// ET plafond de puissance ; elle n'est maintenue que tant que cette boucle
// rafraîchit le jeton de l'enable dynamique. Watchdog de tâche 5 s.
// Les chaînes C4 (45 °C) et C5 (bimétal + TCO) sont indépendantes de ce code.
//
// Console série (115 200 bauds, LOCALE uniquement — aucune commande à distance) :
//   aide | etat | sondes | etal <ROM> <offset> [position] | etalref <ROM> <t_reference>
//   pos <ROM> <position> | etal_effacer | param [nom valeur] | rtc AAAA-MM-JJTHH:MM:SS
//   depart | arret | acquit | silence | bavard
//   test_trip  : le MCU ouvre la chaîne C4 pendant 2 s (contrôle : K1 retombe et RESTE ouvert)
//   test_gel   : fige la boucle principale (contrôle de l'enable dynamique puis du watchdog)
// Positions : P1_haut P1_centre P1_bas P2 P3 air retour.
// =============================================================================
#include <Arduino.h>
#include <esp_task_wdt.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "brochage.h"
#include "hal_esp32.h"
#include "ihm.h"
#include "journal.h"
#include "machine_etats.h"
#include "parametres.h"
#include "parametres_defaut.h"
#include "securite.h"
#include "types_mesures.h"

namespace {

constexpr const char* ID_RUCHE = "proto-1";

tv::Mesures g_m;
tv::Securite g_secu;
tv::MachineEtats g_machine;
tv::Parametres g_param;
tv::TableEtalonnage g_etal;
tv::Bouton g_bouton;
tv::Commandes g_cmd_console;
tv::Sorties g_sorties;
tv::Horodatage g_h;

bool g_conversion_en_cours = false;
uint32_t g_debut_conversion_ms = 0;
uint32_t g_derniere_acquisition_ms = 0;
uint32_t g_dernier_journal_ms = 0;
bool g_echo_serie = true;
bool g_fichier_ouvert = false;
uint32_t g_defauts_journalises = 0;
uint32_t g_avert_journalises = 0;
tv::Appui g_appui_en_attente = tv::Appui::AUCUN;
bool g_chauffe_machine = false;  // décision de la machine au dernier pas (avant plafond de puissance)
uint32_t g_test_trip_fin_ms = 0;  // > 0 : ouverture C4 forcée par la console jusqu'à cet instant

char g_ligne[640];

// -----------------------------------------------------------------------------
void ecrire_ligne(const char* l, bool flush = false, bool echo = true) {
    if (echo && g_echo_serie) Serial.println(l);
    if (hal::sd_presente()) {
        if (!hal::sd_ecrire_ligne(l, flush)) g_machine.declarer_avertissement(tv::AV_SD_ABSENTE);
    }
}

void evenement(const char* code, const char* detail) {
    tv::journal_ligne_evenement(g_h, millis(), g_machine.etat(), code, detail, g_ligne, sizeof g_ligne);
    ecrire_ligne(g_ligne, true);
}

/// Ouvre un nouveau fichier de journal (au boot, puis à chaque départ de cycle) + en-tête.
void ouvrir_journal() {
    g_fichier_ouvert = hal::sd_ouvrir_fichier(g_h);
    if (!g_fichier_ouvert) g_machine.declarer_avertissement(tv::AV_SD_ABSENTE);
    for (uint8_t i = 0;; ++i) {
        if (tv::journal_metadonnees(i, g_machine.parametres(), ID_RUCHE, g_ligne, sizeof g_ligne) == 0) break;
        ecrire_ligne(g_ligne);
    }
    // Table d'étalonnage (traçabilité scientifique : ROM -> offset -> position).
    for (uint8_t i = 0; i < g_etal.n; ++i) {
        char rom[17];
        tv::rom_vers_texte(g_etal.e[i].rom, rom);
        snprintf(g_ligne, sizeof g_ligne, "# etal rom=%s offset=%.3f position=%s", rom,
                 static_cast<double>(g_etal.e[i].offset), tv::position_texte(g_etal.e[i].position));
        ecrire_ligne(g_ligne);
    }
    tv::journal_entete_colonnes(g_ligne, sizeof g_ligne);
    ecrire_ligne(g_ligne, true);
}

void journaliser_mesures() {
    tv::ContexteLigne c;
    c.h = g_h;
    c.t_ms = millis();
    c.etat = g_machine.etat();
    c.consigne = g_machine.parametres().consigne;
    c.chauffe = g_sorties.chauffe;
    c.demande_regulation = g_machine.regulation().sortie();
    c.raisons_coupure = g_machine.regulation().raisons();
    for (uint8_t v = 0; v < tv::NB_VENTILOS; ++v) c.pwm_pct[v] = g_sorties.pwm_pct[v];
    c.defauts = g_secu.defauts();
    c.avertissements = g_machine.avertissements();
    c.palier_cumule_s = g_machine.palier_cumule_s();
    c.hors_plage_s = g_machine.hors_plage_cumule_s();
    c.chauffe_cumulee_s = g_secu.chauffe_cumulee_s();
    c.relais_trip = g_sorties.ouvrir_relais_serie;
    tv::journal_ligne_mesures(g_m, c, g_ligne, sizeof g_ligne);
    ecrire_ligne(g_ligne);
}

// -----------------------------------------------------------------------------
// Console série
// -----------------------------------------------------------------------------
char g_cmd[96];
uint8_t g_cmd_n = 0;

void afficher_sonde(const char* bus, const tv::LectureSonde& s) {
    char rom[17];
    tv::rom_vers_texte(s.rom, rom);
    Serial.printf("  %-6s %s lue=%7.3f offset=%+.3f t=%7.3f etal=%u pos=%s valide=%u echecs=%u\n", bus, rom,
                  static_cast<double>(s.t_lue), static_cast<double>(s.offset), static_cast<double>(s.t),
                  s.etalonnee ? 1u : 0u, tv::position_texte(s.position), s.valide ? 1u : 0u,
                  static_cast<unsigned>(s.echecs));
}

void cmd_sondes() {
    static const char* noms[tv::NB_EMBASES] = {"P1", "P2", "P3"};
    for (uint8_t e = 0; e < tv::NB_EMBASES; ++e) {
        const tv::EtatEmbase& eb = g_m.embases[e];
        Serial.printf("Embase %s : presence=%u trouvees=%u attendues=%u conforme=%u position_ko=%u\n", noms[e],
                      eb.presence ? 1u : 0u, eb.nb_trouvees, tv::SONDES_ATTENDUES[e], eb.conforme ? 1u : 0u,
                      eb.position_ko ? 1u : 0u);
        for (uint8_t s = 0; s < eb.nb_sondes; ++s) afficher_sonde(noms[e], eb.sondes[s]);
    }
    uint8_t n = 0, trouvees = 0;
    const tv::LectureSonde* toit = hal::sondes_toit(n, trouvees);
    Serial.printf("Bus toit : trouvees=%u (attendu 2 : air + retour)\n", trouvees);
    for (uint8_t s = 0; s < n; ++s) afficher_sonde("toit", toit[s]);
    Serial.printf("SHT45 : valide=%u T=%.2f HR=%.1f | NTC element : valide=%u T=%.1f | C4 fermee=%u\n",
                  g_m.sht_valide ? 1u : 0u, static_cast<double>(g_m.sht_t), static_cast<double>(g_m.sht_hr),
                  g_m.t_elem_valide ? 1u : 0u, static_cast<double>(g_m.t_elem), g_m.c4_fermee ? 1u : 0u);
}

/// Cherche une sonde (toutes embases + toit) par ROM ; retourne sa dernière valeur lue sans offset.
bool trouver_sonde(const uint8_t rom[8], float& t_lue) {
    for (uint8_t e = 0; e < tv::NB_EMBASES; ++e) {
        for (uint8_t s = 0; s < g_m.embases[e].nb_sondes; ++s) {
            const tv::LectureSonde& l = g_m.embases[e].sondes[s];
            if (std::memcmp(l.rom, rom, 8) == 0 && l.valide) { t_lue = l.t_lue; return true; }
        }
    }
    uint8_t n = 0, tr = 0;
    const tv::LectureSonde* toit = hal::sondes_toit(n, tr);
    for (uint8_t s = 0; s < n; ++s) {
        if (std::memcmp(toit[s].rom, rom, 8) == 0 && toit[s].valide) { t_lue = toit[s].t_lue; return true; }
    }
    return false;
}

bool enregistrer_etal() {
    g_etal.sceller();
    const bool ok = hal::nvs_ecrire_etalonnage(g_etal);
    Serial.println(ok ? "OK etalonnage enregistre" : "ERREUR ecriture NVS");
    return ok;
}

void cmd_param(char* nom, char* val) {
    tv::Parametres p = g_machine.parametres();
    if (!nom) {
        Serial.printf("consigne=%.2f hyst_bas=%.2f hyst_haut=%.2f palier_min=%lu timeout_montee_min=%lu\n"
                      "pwm_toit=%u pwm_plancher=%u rpm_toit=%u rpm_plancher=%u puissance_max=%u\n",
                      static_cast<double>(p.consigne), static_cast<double>(p.hyst_bas),
                      static_cast<double>(p.hyst_haut), static_cast<unsigned long>(p.duree_palier_min),
                      static_cast<unsigned long>(p.timeout_montee_min), p.pwm_toit_pct, p.pwm_plancher_pct,
                      p.rpm_nominal_toit, p.rpm_nominal_plancher, p.puissance_max_pct);
        return;
    }
    if (g_machine.etat() != tv::Etat::ATTENTE) { Serial.println("REFUS : parametres modifiables en ATTENTE uniquement"); return; }
    if (!val) { Serial.println("usage : param <nom> <valeur>"); return; }
    const float f = static_cast<float>(std::atof(val));
    const long l = std::atol(val);
    if (!std::strcmp(nom, "consigne")) p.consigne = f;
    else if (!std::strcmp(nom, "hyst_bas")) p.hyst_bas = f;
    else if (!std::strcmp(nom, "hyst_haut")) p.hyst_haut = f;
    else if (!std::strcmp(nom, "palier_min")) p.duree_palier_min = static_cast<uint32_t>(l);
    else if (!std::strcmp(nom, "timeout_montee_min")) p.timeout_montee_min = static_cast<uint32_t>(l);
    else if (!std::strcmp(nom, "pwm_toit")) p.pwm_toit_pct = static_cast<uint8_t>(l);
    else if (!std::strcmp(nom, "pwm_plancher")) p.pwm_plancher_pct = static_cast<uint8_t>(l);
    else if (!std::strcmp(nom, "rpm_toit")) p.rpm_nominal_toit = static_cast<uint16_t>(l);
    else if (!std::strcmp(nom, "rpm_plancher")) p.rpm_nominal_plancher = static_cast<uint16_t>(l);
    else if (!std::strcmp(nom, "puissance_max")) p.puissance_max_pct = static_cast<uint8_t>(l);
    else { Serial.println("parametre inconnu"); return; }
    if (tv::parametres_borner(p)) Serial.println("ATTENTION : valeur ramenee dans les bornes figees");
    g_machine.changer_parametres(p);
    hal::nvs_ecrire_parametres(g_machine.parametres());
    char d[48];
    snprintf(d, sizeof d, "%s=%s", nom, val);
    evenement("PARAM", d);
    cmd_param(nullptr, nullptr);
}

void cmd_rtc(const char* t) {
    tv::Horodatage h;
    unsigned a, mo, j, he, mi, s;
    if (!t || std::sscanf(t, "%u-%u-%uT%u:%u:%u", &a, &mo, &j, &he, &mi, &s) != 6) {
        Serial.println("usage : rtc AAAA-MM-JJTHH:MM:SS (heure locale)");
        return;
    }
    h.annee = static_cast<uint16_t>(a); h.mois = static_cast<uint8_t>(mo); h.jour = static_cast<uint8_t>(j);
    h.heure = static_cast<uint8_t>(he); h.minute = static_cast<uint8_t>(mi); h.seconde = static_cast<uint8_t>(s);
    h.valide = true;
    Serial.println(hal::regler_rtc(h) ? "OK RTC reglee" : "ERREUR RTC");
}

void executer_commande(char* ligne) {
    char* mot = std::strtok(ligne, " \t");
    if (!mot) return;
    char* a1 = std::strtok(nullptr, " \t");
    char* a2 = std::strtok(nullptr, " \t");
    char* a3 = std::strtok(nullptr, " \t");

    if (!std::strcmp(mot, "aide")) {
        Serial.println("aide | etat | sondes | etal <ROM> <offset> [pos] | etalref <ROM> <t_ref> | pos <ROM> <pos>\n"
                       "etal_effacer | param [nom val] | rtc AAAA-MM-JJTHH:MM:SS | depart | arret | acquit\n"
                       "test_trip | test_gel | silence | bavard   (pos : P1_haut P1_centre P1_bas P2 P3 air retour)");
    } else if (!std::strcmp(mot, "etat")) {
        char d[200];
        tv::defauts_detail(g_secu.defauts(), d, sizeof d);
        Serial.printf("etat=%s defauts=%s avert=0x%03lX autotest=0x%04lX palier=%lus hors_plage=%lus chauffe=%u\n",
                      tv::etat_texte(g_machine.etat()), d, static_cast<unsigned long>(g_machine.avertissements()),
                      static_cast<unsigned long>(g_machine.dernier_autotest()),
                      static_cast<unsigned long>(g_machine.palier_cumule_s()),
                      static_cast<unsigned long>(g_machine.hors_plage_cumule_s()), g_sorties.chauffe ? 1u : 0u);
    } else if (!std::strcmp(mot, "sondes")) {
        cmd_sondes();
    } else if (!std::strcmp(mot, "etal") || !std::strcmp(mot, "etalref") || !std::strcmp(mot, "pos")) {
        if (g_machine.etat() != tv::Etat::ATTENTE) { Serial.println("REFUS : etalonnage en ATTENTE uniquement"); return; }
        uint8_t rom[8];
        if (!a1 || std::strlen(a1) != 16 || !tv::texte_vers_rom(a1, rom) || !a2) { Serial.println("usage : voir aide"); return; }
        tv::Position pos = tv::Position::INCONNUE;
        bool ok = false;
        if (!std::strcmp(mot, "pos")) {
            if (!tv::position_depuis_texte(a2, pos)) { Serial.println("position inconnue"); return; }
            ok = g_etal.definir_position(rom, pos);
        } else {
            float offset = static_cast<float>(std::atof(a2));
            if (!std::strcmp(mot, "etalref")) {
                float t_lue;
                if (!trouver_sonde(rom, t_lue)) { Serial.println("sonde introuvable ou invalide"); return; }
                offset = static_cast<float>(std::atof(a2)) - t_lue;
                Serial.printf("t_ref=%.3f t_lue=%.3f -> offset=%+.3f\n", std::atof(a2), static_cast<double>(t_lue),
                              static_cast<double>(offset));
            }
            if (a3 && !tv::position_depuis_texte(a3, pos)) { Serial.println("position inconnue"); return; }
            ok = g_etal.definir(rom, offset, pos);
        }
        if (!ok) { Serial.println("REFUS (offset > 1,0 C : sonde suspecte, table pleine ou ROM inconnue)"); return; }
        if (enregistrer_etal()) evenement("ETAL", a1);
    } else if (!std::strcmp(mot, "etal_effacer")) {
        if (g_machine.etat() != tv::Etat::ATTENTE) { Serial.println("REFUS"); return; }
        g_etal.vider();
        enregistrer_etal();
    } else if (!std::strcmp(mot, "param")) {
        cmd_param(a1, a2);
    } else if (!std::strcmp(mot, "rtc")) {
        cmd_rtc(a1);
    } else if (!std::strcmp(mot, "depart")) {
        g_cmd_console.depart = true;
    } else if (!std::strcmp(mot, "arret")) {
        g_cmd_console.arret = true;
    } else if (!std::strcmp(mot, "acquit")) {
        g_cmd_console.acquit = true;
    } else if (!std::strcmp(mot, "test_trip")) {
        // Ouvrir est TOUJOURS permis (sens sûr). Le réarmement reste manuel (bouton RESET C4).
        g_test_trip_fin_ms = millis() + 2000u;
        if (g_test_trip_fin_ms == 0) g_test_trip_fin_ms = 1;
        hal::ouvrir_c4(true);
        evenement("TEST_TRIP", "ouverture C4 par le MCU 2 s");
    } else if (!std::strcmp(mot, "test_gel")) {
        evenement("TEST_GEL", "boucle figee : SSR doit s'ouvrir < 3,5 s, reset watchdog a 5 s");
        Serial.flush();
        for (;;) { /* volontairement bloqué : ni jeton, ni watchdog */ }
    } else if (!std::strcmp(mot, "silence")) {
        g_echo_serie = false;
    } else if (!std::strcmp(mot, "bavard")) {
        g_echo_serie = true;
    } else {
        Serial.println("commande inconnue (aide)");
    }
}

void lire_console() {
    while (Serial.available() > 0) {
        const char c = static_cast<char>(Serial.read());
        if (c == '\r') continue;
        if (c == '\n') {
            g_cmd[g_cmd_n] = '\0';
            executer_commande(g_cmd);
            g_cmd_n = 0;
        } else if (static_cast<size_t>(g_cmd_n) + 1u < sizeof g_cmd) {
            g_cmd[g_cmd_n++] = c;
        }
    }
}

// -----------------------------------------------------------------------------
// Acquisition + pas de la machine (toutes les 2 s)
// -----------------------------------------------------------------------------
void pas_machine(uint32_t maintenant) {
    // Capteurs non 1-Wire.
    g_m.t_ms = maintenant;
    g_m.sht_valide = hal::lire_sht45(g_m.sht_t, g_m.sht_hr);
    g_m.t_elem_valide = hal::lire_ntc_element(g_m.t_elem);
    hal::lire_tachymetres(g_m.rpm);
    g_m.c4_fermee = hal::c4_fermee();
    tv::consolider(g_m);
    g_machine.definir_rtc_valide(hal::lire_rtc(g_h));

    // Commandes : bouton (appui mémorisé entre deux pas) + console.
    tv::Commandes cmd = tv::commandes_depuis_appui(g_appui_en_attente, g_machine.etat());
    g_appui_en_attente = tv::Appui::AUCUN;
    cmd.depart |= g_cmd_console.depart;
    cmd.arret |= g_cmd_console.arret;
    cmd.acquit |= g_cmd_console.acquit;
    g_cmd_console = tv::Commandes();

    const tv::Etat avant = g_machine.etat();
    g_sorties = g_machine.pas(g_m, cmd, g_secu, maintenant);
    g_chauffe_machine = g_sorties.chauffe;
    // Coupure immédiate si la machine ne chauffe plus ; sinon appliquer_chauffe() (loop).
    if (!g_chauffe_machine) hal::commande_chauffe(false);
    hal::ouvrir_c4(g_sorties.ouvrir_relais_serie || g_test_trip_fin_ms != 0);
    hal::pwm_ventilos(g_sorties.pwm_pct);

    // Événements.
    if (g_machine.transition()) {
        if (avant == tv::Etat::AUTOTEST && g_machine.etat() == tv::Etat::MONTEE) ouvrir_journal();
        char d[48];
        snprintf(d, sizeof d, "%s->%s", tv::etat_texte(g_machine.etat_precedent()), tv::etat_texte(g_machine.etat()));
        evenement("TRANSITION", d);
        g_dernier_journal_ms = 0;  // ligne de mesures immédiate
    }
    if (g_machine.dernier_autotest() != tv::AT_OK && g_machine.transition() &&
        g_machine.etat() == tv::Etat::DEFAUT &&
        (avant == tv::Etat::ATTENTE || avant == tv::Etat::AUTOTEST)) {
        char d[24];
        snprintf(d, sizeof d, "0x%04lX", static_cast<unsigned long>(g_machine.dernier_autotest()));
        evenement("AUTOTEST_KO", d);
    }
    if (g_secu.defauts() != g_defauts_journalises) {
        char d[200];
        tv::defauts_detail(g_secu.defauts(), d, sizeof d);
        evenement("DEFAUTS", d);
        g_defauts_journalises = g_secu.defauts();
    }
    if (g_machine.avertissements() != g_avert_journalises) {
        char d[16];
        snprintf(d, sizeof d, "0x%03lX", static_cast<unsigned long>(g_machine.avertissements()));
        evenement("AVERTISSEMENTS", d);
        g_avert_journalises = g_machine.avertissements();
    }
    if (g_machine.resume_pret()) {
        tv::journal_resume(g_h, maintenant, g_machine.resume(), g_ligne, sizeof g_ligne);
        ecrire_ligne(g_ligne, true);
    }

    // Mesures périodiques.
    const tv::Etat e = g_machine.etat();
    const bool actif = e == tv::Etat::AUTOTEST || e == tv::Etat::MONTEE || e == tv::Etat::PALIER ||
                       e == tv::Etat::REFROIDISSEMENT;
    const uint32_t periode = (actif ? defauts::PERIODE_JOURNAL_ACTIF_S : defauts::PERIODE_JOURNAL_ATTENTE_S) * 1000u;
    if (g_dernier_journal_ms == 0 || maintenant - g_dernier_journal_ms >= periode) {
        journaliser_mesures();
        g_dernier_journal_ms = maintenant == 0 ? 1 : maintenant;
    }
}

}  // namespace

// =============================================================================
void setup() {
    hal::initialiser();  // sorties de chauffe à l'état sûr AVANT toute autre chose
    Serial.begin(115200);
    delay(200);
    Serial.printf("\n%s %s — Phase 1, banc a vide. 'aide' pour la console.\n", FIRMWARE_NOM, FIRMWARE_VERSION);

    // Paramètres et étalonnage (NVS + CRC). Corrompus -> défauts compilés + départ refusé.
    const bool param_ok = hal::nvs_lire_parametres(g_param);
    if (!param_ok) g_param = tv::parametres_defaut();
    g_machine.initialiser(g_param, param_ok);
    if (!hal::nvs_lire_etalonnage(g_etal)) {
        g_etal = tv::TableEtalonnage();
        Serial.println("ATTENTION : aucune table d'etalonnage valide (depart refuse tant que les sondes ne sont pas etalonnees)");
    }
    if (!param_ok) {
        Serial.println("ATTENTION : parametres NVS absents/corrompus -> valeurs par defaut, depart refuse.\n"
                       "            Reecrire un parametre (ex. 'param consigne 42.3') pour les valider.");
    }

    g_machine.definir_rtc_valide(hal::lire_rtc(g_h));
    if (!hal::sd_initialiser()) {
        g_machine.declarer_avertissement(tv::AV_SD_ABSENTE);
        Serial.println("AVERTISSEMENT : carte uSD absente (journal serie seulement)");
    }
    ouvrir_journal();
    evenement("DEMARRAGE", FIRMWARE_VERSION);

    // Watchdog de la tâche loop (le timer d'enable s'arrête aussi si la boucle se fige).
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    esp_task_wdt_config_t cfg = {};
    cfg.timeout_ms = defauts::WATCHDOG_S * 1000u;
    cfg.trigger_panic = true;
    esp_task_wdt_reconfigure(&cfg);
#else
    esp_task_wdt_init(defauts::WATCHDOG_S, true);
#endif
    esp_task_wdt_add(nullptr);
}

void loop() {
    const uint32_t maintenant = millis();
    esp_task_wdt_reset();

    // Bouton (toutes les ~10 ms) : un appui est mémorisé jusqu'au prochain pas.
    const tv::Appui a = g_bouton.mettre_a_jour(hal::bouton_appuye(), maintenant);
    if (a != tv::Appui::AUCUN) g_appui_en_attente = a;

    lire_console();
    if (g_test_trip_fin_ms != 0 && static_cast<int32_t>(maintenant - g_test_trip_fin_ms) >= 0) {
        g_test_trip_fin_ms = 0;
        hal::ouvrir_c4(g_sorties.ouvrir_relais_serie);
    }

    // Séquenceur d'acquisition.
    if (!g_conversion_en_cours && maintenant - g_derniere_acquisition_ms >= defauts::PERIODE_ACQUISITION_MS) {
        g_derniere_acquisition_ms = maintenant;
        hal::demarrer_conversion(g_m, g_etal);
        g_debut_conversion_ms = maintenant;
        g_conversion_en_cours = true;
    }
    if (g_conversion_en_cours && maintenant - g_debut_conversion_ms >= defauts::DUREE_CONVERSION_MS) {
        g_conversion_en_cours = false;
        hal::lire_conversion(g_m);
        pas_machine(maintenant);
    }

    // Application de la chauffe à CHAQUE tour de boucle : décision du dernier pas ET plafond
    // de puissance (fenêtre 10 s). Rafraîchit le jeton de l'enable dynamique : si cette boucle
    // se fige, le signal carré s'arrête en JETON_SSR_MAX_AGE_MS et le SSR s'ouvre.
    // Si aucune mesure n'a été traitée depuis 2 périodes d'acquisition, on coupe (données périmées).
    const bool donnees_fraiches = maintenant - g_m.t_ms <= 2u * defauts::PERIODE_ACQUISITION_MS + 500u;
    const bool chauffe = g_chauffe_machine && donnees_fraiches &&
                         tv::fenetre_puissance(maintenant, g_machine.parametres().puissance_max_pct);
    hal::commande_chauffe(chauffe);
    g_sorties.chauffe = chauffe;

    // LED (~1 Hz de clignotement).
    const bool clignote = (maintenant / 500u) % 2u == 0u;
    const bool surtemp = g_sorties.ouvrir_relais_serie;
    const tv::Couleur c = tv::couleur_led(g_machine.etat(), g_machine.avertissements(), surtemp, clignote);
    static uint32_t derniere_led = 0;
    if (maintenant - derniere_led >= 100) {
        hal::led(c.r, c.g, c.b);
        derniere_led = maintenant;
    }
    delay(5);
}
