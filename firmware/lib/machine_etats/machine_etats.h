// =============================================================================
// machine_etats.h — Séquence du cycle, version Phase 1 (banc à vide)
//
//   ATTENTE --départ--> AUTOTEST --OK--> MONTEE --Tmin>=42,0 °C 5 min--> PALIER
//   PALIER --temps cumulé >= durée--> REFROIDISSEMENT --Tmax<=37 °C ou timeout--> FIN
//   tout défaut (securite) -> DEFAUT (verrouillé, acquittement local)
//   arrêt opérateur (appui long) en MONTEE/PALIER -> REFROIDISSEMENT
//
// AUTOTEST est un état transitoire (~8 s) ajouté à la machine de l'architecture
// pour faire tourner les ventilateurs et contrôler leurs tachymètres avant de chauffer.
//
// Simplifications Phase 1 (complétées en Phase 2) : pas de rampe de consigne
// (le TOR vise directement la consigne : la montée est limitée par la puissance
// et par la limite air soufflé), pas de FRAM ni de reprise après coupure (tout
// redémarrage revient en ATTENTE), pas de détection « chauffe inefficace » ni
// « SSR collé » automatisée (observées et notées dans le protocole de test).
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

/// Commandes opérateur (bouton local ou console série locale).
struct Commandes {
    bool depart = false;   // appui court en ATTENTE
    bool arret = false;    // appui long en MONTEE/PALIER
    bool acquit = false;   // appui court en FIN ou DEFAUT
};

/// Sorties de la machine vers le matériel.
struct Sorties {
    bool chauffe = false;                    // commande SSR effective (régulation ET sécurité ET état)
    uint8_t pwm_pct[NB_VENTILOS] = {0, 0, 0};
    bool ouvrir_relais_serie = false;        // le MCU ouvre la chaîne C4
};

/// Résultat de l'auto-test de départ (bits, journalisés).
enum CodeAutotest : uint32_t {
    AT_OK                = 0,
    AT_PEIGNE_P1         = 1u << 0,
    AT_PEIGNE_P2         = 1u << 1,
    AT_PEIGNE_P3         = 1u << 2,
    AT_SONDE_AIR         = 1u << 3,
    AT_NON_ETALONNEE     = 1u << 4,   // une sonde couvain ou air sans offset d'étalonnage
    AT_COUVAIN_HORS_PLAGE = 1u << 5,  // T couvain hors [15 ; 39] °C
    AT_C4_OUVERTE        = 1u << 6,
    AT_VENTILO_TOIT      = 1u << 7,
    AT_VENTILO_PLANCHER_A = 1u << 8,
    AT_VENTILO_PLANCHER_B = 1u << 9,
    AT_PARAMETRES        = 1u << 10,
    AT_DEFAUT_PRESENT    = 1u << 11,
};

class MachineEtats {
public:
    void initialiser(const Parametres& p, bool parametres_ok);

    /// Un pas de la machine, à chaque nouvelle mesure (2 s).
    Sorties pas(const Mesures& m, const Commandes& cmd, Securite& secu, uint32_t maintenant_ms);

    Etat etat() const { return etat_; }
    const Parametres& parametres() const { return p_; }
    /// Change les paramètres (uniquement en ATTENTE). Retourne false sinon.
    bool changer_parametres(const Parametres& p);

    uint32_t palier_cumule_s() const { return palier_cumule_ms_ / 1000u; }
    uint32_t hors_plage_cumule_s() const { return hors_plage_ms_ / 1000u; }
    uint32_t dernier_autotest() const { return autotest_code_; }
    const RegulationTOR& regulation() const { return reg_; }
    /// Vrai si l'état vient de changer au dernier pas (pour journaliser l'événement).
    bool transition() const { return transition_; }
    Etat etat_precedent() const { return etat_prec_; }
    uint32_t debut_cycle_ms() const { return debut_cycle_ms_; }

    /// Auto-test statique (sans ventilateurs) — exposé pour la console et les tests.
    uint32_t autotest_statique(const Mesures& m) const;

private:
    void aller(Etat e, uint32_t maintenant_ms);
    void ventilos(Sorties& s, uint8_t pct_toit, uint8_t pct_plancher) const;

    Parametres p_;
    bool parametres_ok_ = true;
    Etat etat_ = Etat::ATTENTE;
    Etat etat_prec_ = Etat::ATTENTE;
    bool transition_ = false;
    uint32_t entree_etat_ms_ = 0;
    uint32_t dernier_ms_ = 0;
    uint32_t debut_cycle_ms_ = 0;
    uint32_t stable_ms_ = 0;          // MONTEE : durée continue Tmin >= 42,0 °C
    uint32_t palier_cumule_ms_ = 0;   // PALIER : temps cumulé dans la plage
    uint32_t hors_plage_ms_ = 0;      // PALIER : temps cumulé hors plage
    uint32_t autotest_code_ = AT_OK;
    RegulationTOR reg_;
};

}  // namespace tv
