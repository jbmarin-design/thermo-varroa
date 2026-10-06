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
#define FIRMWARE_VERSION "0.2.0-phase1"  // 0.2 : variante plancher chauffant (D21)

namespace defauts {

// --- Consigne de régulation (tout-ou-rien, Phase 1) --------------------------
constexpr float T_CONSIGNE_PALIER      = 42.3f;  // régulée sur la sonde couvain la PLUS FROIDE
constexpr float T_CONSIGNE_BORNE_MIN   = 41.0f;  // borne figée basse
constexpr float T_CONSIGNE_BORNE_MAX   = 43.5f;  // borne figée haute
constexpr float HYST_BAS               = 0.20f;  // chauffe ON si Tmin <= consigne - HYST_BAS [H]
constexpr float HYST_HAUT              = 0.10f;  // chauffe OFF si Tmin >= consigne + HYST_HAUT [H]
constexpr float HYST_BORNE_MIN         = 0.05f;
constexpr float HYST_BORNE_MAX         = 1.00f;

// --- Plafond de puissance de l'élément (Phase 1, réglage de banc) -------------
// Quand la régulation TOR demande la chauffe, le SSR n'est passant que pendant les
// premiers PUISSANCE_MAX_PCT % de chaque fenêtre de 10 s. 100 % = tout-ou-rien pur.
// Sert à limiter le dépassement de T air soufflé après coupure (marge 44,0 -> 45,0 °C
// faible, inertie de l'élément) sans PID. Ce n'est PAS une régulation : un plafond fixe.
constexpr uint8_t  PUISSANCE_MAX_PCT       = 60;   // [H] départ prudent au banc, à relever (protocole E5)
constexpr uint8_t  PUISSANCE_BORNE_MIN_PCT = 20;
constexpr uint8_t  PUISSANCE_BORNE_MAX_PCT = 100;
constexpr uint32_t FENETRE_PUISSANCE_MS    = 10000;  // SSR zéro-crossing : 500 alternances par fenêtre

// --- Comptage du palier ------------------------------------------------------
constexpr float    T_PALIER_MIN            = 42.0f;  // TOUTES les sondes couvain >= ce seuil
constexpr uint32_t STABILITE_ENTREE_PALIER_S = 5u * 60u;  // MONTÉE -> PALIER après 5 min stables
constexpr uint32_t DUREE_PALIER_MIN        = 120;   // minutes cumulées
constexpr uint32_t DUREE_PALIER_BORNE_MIN  = 90;
constexpr uint32_t DUREE_PALIER_BORNE_MAX  = 150;
constexpr uint32_t PALIER_PERDU_MAX_MIN    = 30;    // cumul hors plage en PALIER -> DÉFAUT

// --- Détections de cycle (architecture §2.2) -----------------------------------
// « Chauffe inefficace » (MONTÉE) : chauffe > 80 % du temps sur 20 min avec Tmin couvain
// qui progresse de moins de 0,5 °C -> DÉFAUT.
constexpr uint32_t FENETRE_INEFFICACE_MIN   = 20;
constexpr float    TAUX_CHAUFFE_INEFFICACE  = 0.80f;
constexpr float    PROGRES_MIN_INEFFICACE   = 0.5f;
// « Homogénéité insuffisante » (MONTÉE/PALIER) : la limite couvain (Tmax > 43,5 °C) coupe
// la chauffe alors que Tmin < 42,0 °C, cumulé plus de 30 min -> DÉFAUT. [H]
constexpr uint32_t HOMOGENEITE_MAX_MIN      = 30;

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
// --- Redescente pilotée (REFROIDISSEMENT) ---------------------------------------------
// La consigne descend en rampe depuis la consigne de palier (ou depuis Tmin couvain si
// arrêt opérateur avant le palier) jusqu'à la température de couvain relevée au départ
// du cycle, bornée à [T_RETOUR_MIN ; T_FIN_REFROID]. La chauffe ne sert qu'à FREINER
// le refroidissement (TOR sur la sonde la plus froide) ; les limites C1/C2 restent actives.
// L'hygrométrie de départ est relevée et journalisée (aucun actionneur d'humidité).
constexpr float    PENTE_DESCENTE_C_MIN     = 0.10f;  // °C/min [H] : 42 -> 35 °C en ~70 min
constexpr float    T_RETOUR_MIN             = 33.0f;  // borne basse de la cible (banc à vide : ambiance froide)
constexpr float    T_FIN_REFROID            = 37.0f;  // borne haute de la cible de retour
constexpr float    MARGE_FIN_DESCENTE       = 1.0f;   // FIN quand Tmax couvain <= cible + marge
constexpr uint32_t TIMEOUT_REFROID_MIN      = 180;    // rampe ~70–90 min + inertie
constexpr uint32_t BRASSAGE_REFROID_MIN     = 15;     // (DÉFAUT) brassage après coupure
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
constexpr uint8_t  PWM_PLANCHER_CHAUFFE_PCT = 80; // [H] idem, soufflantes du plancher
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

// --- Variante « plancher chauffant d'appoint » (décision D21) ------------------------------
// Film chauffant 24 V DC 60–80 W [H] collé sur la plaque d'obturation du plancher, SOUS la
// grille (inaccessible aux abeilles). Commande TOR par MOSFET (GPIO FILM_PLANCHER), qui suit la
// MÊME demande que le toit (TOR sur la sonde couvain la plus froide + limites couvain/air), avec
// une limite propre sur la sonde de surface du film. Interdit si les soufflantes de plancher ne
// tournent pas (point chaud). Paramètre `plancher_chauffant` (0/1) : défaut 0 = désactivé.
// Couches matérielles indépendantes du MCU (documentaires ici) : bimétal NF 55 °C + TCO 72 °C en
// série dans le 24 V du film, contact de K2 (chaîne C4) en série.
constexpr uint8_t  PLANCHER_CHAUFFANT_DEFAUT = 0;
constexpr float    T_FILM_MAX_REG    = 50.0f;  // surface du film : au-delà, film coupé (C1) ...
constexpr float    HYST_FILM         = 2.0f;   // ... reprise si T film <= 48,0 °C [H]
constexpr float    T_FILM_DEFAUT     = 55.0f;  // surface du film >= 55 °C ...
constexpr uint32_t T_FILM_DEFAUT_S   = 10;     // ... pendant 10 s = DÉFAUT verrouillé (C2)
constexpr float    T_BIMETAL_FILM    = 55.0f;  // [H] documentaire : bimétal NF, tolérance ±5 K typique
constexpr float    T_TCO_FILM        = 72.0f;  // [H] documentaire : fusible thermique non réarmable

// --- Enable dynamique du SSR (C3) ---------------------------------------------------------
constexpr uint32_t FREQ_ENABLE_SSR_HZ = 500;   // signal carré (basculé en logiciel toutes les 1 ms) exigé par la pompe de charge
constexpr uint32_t JETON_SSR_MAX_AGE_MS = 3000; // la boucle principale doit rafraîchir le jeton : sinon plus de signal

// --- SSR collé (architecture §2.2 REFROIDISSEMENT, §3.1) ------------------------------------
// Commande SSR à 0 depuis plus de SSR_COLLE_GRACE_S, soufflante du toit en marche :
// si T air soufflé dépasse de SSR_COLLE_HAUSSE son minimum depuis la coupure -> DÉFAUT.
constexpr uint32_t SSR_COLLE_GRACE_S        = 90;    // [H] inertie de l'élément après coupure
constexpr float    SSR_COLLE_HAUSSE         = 0.5f;  // °C
// En REFROIDISSEMENT (ventilateurs éventuellement arrêtés) : sonde couvain la plus chaude.
constexpr uint32_t REFROID_GRACE_COUVAIN_MIN = 10;   // [H] inertie couvain après coupure
constexpr float    REFROID_HAUSSE_COUVAIN    = 0.5f;

// --- Conditions d'utilisation (architecture §2.4) : AVERTISSEMENT seulement en Phase 1 -----
// Phase 1 : pas de sonde d'ambiance dédiée ; on contrôle le SHT45 sous le toit au départ
// (à vide, il lit l'ambiance du local d'essai).
constexpr float T_AMBIANCE_MIN = 12.0f;
constexpr float T_AMBIANCE_MAX = 32.0f;

// --- Bouton -----------------------------------------------------------------------------------
constexpr uint32_t BOUTON_ANTIREBOND_MS = 50;
constexpr uint32_t BOUTON_APPUI_LONG_MS = 3000;  // appui long = arrêt opérateur (MONTÉE/PALIER)

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
static_assert(T_FILM_MAX_REG < T_FILM_DEFAUT, "limite de régulation du film < seuil de défaut");
static_assert(T_FILM_DEFAUT < T_PLAUSIBLE_MAX, "la sonde du film doit rester plausible au seuil de défaut");
static_assert(T_FILM_DEFAUT < T_TCO_FILM, "le défaut logiciel du film doit précéder le TCO");
static_assert(HYST_FILM > 0.0f && HYST_FILM < 5.0f, "hystérésis du film incohérente");
static_assert(DUREE_PALIER_BORNE_MAX + TIMEOUT_MONTEE_BORNE_MAX <= DUREE_CHAUFFE_MAX_MIN + 30,
              "durées incompatibles avec la durée max de chauffe");

}  // namespace defauts
