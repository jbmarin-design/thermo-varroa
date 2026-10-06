// securite.cpp — voir securite.h
#include "securite.h"

#include "parametres_defaut.h"

namespace tv {

namespace {
// DEF_FILM_SURTEMP en fait partie : l'ouverture de C4 par le MCU fait retomber K2, dont le
// second contact est en série dans le 24 V du film (coupure indépendante d'un MOSFET collé).
constexpr uint32_t DEFAUTS_SURTEMP = DEF_AIR_SURTEMP | DEF_COUVAIN_SURTEMP | DEF_BRUTE_45 | DEF_DUREE_MAX |
                                   DEF_SSR_COLLE | DEF_FILM_SURTEMP;
}

const char* defaut_texte(uint32_t bit) {
    switch (bit) {
        case DEF_AIR_SURTEMP:        return "air_surtemp";
        case DEF_COUVAIN_SURTEMP:    return "couvain_surtemp";
        case DEF_BRUTE_45:           return "brute_45";
        case DEF_PEIGNE_P1:          return "peigne_P1";
        case DEF_PEIGNE_P2:          return "peigne_P2";
        case DEF_PEIGNE_P3:          return "peigne_P3";
        case DEF_SONDE_AIR:          return "sonde_air";
        case DEF_VENTILO_TOIT:       return "ventilo_toit";
        case DEF_VENTILO_PLANCHER_A: return "ventilo_plancher_A";
        case DEF_VENTILO_PLANCHER_B: return "ventilo_plancher_B";
        case DEF_C4_OUVERTE:         return "C4_ouverte";
        case DEF_DUREE_MAX:          return "duree_max";
        case DEF_TIMEOUT_MONTEE:     return "timeout_montee";
        case DEF_PALIER_PERDU:       return "palier_perdu";
        case DEF_PARAMETRES:         return "parametres";
        case DEF_AUTOTEST:           return "autotest";
        case DEF_SSR_COLLE:          return "ssr_colle";
        case DEF_CHAUFFE_INEFFICACE: return "chauffe_inefficace";
        case DEF_HOMOGENEITE:        return "homogeneite";
        case DEF_FILM_SURTEMP:       return "film_surtemp";
        case DEF_SONDE_FILM:         return "sonde_film";
        default:                     return "?";
    }
}

void Securite::reinitialiser() { *this = Securite(); }

void Securite::debut_cycle() {
    chauffe_cumulee_ms_ = 0;
    ssr_off_ms_ = 0;
    air_min_off_ = 1000.0f;
    refroid_ms_ = 0;
    couvain_min_refroid_ = 1000.0f;
    for (uint8_t v = 0; v < NB_VENTILOS; ++v) { ventilo_bas_ms_[v] = 0; ventilo_depuis_ms_[v] = 0; pwm_prec_[v] = 0; }
}

bool Securite::ventilo_ok(uint16_t rpm, uint8_t pwm_pct, uint16_t rpm_nominal) {
    if (pwm_pct == 0) return true;  // ventilateur volontairement arrêté
    // Vitesse attendue ~ proportionnelle au PWM [H] (à recaler au banc : courbe réelle relevée).
    const float attendu = static_cast<float>(rpm_nominal) * static_cast<float>(pwm_pct) / 100.0f;
    return static_cast<float>(rpm) >= defauts::SEUIL_TACHY_FRACTION * attendu;
}

void Securite::evaluer(const Mesures& m, const ContexteSecurite& c, uint32_t maintenant_ms) {
    const uint32_t dt = premier_ ? 0u : (maintenant_ms - dernier_ms_);
    premier_ = false;
    dernier_ms_ = maintenant_ms;

    // --- Cumul de chauffe effective (toutes phases) -----------------------------
    if (c.ssr_commande) chauffe_cumulee_ms_ += dt;
    if (chauffe_cumulee_ms_ >= defauts::DUREE_CHAUFFE_MAX_MIN * 60u * 1000u) defauts_ |= DEF_DUREE_MAX;

    // --- Surtempérature air soufflé : >= 44,5 °C pendant 10 s ---------------------
    // Évalué en permanence (même hors cycle : un SSR collé chaufferait aussi en ATTENTE).
    if (m.t_air.valide && m.t_air.t >= defauts::T_AIR_DEFAUT) {
        air_haut_ms_ += dt;
        if (air_haut_ms_ >= defauts::T_AIR_DEFAUT_S * 1000u) defauts_ |= DEF_AIR_SURTEMP;
    } else {
        air_haut_ms_ = 0;
    }

    // --- Surtempérature couvain (sonde la plus chaude) : >= 44,0 °C pendant 60 s ---
    if (m.nb_couvain_valides > 0 && m.t_couvain_max >= defauts::T_COEUR_DEFAUT) {
        couvain_haut_ms_ += dt;
        if (couvain_haut_ms_ >= defauts::T_COEUR_DEFAUT_S * 1000u) defauts_ |= DEF_COUVAIN_SURTEMP;
    } else {
        couvain_haut_ms_ = 0;
    }

    // --- Plancher chauffant : surface du film >= 55 °C pendant 10 s -----------------------
    // Évalué en permanence dès que la sonde du film est valide, que la variante soit active
    // ou non (un MOSFET collé chaufferait aussi hors cycle ou variante désactivée).
    if (m.t_film.valide && m.t_film.t >= defauts::T_FILM_DEFAUT) {
        film_haut_ms_ += dt;
        if (film_haut_ms_ >= defauts::T_FILM_DEFAUT_S * 1000u) defauts_ |= DEF_FILM_SURTEMP;
    } else {
        film_haut_ms_ = 0;
    }

    // --- Valeur brute > 45 °C, 3 fois de suite = immédiat ---------------------------
    if (m.t_brute_max > defauts::T_BRUTE_IMMEDIATE) {
        if (brute_45_n_ < 255) ++brute_45_n_;
        if (brute_45_n_ >= defauts::N_BRUTE_IMMEDIATE) defauts_ |= DEF_BRUTE_45;
    } else {
        brute_45_n_ = 0;
    }

    if (!c.cycle_actif) {
        // Hors cycle : la conformité des peignes et des ventilateurs est vérifiée
        // par l'auto-test de départ (machine_etats), pas ici.
        for (uint8_t v = 0; v < NB_VENTILOS; ++v) { ventilo_bas_ms_[v] = 0; ventilo_depuis_ms_[v] = 0; }
        ssr_off_ms_ = 0;
        air_min_off_ = 1000.0f;
        refroid_ms_ = 0;
        couvain_min_refroid_ = 1000.0f;
        return;
    }

    // --- SSR collé (§3.1) ------------------------------------------------------------
    // (a) Air soufflé, soufflante du toit en marche : commande à 0 depuis plus que le délai
    //     de grâce et T air qui REMONTE de plus de 0,5 °C au-dessus de son minimum.
    if (c.ssr_commande) {
        ssr_off_ms_ = 0;
        air_min_off_ = 1000.0f;
    } else {
        ssr_off_ms_ += dt;
        const bool soufflante = c.pwm_pct[VENTILO_TOIT] > 0;
        if (ssr_off_ms_ >= defauts::SSR_COLLE_GRACE_S * 1000u && soufflante && m.t_air.valide) {
            if (m.t_air.t < air_min_off_) air_min_off_ = m.t_air.t;
            if (m.t_air.t > air_min_off_ + defauts::SSR_COLLE_HAUSSE) defauts_ |= DEF_SSR_COLLE;
        } else if (!soufflante) {
            air_min_off_ = 1000.0f;  // ventilateurs arrêtés : l'air stagnant n'est pas représentatif
        }
    }
    // (b) REFROIDISSEMENT : sonde couvain la plus chaude qui remonte alors que la chauffe
    //     n'est PAS commandée. La redescente pilotée peut chauffer pour freiner : toute
    //     commande de chauffe réarme le délai de grâce et le minimum de référence.
    if (c.refroidissement && !c.ssr_commande) {
        refroid_ms_ += dt;
        if (refroid_ms_ >= defauts::REFROID_GRACE_COUVAIN_MIN * 60u * 1000u && m.nb_couvain_valides > 0) {
            if (m.t_couvain_max < couvain_min_refroid_) couvain_min_refroid_ = m.t_couvain_max;
            if (m.t_couvain_max > couvain_min_refroid_ + defauts::REFROID_HAUSSE_COUVAIN) defauts_ |= DEF_SSR_COLLE;
        }
    } else {
        refroid_ms_ = 0;
        couvain_min_refroid_ = 1000.0f;
    }

    // --- Pendant un cycle : peigne débranché / sonde invalide = DÉFAUT immédiat ------
    static const uint32_t code_peigne[NB_EMBASES] = {DEF_PEIGNE_P1, DEF_PEIGNE_P2, DEF_PEIGNE_P3};
    for (uint8_t e = 0; e < NB_EMBASES; ++e) {
        if (!m.embases[e].conforme) defauts_ |= code_peigne[e];
    }
    if (!m.t_air.valide) defauts_ |= DEF_SONDE_AIR;
    // Sonde du film : critique UNIQUEMENT si la variante plancher chauffant est active.
    if (c.plancher_chauffant && !m.t_film.valide) defauts_ |= DEF_SONDE_FILM;

    // --- Chaîne matérielle C4 ouverte pendant le cycle ----------------------------
    if (!m.c4_fermee) defauts_ |= DEF_C4_OUVERTE;

    // --- Ventilateurs : tachymètre < 50 % attendu pendant 10 s ------------------------
    static const uint32_t code_ventilo[NB_VENTILOS] = {DEF_VENTILO_TOIT, DEF_VENTILO_PLANCHER_A,
                                                       DEF_VENTILO_PLANCHER_B};
    for (uint8_t v = 0; v < NB_VENTILOS; ++v) {
        // Délai de démarrage après tout changement de consigne PWM.
        if (c.pwm_pct[v] != pwm_prec_[v]) { ventilo_depuis_ms_[v] = 0; pwm_prec_[v] = c.pwm_pct[v]; }
        ventilo_depuis_ms_[v] += dt;
        if (ventilo_depuis_ms_[v] < defauts::DELAI_DEMARRAGE_VENTILO_S * 1000u) { ventilo_bas_ms_[v] = 0; continue; }
        if (!ventilo_ok(m.rpm[v], c.pwm_pct[v], c.rpm_nominal[v])) {
            ventilo_bas_ms_[v] += dt;
            if (ventilo_bas_ms_[v] >= defauts::SEUIL_TACHY_DUREE_S * 1000u) defauts_ |= code_ventilo[v];
        } else {
            ventilo_bas_ms_[v] = 0;
        }
    }
}

bool Securite::ouvrir_relais_serie() const { return (defauts_ & DEFAUTS_SURTEMP) != 0; }

bool Securite::brassage_requis() const {
    // Surtempérature : garder le brassage pour casser le point chaud près de l'élément,
    // sauf si c'est justement le ventilateur du toit qui est en défaut.
    return (defauts_ & DEFAUTS_SURTEMP) != 0 && (defauts_ & DEF_VENTILO_TOIT) == 0;
}

bool Securite::acquitter(const Mesures& m) {
    uint32_t restant = defauts_;
    // On efface seulement ce dont la cause a disparu à l'instant de l'acquittement.
    if (m.t_air.valide && m.t_air.t < defauts::T_AIR_MAX_REG) restant &= ~static_cast<uint32_t>(DEF_AIR_SURTEMP);
    if (m.nb_couvain_valides > 0 && m.t_couvain_max < defauts::T_COEUR_MAX_REG) {
        restant &= ~static_cast<uint32_t>(DEF_COUVAIN_SURTEMP);
    }
    if (m.t_brute_max <= defauts::T_BRUTE_IMMEDIATE) restant &= ~static_cast<uint32_t>(DEF_BRUTE_45);
    // SSR collé : acquittable seulement une fois l'air ET le couvain revenus sous les limites de
    // régulation ; le SSR DOIT être contrôlé/remplacé avant tout nouveau départ (protocole).
    if (m.t_air.valide && m.t_air.t < defauts::T_AIR_MAX_REG &&
        (m.nb_couvain_valides == 0 || m.t_couvain_max < defauts::T_COEUR_MAX_REG)) {
        restant &= ~static_cast<uint32_t>(DEF_SSR_COLLE);
    }
    if (m.embases[EMBASE_P1].conforme) restant &= ~static_cast<uint32_t>(DEF_PEIGNE_P1);
    if (m.embases[EMBASE_P2].conforme) restant &= ~static_cast<uint32_t>(DEF_PEIGNE_P2);
    if (m.embases[EMBASE_P3].conforme) restant &= ~static_cast<uint32_t>(DEF_PEIGNE_P3);
    if (m.t_air.valide) restant &= ~static_cast<uint32_t>(DEF_SONDE_AIR);
    // Film : acquittable une fois la surface revenue sous la limite de régulation (50 °C).
    if (m.t_film.valide && m.t_film.t < defauts::T_FILM_MAX_REG) restant &= ~static_cast<uint32_t>(DEF_FILM_SURTEMP);
    if (m.t_film.valide) restant &= ~static_cast<uint32_t>(DEF_SONDE_FILM);
    if (m.c4_fermee) restant &= ~static_cast<uint32_t>(DEF_C4_OUVERTE);
    // Ventilateurs, timeouts, palier perdu, auto-test, durée max : constat de fin de cycle,
    // effacés par l'acquittement (le prochain départ refait l'auto-test complet).
    restant &= ~static_cast<uint32_t>(DEF_VENTILO_TOIT | DEF_VENTILO_PLANCHER_A | DEF_VENTILO_PLANCHER_B |
                                      DEF_TIMEOUT_MONTEE | DEF_PALIER_PERDU | DEF_AUTOTEST | DEF_DUREE_MAX |
                                      DEF_CHAUFFE_INEFFICACE | DEF_HOMOGENEITE);
    // DEF_PARAMETRES : jamais effacé ici (il faut réécrire des paramètres valides).
    defauts_ = restant;
    if (defauts_ == DEF_AUCUN) {
        air_haut_ms_ = couvain_haut_ms_ = film_haut_ms_ = 0;
        brute_45_n_ = 0;
        chauffe_cumulee_ms_ = 0;
        ssr_off_ms_ = 0;
        air_min_off_ = 1000.0f;
        refroid_ms_ = 0;
        couvain_min_refroid_ = 1000.0f;
        for (uint8_t v = 0; v < NB_VENTILOS; ++v) { ventilo_bas_ms_[v] = 0; ventilo_depuis_ms_[v] = 0; }
    }
    return defauts_ == DEF_AUCUN;
}

}  // namespace tv
