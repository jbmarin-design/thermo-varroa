// =============================================================================
// hal_esp32.h — Accès matériel du nœud ruche (ESP32 Arduino uniquement)
// Tout ce qui touche un registre ou une broche est ici ; la logique (sécurité,
// régulation, machine à états, journal) reste dans lib/ et est testée en native.
// =============================================================================
#pragma once

#include <Arduino.h>

#include "journal.h"
#include "parametres.h"
#include "types_mesures.h"

namespace hal {

/// Initialise GPIO, I2C, timers, PWM, tachymètres, ADC. Sorties de chauffe à l'état sûr.
void initialiser();

// --- Chauffe -------------------------------------------------------------------
/// Commande de chauffe voulue (rafraîchit aussi le jeton de l'enable dynamique).
void commande_chauffe(bool chauffe);
/// Le MCU ouvre la chaîne C4 (HAUT sur RELAIS_TRIP). false = ne force rien.
void ouvrir_c4(bool ouvrir);
bool c4_fermee();

// --- Ventilation -----------------------------------------------------------------
void pwm_ventilos(const uint8_t pct[tv::NB_VENTILOS]);
/// Mesure des tachymètres depuis le dernier appel -> tr/min.
void lire_tachymetres(uint16_t rpm[tv::NB_VENTILOS]);

// --- Sondes 1-Wire -----------------------------------------------------------------
/// Lance une conversion sur les 4 bus (balayage ROM inclus). Non bloquant.
void demarrer_conversion(tv::Mesures& m, const tv::TableEtalonnage& etal);
/// Lit les scratchpads (à appeler >= DUREE_CONVERSION_MS après le démarrage).
void lire_conversion(tv::Mesures& m);

/// Sondes du bus interne au toit (air + retour), pour la console d'étalonnage.
/// trouvees = nombre vu au dernier balayage (> 2 : sonde en trop).
const tv::LectureSonde* sondes_toit(uint8_t& n, uint8_t& trouvees);

// --- Autres capteurs -----------------------------------------------------------------
bool lire_sht45(float& t, float& hr);
bool lire_ntc_element(float& t);

// --- Horloge (DS3231) ------------------------------------------------------------------
bool rtc_presente();
bool lire_rtc(tv::Horodatage& h);   // false si absente ou oscillateur arrêté (OSF)
bool regler_rtc(const tv::Horodatage& h);

// --- IHM ----------------------------------------------------------------------------------
bool bouton_appuye();
void led(uint8_t r, uint8_t g, uint8_t b);

// --- µSD -----------------------------------------------------------------------------------
bool sd_initialiser();
bool sd_presente();
/// Ouvre (crée) le fichier de cycle /tv_AAAAMMJJ_HHMMSS.csv ; retourne false si échec.
bool sd_ouvrir_fichier(const tv::Horodatage& h);
/// Ajoute une ligne (avec \n). flush toutes les N lignes et sur demande.
bool sd_ecrire_ligne(const char* ligne, bool forcer_flush = false);
const char* sd_nom_fichier();

// --- NVS (paramètres + étalonnage) ---------------------------------------------------------
bool nvs_lire_parametres(tv::Parametres& p);
bool nvs_ecrire_parametres(const tv::Parametres& p);
bool nvs_lire_etalonnage(tv::TableEtalonnage& t);
bool nvs_ecrire_etalonnage(const tv::TableEtalonnage& t);

}  // namespace hal
