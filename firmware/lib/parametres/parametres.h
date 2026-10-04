// =============================================================================
// parametres.h — Paramètres modifiables (dans des bornes figées) + table
// d'étalonnage des sondes. Code portable ; la persistance NVS est dans
// parametres_nvs.cpp (ESP32 uniquement).
// =============================================================================
#pragma once

#include <cstddef>
#include <cstdint>

#include "types_mesures.h"

namespace tv {

constexpr uint16_t VERSION_SCHEMA_PARAMETRES = 2;  // v2 : ajout puissance_max_pct

/// Paramètres de traitement. Toute écriture passe par parametres_borner().
struct Parametres {
    uint16_t version = VERSION_SCHEMA_PARAMETRES;
    float consigne = 0;            // °C, régulée sur la sonde couvain la plus froide
    float hyst_bas = 0;            // °C sous la consigne : chauffe ON
    float hyst_haut = 0;           // °C au-dessus de la consigne : chauffe OFF
    uint32_t duree_palier_min = 0;
    uint32_t timeout_montee_min = 0;
    uint8_t pwm_toit_pct = 0;
    uint8_t pwm_plancher_pct = 0;
    uint16_t rpm_nominal_toit = 0;      // tr/min à 100 % (relevé au banc)
    uint16_t rpm_nominal_plancher = 0;
    uint8_t puissance_max_pct = 0;      // plafond de puissance de l'élément (fenêtre 10 s)
    uint32_t crc = 0;                   // CRC32 de tout ce qui précède
};

/// Valeurs par défaut issues de parametres_defaut.h (CRC calculé).
Parametres parametres_defaut();

/// Ramène chaque champ dans ses bornes figées. Retourne true si un champ a été modifié.
bool parametres_borner(Parametres& p);

/// Calcule et range le CRC.
void parametres_sceller(Parametres& p);

/// Vérifie version + CRC + bornes (aucune correction).
bool parametres_valides(const Parametres& p);

/// CRC32 (IEEE 802.3, réfléchi), portable.
uint32_t crc32(const uint8_t* donnees, size_t n, uint32_t crc_init = 0);

// -----------------------------------------------------------------------------
// Étalonnage des sondes DS18B20 (offset par ID, position physique)
// -----------------------------------------------------------------------------

// enum class Position : défini dans types_mesures.h (propriété de la sonde).

const char* position_texte(Position p);
bool position_depuis_texte(const char* t, Position& p);

/// Offset maximal admis : au-delà, la sonde est suspecte (DS18B20 : ±0,5 °C annoncé).
constexpr float OFFSET_ETALONNAGE_MAX = 1.0f;

struct EntreeEtalonnage {
    uint8_t rom[8] = {0};
    float offset = 0.0f;
    Position position = Position::INCONNUE;
};

struct TableEtalonnage {
    static constexpr uint8_t CAPACITE = 16;
    uint16_t version = VERSION_SCHEMA_PARAMETRES;
    uint8_t n = 0;
    EntreeEtalonnage e[CAPACITE];
    uint32_t crc = 0;

    /// Index de l'entrée pour ce ROM, ou -1.
    int8_t chercher(const uint8_t rom[8]) const;
    /// Ajoute ou met à jour l'offset (et la position si != INCONNUE).
    /// Retourne false si offset hors ±OFFSET_ETALONNAGE_MAX ou table pleine.
    bool definir(const uint8_t rom[8], float offset, Position pos);
    /// Change uniquement la position d'une sonde déjà étalonnée.
    bool definir_position(const uint8_t rom[8], Position pos);
    void vider() { n = 0; }
    void sceller();
    bool valide() const;
};

}  // namespace tv
