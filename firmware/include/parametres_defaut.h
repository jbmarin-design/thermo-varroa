// =============================================================================
// parametres_defaut.h — Valeurs par défaut et BORNES FIGÉES à la compilation
// Source : docs/architecture.md §2.2 (paramètres) et §3 (sécurité en couches).
//
// Règle : une valeur « modifiable » ne peut l'être (commande série) qu'à
// l'intérieur des bornes ci-dessous. Les seuils de sécurité (DEFAUT_*, C4)
// ne sont PAS modifiables en fonctionnement : il faut recompiler.
// Les valeurs marquées [H] sont des hypothèses à confirmer au banc (Phase 1).
// =============================================================================
#pragma once

#include <cstdint>

#define FIRMWARE_NOM     "thermo-varroa-noeud"
#define FIRMWARE_VERSION "0.1.0-phase1"

namespace defauts {

// --- Consigne de régulation (tout-ou-rien, Phase 1) --------------------------
constexpr float T_CONSIGNE_PALIER      = 42.3f;  // régulée sur la sonde couvain la PLUS FROIDE
constexpr float T_CONSIGNE_BORNE_MIN   = 41.0f;  // borne figée basse
constexpr float T_CONSIGNE_BORNE_MAX   = 43.5f;  // borne figée haute
constexpr float HYST_BAS               = 0.20f;  // chauffe ON si Tmin <= consigne - HYST_BAS [H]
constexpr float HYST_HAUT              = 0.10f;  // chauffe OFF si Tmin >= consigne + HYST_HAUT [H]
constexpr float HYST_BORNE_MIN         = 0.05f;
constexpr float HYST_BORNE_MAX         = 1.00f;

// --- Comptage du palier ------------------------------------------------------
constexpr float    T_PALIER_MIN            = 42.0f;  // TOUTES les sondes couvain >= ce seuil
constexpr uint32_t STABILITE_ENTREE_PALIER_S = 5u * 60u;  // MONTÉE -> PALIER après 5 min stables
constexpr uint32_t DUREE_PALIER_MIN        = 120;   // minutes cumulées
constexpr uint32_t DUREE_PALIER_BORNE_MIN  = 90;
constexpr uint32_t DUREE_PALIER_BORNE_MAX  = 150;
constexpr uint32_t PALIER_PERDU_MAX_MIN    = 30;    // cumul hors plage en PALIER -> DÉFAUT

// --- Limites de régulation (C1) : coupent la chauffe, non bloquantes ---------
constexpr float T_COEUR_MAX_REG     = 43.5f;  // sonde couvain la plus chaude : au-delà, chauffe = 0
constexpr float T_COEUR_REPRISE     = 43.0f;  // ... jusqu'au retour sous cette valeur
constexpr float T_AIR_MAX_REG       = 44.0f;  // air soufflé : au-delà, chauffe = 0
constexpr float HYST_AIR            = 0.50f;  // reprise si T_air <= 44,0 - 0,5 = 43,5 °C [H]

// --- Seuils de DÉFAUT (C2) : verrouillent, acquittement local ----------------
constexpr float    T_COEUR_DEFAUT       = 44.0f;
constexpr uint32_t T_COEUR_DEFAUT_S     = 60;
constexpr float    T_AIR_DEFAUT         = 44.5f;
constexpr uint32_t T_AIR_DEFAUT_S       = 10;
constexpr float    T_BRUTE_IMMEDIATE    = 45.0f;  // valeur BRUTE > 45 °C ...
constexpr uint8_t  N_BRUTE_IMMEDIATE    = 3;      // ... 3 fois de suite = DÉFAUT immédiat
constexpr float    T_C4_MATERIEL        = 45.0f;  // seuil de la chaîne matérielle (documentaire : non lu par le MCU)

// --- Durées --------------------------------------------------------------------
constexpr uint32_t TIMEOUT_MONTEE_MIN       = 150;
constexpr uint32_t TIMEOUT_MONTEE_BORNE_MIN = 60;
constexpr uint32_t TIMEOUT_MONTEE_BORNE_MAX = 240;   // banc : masses thermiques lourdes [H]
constexpr float    T_FIN_REFROID            = 37.0f;
constexpr uint32_t TIMEOUT_REFROID_MIN      = 90;
constexpr uint32_t BRASSAGE_REFROID_MIN     = 15;
constexpr uint32_t DUREE_CHAUFFE_MAX_MIN    = 6u * 60u;  // absolue, toutes phases

// --- Plausibilité des sondes -----------------------------------------------------
constexpr float   T_PLAUSIBLE_MIN      = -10.0f;
constexpr float   T_PLAUSIBLE_MAX      = 60.0f;
constexpr uint8_t ECHECS_AVANT_INVALIDE = 3;        // lectures KO consécutives
constexpr float   ECART_SONDES_MAX     = 3.0f;      // [H] avertissement seul en Phase 1 (mesuré)
constexpr float   T_COUVAIN_DEPART_MIN = 15.0f;     // auto-test : couvain plausible au départ
constexpr float   T_COUVAIN_DEPART_MAX = 39.0f;

// --- Acquisition / journal ---------------------------------------------------------
constexpr uint32_t PERIODE_ACQUISITION_MS = 2000;
constexpr uint32_t DUREE_CONVERSION_MS    = 800;    // DS18B20 12 bits : 750 ms max
constexpr uint32_t PERIODE_JOURNAL_ATTENTE_S = 60;
constexpr uint32_t PERIODE_JOURNAL_ACTIF_S   = 10;

// --- Ventilation ------------------------------------------------------------------------
constexpr uint8_t  PWM_TOIT_CHAUFFE_PCT = 80;   // [H] à ajuster au banc (débit nécessaire, cf. module-toit)
constexpr uint8_t  PWM_BORNE_MIN_PCT    = 30;
constexpr uint8_t  PWM_BORNE_MAX_PCT    = 100;
constexpr uint8_t  PWM_BRASSAGE_PCT     = 60;
constexpr uint32_t RPM_NOMINAL_TOIT     = 4000; // [H] à 100 %, relever au banc puis corriger
constexpr uint32_t RPM_NOMINAL_PLANCHER = 6000; // [H] idem
constexpr float    SEUIL_TACHY_FRACTION = 0.5f; // < 50 % de la vitesse attendue ...
constexpr uint32_t SEUIL_TACHY_DUREE_S  = 10;   // ... pendant 10 s = DÉFAUT
constexpr uint32_t DELAI_DEMARRAGE_VENTILO_S = 5;
constexpr uint32_t DUREE_TEST_VENTILO_MS = 8000; // test au départ
constexpr uint8_t  IMPULSIONS_PAR_TOUR  = 2;
constexpr uint32_t FREQ_PWM_VENTILO_HZ  = 25000;

// --- Enable dynamique du SSR (C3) ---------------------------------------------------------
constexpr uint32_t FREQ_ENABLE_SSR_HZ = 500;   // signal carré (basculé en logiciel toutes les 1 ms) exigé par la pompe de charge
constexpr uint32_t JETON_SSR_MAX_AGE_MS = 3000; // la boucle principale doit rafraîchir le jeton : sinon plus de signal

// --- Watchdog logiciel -------------------------------------------------------------------------
constexpr uint32_t WATCHDOG_S = 5;

// --- Cohérence des bornes (vérifiée à la compilation) -----------------------------------------
static_assert(T_CONSIGNE_PALIER >= T_CONSIGNE_BORNE_MIN && T_CONSIGNE_PALIER <= T_CONSIGNE_BORNE_MAX,
              "consigne par défaut hors bornes");
static_assert(T_CONSIGNE_BORNE_MAX <= T_COEUR_MAX_REG, "la consigne ne doit jamais dépasser la limite couvain");
static_assert(T_COEUR_MAX_REG < T_COEUR_DEFAUT, "limite de régulation couvain < seuil de défaut");
static_assert(T_AIR_MAX_REG < T_AIR_DEFAUT, "limite de régulation air < seuil de défaut");
static_assert(T_AIR_DEFAUT < T_C4_MATERIEL, "le défaut logiciel doit précéder la coupure matérielle C4");
static_assert(T_PALIER_MIN < T_CONSIGNE_PALIER, "le seuil de palier doit être sous la consigne");
static_assert(T_COEUR_REPRISE < T_COEUR_MAX_REG, "hystérésis couvain incohérente");
static_assert(DUREE_PALIER_MIN >= DUREE_PALIER_BORNE_MIN && DUREE_PALIER_MIN <= DUREE_PALIER_BORNE_MAX,
              "durée de palier hors bornes");
static_assert(DUREE_PALIER_BORNE_MAX + TIMEOUT_MONTEE_BORNE_MAX <= DUREE_CHAUFFE_MAX_MIN + 30,
              "durées incompatibles avec la durée max de chauffe");

}  // namespace defauts
