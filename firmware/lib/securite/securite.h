// =============================================================================
// securite.h — Supervision logicielle (couche C2), version Phase 1
//
// Indépendante de `regulation` et de `machine_etats` : lit les mêmes mesures
// et a le DERNIER MOT sur la sortie chauffe :
//     commande SSR = regulation.sortie() ET securite.autorise_chauffe()
//
// Phase 1 : les défauts ci-dessous sont VERROUILLÉS (acquittement local par
// bouton, seulement si la cause a disparu). La liste complète (sonde figée,
// pente, cohérence, SSR collé, watchdog externe) arrive en Phase 2.
//
// La chaîne matérielle C4 (comparateur 45 °C) et C5 (bimétal + TCO) restent
// TOTALEMENT indépendantes de ce code.
// Code portable (testé en native).
// =============================================================================
#pragma once

#include <cstdint>

#include "types_mesures.h"

namespace tv {

/// Codes de défaut (bits). Journalisés en hexadécimal.
enum CodeDefaut : uint32_t {
    DEF_AUCUN             = 0,
    DEF_AIR_SURTEMP       = 1u << 0,   // T air soufflé >= 44,5 °C pendant 10 s
    DEF_COUVAIN_SURTEMP   = 1u << 1,   // T couvain max >= 44,0 °C pendant 60 s
    DEF_BRUTE_45          = 1u << 2,   // valeur brute > 45 °C, 3 lectures consécutives
    DEF_PEIGNE_P1         = 1u << 3,   // embase P1 non conforme pendant la chauffe
    DEF_PEIGNE_P2         = 1u << 4,
    DEF_PEIGNE_P3         = 1u << 5,
    DEF_SONDE_AIR         = 1u << 6,   // sonde air soufflé invalide
    DEF_VENTILO_TOIT      = 1u << 7,   // tachymètre < 50 % attendu pendant 10 s
    DEF_VENTILO_PLANCHER_A = 1u << 8,
    DEF_VENTILO_PLANCHER_B = 1u << 9,
    DEF_C4_OUVERTE        = 1u << 10,  // chaîne matérielle ouverte pendant un cycle
    DEF_DUREE_MAX         = 1u << 11,  // chauffe active cumulée > 6 h
    DEF_TIMEOUT_MONTEE    = 1u << 12,  // posé par machine_etats
    DEF_PALIER_PERDU      = 1u << 13,  // posé par machine_etats
    DEF_PARAMETRES        = 1u << 14,  // paramètres NVS corrompus (valeurs par défaut chargées)
    DEF_AUTOTEST          = 1u << 15,  // auto-test de départ KO
};

/// Contexte fourni par la machine à états (lecture seule pour securite).
struct ContexteSecurite {
    bool cycle_actif = false;      // MONTÉE ou PALIER ou REFROIDISSEMENT
    bool chauffe_demandee = false; // sortie régulation (pour le cumul de durée)
    bool ssr_commande = false;     // commande effective appliquée au cycle précédent
    uint8_t pwm_pct[NB_VENTILOS] = {0, 0, 0};
    uint16_t rpm_nominal[NB_VENTILOS] = {0, 0, 0};  // tr/min à 100 %
};

class Securite {
public:
    void reinitialiser();

    /// À appeler à chaque nouvelle mesure. maintenant_ms : horloge monotone.
    void evaluer(const Mesures& m, const ContexteSecurite& c, uint32_t maintenant_ms);

    /// Pose un défaut venant d'un autre module (timeout, palier perdu, auto-test).
    void declarer(uint32_t code) { defauts_ |= code; }

    /// Vrai si aucun défaut verrouillé : la chauffe PEUT être autorisée.
    bool autorise_chauffe() const { return defauts_ == DEF_AUCUN; }

    /// Vrai si le MCU doit ouvrir la chaîne C4 (relais série) : défaut de surtempérature.
    bool ouvrir_relais_serie() const;

    /// Vrai si le défaut impose de garder le brassage (surtempérature) plutôt que d'arrêter les ventilateurs.
    bool brassage_requis() const;

    uint32_t defauts() const { return defauts_; }

    /// Acquittement local : efface les défauts dont la cause a disparu.
    /// Retourne true si plus aucun défaut.
    bool acquitter(const Mesures& m);

    /// Durée de chauffe active cumulée (s), pour le journal.
    uint32_t chauffe_cumulee_s() const { return chauffe_cumulee_ms_ / 1000u; }

    /// Vérifie un ventilateur : vrai si le tachymètre est cohérent avec le PWM.
    static bool ventilo_ok(uint16_t rpm, uint8_t pwm_pct, uint16_t rpm_nominal);

private:
    uint32_t defauts_ = DEF_AUCUN;
    uint32_t dernier_ms_ = 0;
    bool premier_ = true;
    uint32_t air_haut_ms_ = 0;
    uint32_t couvain_haut_ms_ = 0;
    uint8_t brute_45_n_ = 0;
    uint32_t ventilo_bas_ms_[NB_VENTILOS] = {0, 0, 0};
    uint32_t ventilo_depuis_ms_[NB_VENTILOS] = {0, 0, 0};
    uint8_t pwm_prec_[NB_VENTILOS] = {0, 0, 0};
    uint32_t chauffe_cumulee_ms_ = 0;
};

}  // namespace tv
