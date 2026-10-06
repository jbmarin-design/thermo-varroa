// =============================================================================
// machine_etats.h — Séquence du cycle, version Phase 1 (banc à vide)
//
//   ATTENTE --départ + auto-test statique OK--> AUTOTEST (ventilateurs, ~8 s)
//   AUTOTEST --tachymètres OK--> MONTEE --Tmin >= 42,0 °C pendant 5 min--> PALIER
//   PALIER --temps cumulé >= durée--> REFROIDISSEMENT (redescente pilotée en rampe vers la
//   température de couvain du départ) --cible atteinte ou timeout--> FIN
//   tout défaut (securite ou détection de cycle) -> DEFAUT (verrouillé, acquittement local)
//   arrêt opérateur (appui long) en MONTEE/PALIER -> REFROIDISSEMENT ; en AUTOTEST -> ATTENTE
//   FIN --acquittement--> ATTENTE ; DEFAUT --acquittement ET cause disparue--> ATTENTE
//
// AUTOTEST est un état transitoire ajouté à la machine de l'architecture (§2.2)
// pour faire tourner les ventilateurs et contrôler leurs tachymètres avant de chauffer.
//
// Implémenté dès la Phase 1 : timeout de montée, palier cumulatif (Tmin >= 42,0 ET
// Tmax <= 43,5 °C), palier perdu (> 30 min cumulées hors plage), chauffe inefficace,
// homogénéité insuffisante, SSR collé (dans `securite`), durée max de chauffe.
//
// Simplifications Phase 1 (complétées en Phase 2) : pas de rampe de consigne (le
// tout-ou-rien vise directement la consigne : la montée est limitée par la puissance
// et par la limite air soufflé), pas de FRAM ni de reprise après coupure (tout
// redémarrage revient en ATTENTE), cohérence entre sondes en AVERTISSEMENT seulement.
// Code portable (testé en native).
// =============================================================================
#pragma once

#include <cstdint>

#include "parametres.h"
#include "regulation.h"
#include "securite.h"
#include "types_mesures.h"

namespace tv {

enum class Etat : uint8_t { ATTENTE = 0, AUTOTEST, MONTEE, PALIER, REFROIDISSEMENT, FIN, DEFAUT };

const char* etat_texte(Etat e);

/// Commandes opérateur (bouton local ou console série locale). Jamais de commande distante.
struct Commandes {
    bool depart = false;   // appui court en ATTENTE
    bool arret = false;    // appui long en AUTOTEST/MONTEE/PALIER
    bool acquit = false;   // appui court en FIN ou DEFAUT
};

/// Sorties de la machine vers le matériel.
struct Sorties {
    bool chauffe = false;                    // commande SSR effective (régulation ET sécurité ET état)
    uint8_t pwm_pct[NB_VENTILOS] = {0, 0, 0};
    bool ouvrir_relais_serie = false;        // le MCU ouvre la chaîne C4 (jamais il ne la ferme)
};

/// Résultat de l'auto-test de départ (bits, journalisés). Tout bit = départ refusé.
enum CodeAutotest : uint32_t {
    AT_OK                 = 0,
    AT_PEIGNE_P1          = 1u << 0,
    AT_PEIGNE_P2          = 1u << 1,
    AT_PEIGNE_P3          = 1u << 2,
    AT_SONDE_AIR          = 1u << 3,
    AT_NON_ETALONNEE      = 1u << 4,   // une sonde couvain ou air sans offset d'étalonnage
    AT_COUVAIN_HORS_PLAGE = 1u << 5,   // T couvain hors [15 ; 39] °C
    AT_C4_OUVERTE         = 1u << 6,
    AT_VENTILO_TOIT       = 1u << 7,
    AT_VENTILO_PLANCHER_A = 1u << 8,
    AT_VENTILO_PLANCHER_B = 1u << 9,
    AT_PARAMETRES         = 1u << 10,
    AT_DEFAUT_PRESENT     = 1u << 11,
    AT_RTC                = 1u << 12,  // horloge non valide (horodatage scientifique impossible)
};

/// Avertissements (non bloquants, journalisés).
enum CodeAvertissement : uint32_t {
    AV_AUCUN          = 0,
    AV_SD_ABSENTE     = 1u << 0,  // posé par le programme principal
    AV_SHT45          = 1u << 1,  // SHT45 absent ou CRC KO
    AV_AMBIANCE       = 1u << 2,  // T sous le toit hors [12 ; 32] °C au départ
    AV_ECART_SONDES   = 1u << 3,  // Tmax - Tmin > ECART_SONDES_MAX en PALIER
    AV_REFROID_TIMEOUT = 1u << 4, // fin de refroidissement par timeout (décroissance lente)
    AV_LIMITE_COUVAIN = 1u << 5,  // la limite couvain 43,5 °C a coupé la chauffe au moins une fois
    AV_LIMITE_AIR     = 1u << 6,  // la limite air 44,0 °C a coupé la chauffe au moins une fois
    AV_SONDE_RETOUR   = 1u << 7,  // sonde air de retour absente (diagnostic)
};

/// Résumé de cycle (écrit à la FIN ou au DÉFAUT).
struct ResumeCycle {
    uint32_t duree_montee_s = 0;
    uint32_t palier_cumule_s = 0;
    uint32_t hors_plage_s = 0;
    uint32_t duree_cycle_s = 0;
    float tmax_couvain[NB_SONDES_COUVAIN] = {0, 0, 0, 0, 0};
    float tmax_air = 0;
    float ecart_max_palier = 0;   // max(Tmax - Tmin) observé en PALIER
    float t_couvain_init = 0;     // moyenne (Tmin+Tmax)/2 au départ du cycle
    float hr_init = -1;           // HR sous le toit au départ (-1 : SHT45 indisponible)
    float t_retour_cible = 0;     // cible de la redescente pilotée
    uint32_t defauts = 0;
    uint32_t avertissements = 0;
    bool palier_complet = false;
};

class MachineEtats {
public:
    void initialiser(const Parametres& p, bool parametres_ok);

    /// Un pas de la machine, à chaque nouvelle mesure (2 s). Évalue d'abord `secu`
    /// (avec le contexte du pas précédent), puis la séquence. maintenant_ms : horloge monotone.
    Sorties pas(const Mesures& m, const Commandes& cmd, Securite& secu, uint32_t maintenant_ms);

    Etat etat() const { return etat_; }
    const Parametres& parametres() const { return p_; }
    /// Change les paramètres (uniquement en ATTENTE, valeurs bornées). Retourne false sinon.
    bool changer_parametres(const Parametres& p);

    uint32_t palier_cumule_s() const { return palier_cumule_ms_ / 1000u; }
    uint32_t hors_plage_cumule_s() const { return hors_plage_ms_ / 1000u; }
    uint32_t dernier_autotest() const { return autotest_code_; }
    const RegulationTOR& regulation() const { return reg_; }
    /// Vrai si l'état vient de changer au dernier pas (pour journaliser l'événement).
    bool transition() const { return transition_; }
    Etat etat_precedent() const { return etat_prec_; }
    uint32_t debut_cycle_ms() const { return debut_cycle_ms_; }
    uint32_t avertissements() const { return avert_; }
    void declarer_avertissement(uint32_t a) { avert_ |= a; }
    /// Horloge valide (fournie par le programme principal ; vrai par défaut en native).
    void definir_rtc_valide(bool v) { rtc_valide_ = v; }
    const ResumeCycle& resume() const { return resume_; }
    /// Vrai au pas où un résumé de cycle vient d'être finalisé (entrée en FIN ou DEFAUT depuis un cycle).
    bool resume_pret() const { return resume_pret_; }

    /// Auto-test statique (sans ventilateurs) — exposé pour la console et les tests.
    uint32_t autotest_statique(const Mesures& m) const;

    /// Consigne de la redescente pilotée (valide en REFROIDISSEMENT, sinon consigne de palier).
    float consigne_courante() const { return consigne_courante_; }

private:
    void aller(Etat e, uint32_t maintenant_ms);
    void ventilos(Sorties& s, uint8_t pct_toit, uint8_t pct_plancher) const;
    bool regler(const Mesures& m, float consigne);
    void suivre_resume(const Mesures& m);
    void finaliser_resume(uint32_t maintenant_ms, uint32_t defauts);

    Parametres p_;
    bool parametres_ok_ = true;
    bool rtc_valide_ = true;
    Etat etat_ = Etat::ATTENTE;
    Etat etat_prec_ = Etat::ATTENTE;
    bool transition_ = false;
    bool premier_ = true;
    uint32_t entree_etat_ms_ = 0;
    uint32_t dernier_ms_ = 0;
    uint32_t debut_cycle_ms_ = 0;
    uint32_t stable_ms_ = 0;          // MONTEE : durée continue Tmin >= 42,0 °C
    uint32_t palier_cumule_ms_ = 0;   // PALIER : temps cumulé dans la plage
    uint32_t hors_plage_ms_ = 0;      // PALIER : temps cumulé hors plage
    uint32_t homogeneite_ms_ = 0;     // MONTEE+PALIER : limite couvain active alors que Tmin < 42,0 °C
    uint32_t autotest_code_ = AT_OK;
    uint32_t avert_ = AV_AUCUN;
    // Chauffe inefficace : fenêtres successives de 20 min en MONTEE.
    uint32_t fen_ms_ = 0;
    uint32_t fen_chauffe_ms_ = 0;
    float fen_tmin_debut_ = 0;
    // Mémoire du pas précédent (contexte de `securite`).
    bool derniere_chauffe_ = false;
    uint8_t derniers_pwm_[NB_VENTILOS] = {0, 0, 0};
    // Redescente pilotée.
    bool descente_init_ = false;
    float descente_debut_ = 0;        // consigne au début de la rampe
    float consigne_courante_ = 0;
    bool cycle_en_cours_ = false;     // un cycle a démarré et n'a pas encore produit de résumé
    bool resume_pret_ = false;
    ResumeCycle resume_;
    RegulationTOR reg_;
};

}  // namespace tv
