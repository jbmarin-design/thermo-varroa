// =============================================================================
// brochage.h — Affectation des GPIO du nœud ruche (ESP32-WROOM-32E, Phase 1)
// Référence : hardware/cablage-phase1.md §4 (tableau de brochage).
// Toute modification ici DOIT être reportée dans le schéma de câblage.
// =============================================================================
#pragma once

#include <cstdint>

namespace broche {

// --- Bus 1-Wire : UN bus par embase M8 + un bus interne au toit -------------
constexpr uint8_t OW_P1   = 32;  // embase P1 (peigne centre, 3 sondes) — pull-up 2,2 kΩ externe
constexpr uint8_t OW_P2   = 33;  // embase P2 (peigne bord gauche, 1 sonde)
constexpr uint8_t OW_P3   = 25;  // embase P3 (peigne bord droit, 1 sonde)
constexpr uint8_t OW_TOIT = 4;   // T_air (grille de soufflage) + T_retour (bande d'aspiration),
                                 // câblage interne au toit ; distinguées par leur position d'étalonnage

// --- I2C : SHT45 (0x44) + RTC DS3231 (0x68) ---------------------------------
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;

// --- µSD (SPI VSPI) -----------------------------------------------------------
constexpr uint8_t SD_SCK  = 18;
constexpr uint8_t SD_MISO = 19;
constexpr uint8_t SD_MOSI = 23;
constexpr uint8_t SD_CS   = 5;   // broche de strapping : pull-up 10 kΩ obligatoire (module µSD)

// --- Chauffe -------------------------------------------------------------------
constexpr uint8_t SSR_ENABLE   = 13;  // signal carré 500 Hz (esp_timer 1 ms) -> pompe de charge -> Q3 -> SSR
constexpr uint8_t RELAIS_TRIP  = 17;  // HAUT = le MCU OUVRE la chaîne C4 (ne peut jamais la fermer)
                                      // pull-down 100 kΩ : flottant au boot = pas d'ouverture
constexpr uint8_t C4_ETAT      = 16;  // phototransistor de l'optocoupleur U3 (PC817) dont la LED est en
                                      // parallèle sur la bobine de K1 ; pull-up 10 kΩ + 10 nF :
                                      // BAS = K1 excité = chaîne C4 armée ; fil coupé = HAUT = « ouverte »

// --- Ventilation ----------------------------------------------------------------
constexpr uint8_t PWM_TOIT        = 26;
constexpr uint8_t PWM_PLANCHER_A  = 27;
constexpr uint8_t PWM_PLANCHER_B  = 14;  // émet un bref signal au boot : sans conséquence sur un ventilateur
constexpr uint8_t TACH_TOIT       = 34;  // entrée seule : pull-up 10 kΩ externe vers 3,3 V + RC
constexpr uint8_t TACH_PLANCHER_A = 35;  // idem
constexpr uint8_t TACH_PLANCHER_B = 15;  // pull-up 10 kΩ (strapping MTDO : HAUT au boot = normal)
constexpr uint8_t ALIM_VENTILOS   = 12;  // HAUT = 12 V appliqué aux 3 soufflantes (Q5 NPN + P-MOSFET Q4).
                                         // Strapping MTDI : DOIT être BAS au boot -> pull-down 100 kΩ
                                         // (jamais de pull-up : le module ne démarrerait plus).
                                         // Nécessaire : un ventilateur 4 fils à PWM 0 % peut continuer
                                         // de tourner (comportement non normalisé).
// GPIO 39 laissé LIBRE : errata ESP32 n°3.11 — avec l'ADC actif (NTC élément sur GPIO 36),
// les entrées 36/39 voient de fausses impulsions : interdit pour un tachymètre de sécurité.

// --- Divers ----------------------------------------------------------------------
constexpr uint8_t NTC_ELEMENT = 36;  // ADC1_CH0 (SENSOR_VP) : NTC élément, DIAGNOSTIC seulement
constexpr uint8_t LED_ETAT    = 2;   // LED RGB adressable WS2812B (1 pixel), sortie seule
constexpr uint8_t BOUTON      = 0;   // bouton départ / acquittement, actif BAS (en parallèle du bouton BOOT)
                                     // appui pendant un reset = mode téléchargement : sans danger

// Étage de commande PWM des ventilateurs 4 fils : transistor collecteur ouvert
// -> un niveau HAUT sur le GPIO tire l'entrée PWM du ventilateur au 0 V.
// Le rapport cyclique est donc inversé côté GPIO.
constexpr bool PWM_VENTILO_INVERSE = true;

}  // namespace broche

// --- Réseau NTC élément (diagnostic, voir cablage-phase1.md) -------------------
namespace ntc_element {
constexpr float R_SERIE_OHM = 47000.0f;   // résistance haute vers 3,3 V, NTC vers 0 V
constexpr float R0_OHM      = 100000.0f;  // NTC 100 kΩ à 25 °C (verre, tenue 250 °C)
constexpr float BETA        = 3950.0f;    // [H] selon référence achetée
constexpr float VCC_MV      = 3300.0f;
}  // namespace ntc_element
