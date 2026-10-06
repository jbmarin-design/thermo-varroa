// =============================================================================
// types_mesures.h — Structures de mesure partagées par tous les modules
// (capteurs -> regulation / securite / machine_etats / journal).
// Code portable : aucune dépendance matérielle (testé sous env native).
// =============================================================================
#pragma once

#include <cstddef>
#include <cstdint>

#include "filtre_median.h"

namespace tv {

constexpr uint8_t NB_EMBASES = 3;             // P1 (centre), P2 (bord G), P3 (bord D)
constexpr uint8_t MAX_SONDES_PAR_EMBASE = 3;
constexpr uint8_t SONDES_ATTENDUES[NB_EMBASES] = {3, 1, 1};
constexpr uint8_t NB_SONDES_COUVAIN = 5;
constexpr uint8_t NB_VENTILOS = 3;

enum Embase : uint8_t { EMBASE_P1 = 0, EMBASE_P2 = 1, EMBASE_P3 = 2 };

/// Position physique d'une sonde, mémorisée par ID dans la table d'étalonnage.
/// (Déplacée ici depuis parametres.h : c'est une propriété de la sonde.)
/// Valeurs stockées dans la table d'étalonnage (NVS) : n'ajouter qu'EN FIN de liste.
enum class Position : uint8_t { INCONNUE = 0, P1_HAUT, P1_CENTRE, P1_BAS, P2, P3, AIR, RETOUR, FILM };

/// Vrai si une sonde de cette position a le droit d'être branchée sur cette embase.
bool position_compatible(Embase e, Position p);
enum VentiloId : uint8_t { VENTILO_TOIT = 0, VENTILO_PLANCHER_A = 1, VENTILO_PLANCHER_B = 2 };

/// Nom court des 5 points couvain, dans l'ordre de temperatures_couvain().
extern const char* const NOMS_COUVAIN[NB_SONDES_COUVAIN];

/// Une sonde DS18B20 et son historique de lecture.
struct LectureSonde {
    uint8_t rom[8] = {0};     // identifiant 1-Wire (traçabilité)
    bool presente = false;    // trouvée lors du dernier balayage du bus
    bool valide = false;      // au moins une valeur ET < 3 échecs consécutifs (absence au balayage = échec)
    bool etalonnee = false;   // un offset d'étalonnage existe pour cet ID
    Position position = Position::INCONNUE;  // position déclarée (table d'étalonnage)
    float offset = 0.0f;      // offset appliqué (°C)
    float t_lue = 0.0f;       // dernière valeur lue SANS offset (étalonnage)
    float t_brute = 0.0f;     // dernière valeur lue + offset (°C), non filtrée
    float t = 0.0f;           // valeur filtrée (médiane 5 points) (°C)
    uint8_t echecs = 0;       // échecs consécutifs (CRC, 85 °C, hors plage, absente)
    FiltreMedian5 filtre;
};

/// Une embase M8 = un bus 1-Wire = un peigne.
struct EtatEmbase {
    bool presence = false;    // impulsion de présence 1-Wire reçue
    uint8_t nb_trouvees = 0;  // sondes trouvées au dernier balayage (peut dépasser MAX)
    uint8_t nb_sondes = 0;    // sondes mémorisées dans sondes[] (<= MAX_SONDES_PAR_EMBASE)
    LectureSonde sondes[MAX_SONDES_PAR_EMBASE];
    bool position_ko = false; // une sonde étalonnée n'a pas la position attendue sur cette embase
    bool conforme = false;    // nb attendu, pas de sonde en trop, toutes valides, positions cohérentes
};

/// Instantané complet des mesures, produit toutes les 2 s.
struct Mesures {
    uint32_t t_ms = 0;
    EtatEmbase embases[NB_EMBASES];
    LectureSonde t_air;       // sonde air soufflé (toit, bande de soufflage) — CRITIQUE
    LectureSonde t_retour;    // sonde air de retour (toit, bande d'aspiration) — diagnostic,
                              // ajoutée en Phase 1 pour le bilan thermique (débit d'air)
    LectureSonde t_film;      // surface du film chauffant du plancher (variante D21) : bus du toit
                              // prolongé par J4 ; critique SEULEMENT si `plancher_chauffant` = 1.
                              // Exclue de t_brute_max (le film travaille normalement à ~50 °C).
    bool t_elem_valide = false;
    float t_elem = 0.0f;      // NTC élément (diagnostic)
    bool sht_valide = false;
    float sht_t = 0.0f;
    float sht_hr = 0.0f;
    uint16_t rpm[NB_VENTILOS] = {0, 0, 0};
    bool c4_fermee = false;   // chaîne de sécurité matérielle fermée (relais C4 collé)

    // --- Grandeurs consolidées (calculées par consolider()) ---
    uint8_t nb_couvain_valides = 0;
    bool couvain_complet = false;  // les 5 sondes couvain valides
    float t_couvain_min = 0.0f;    // sonde la plus froide (régulation, palier)
    float t_couvain_max = 0.0f;    // sonde la plus chaude (limites, défauts)
    float t_brute_max = -1000.0f;  // max des valeurs BRUTES couvain + air (défaut immédiat > 45 °C)
};

/// Calcule les grandeurs consolidées et le statut « conforme » des embases.
void consolider(Mesures& m);

/// Remplit tab[] avec les 5 températures couvain dans l'ordre P1[0..2], P2, P3.
/// Une sonde absente/invalide est signalée par valide[i] = false.
void temperatures_couvain(const Mesures& m, float tab[NB_SONDES_COUVAIN], bool valide[NB_SONDES_COUVAIN]);

/// Fusionne le résultat d'un balayage 1-Wire avec les sondes déjà connues d'un bus.
/// - une sonde retrouvée garde son état (filtre, compteur d'échecs) et est marquée présente ;
/// - une sonde connue absente du balayage est conservée (presente = false) tant que
///   echecs < ECHECS_AVANT_INVALIDE, puis retirée ;
/// - une nouvelle sonde occupe une place libre (les présentes passent en priorité).
/// Retourne le nombre de sondes trouvées (peut dépasser max : peigne erroné / sonde en trop).
uint8_t fusionner_sondes(LectureSonde* slots, uint8_t& nb, uint8_t max, const uint8_t (*roms)[8], uint8_t n_trouves);

/// Trie les sondes d'un bus par position déclarée (P1 : haut, centre, bas), puis par ROM.
void ordonner_sondes(LectureSonde* slots, uint8_t nb);

/// Formate un ROM ID 1-Wire en 16 caractères hexadécimaux (buffer >= 17).
void rom_vers_texte(const uint8_t rom[8], char* texte);

/// Analyse 16 caractères hexadécimaux en ROM ID. Retourne false si invalide.
bool texte_vers_rom(const char* texte, uint8_t rom[8]);

}  // namespace tv
