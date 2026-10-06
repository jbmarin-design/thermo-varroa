// regulation.cpp — voir regulation.h
#include "regulation.h"

#include "parametres_defaut.h"

namespace tv {

bool fenetre_puissance(uint32_t t_ms, uint8_t pct) {
    if (pct >= 100) return true;
    if (pct == 0) return false;
    const uint32_t phase = t_ms % defauts::FENETRE_PUISSANCE_MS;
    return phase < (defauts::FENETRE_PUISSANCE_MS / 100u) * pct;
}

void RegulationTOR::reinitialiser() {
    tor_ = false;
    lim_couvain_ = false;
    lim_air_ = false;
    sortie_ = false;
    raisons_ = COUPURE_DONNEES;
}

bool RegulationTOR::calculer(const EntreeRegulation& e) {
    raisons_ = COUPURE_AUCUNE;

    if (!e.couvain_valide || !e.air_valide) {
        // Sans mesure fiable : jamais de chauffe. On remet le TOR à 0 pour
        // repartir proprement quand les données reviennent.
        tor_ = false;
        sortie_ = false;
        raisons_ = COUPURE_DONNEES;
        return sortie_;
    }

    // 1) Tout-ou-rien avec hystérésis sur la sonde couvain la plus froide.
    if (e.t_couvain_min <= e.consigne - e.hyst_bas) {
        tor_ = true;
    } else if (e.t_couvain_min >= e.consigne + e.hyst_haut) {
        tor_ = false;
    }
    if (!tor_) raisons_ |= COUPURE_CONSIGNE;

    // 2) Limite couvain (sonde la plus chaude), hystérésis 43,5 / 43,0 °C.
    if (e.t_couvain_max > defauts::T_COEUR_MAX_REG) {
        lim_couvain_ = true;
    } else if (e.t_couvain_max < defauts::T_COEUR_REPRISE) {
        lim_couvain_ = false;
    }
    if (lim_couvain_) raisons_ |= COUPURE_LIMITE_COUVAIN;

    // 3) Limite air soufflé, hystérésis 44,0 / 43,5 °C.
    if (e.t_air > defauts::T_AIR_MAX_REG) {
        lim_air_ = true;
    } else if (e.t_air <= defauts::T_AIR_MAX_REG - defauts::HYST_AIR) {
        lim_air_ = false;
    }
    if (lim_air_) raisons_ |= COUPURE_LIMITE_AIR;

    sortie_ = tor_ && !lim_couvain_ && !lim_air_;
    if (sortie_) raisons_ = COUPURE_AUCUNE;
    return sortie_;
}

// -----------------------------------------------------------------------------

void CommandeFilm::reinitialiser() {
    lim_ = false;
    sortie_ = false;
    raisons_ = FILM_INACTIF;
}

bool CommandeFilm::calculer(const EntreeFilm& e) {
    raisons_ = FILM_AUCUNE;
    if (!e.actif) {
        lim_ = false;
        sortie_ = false;
        raisons_ = FILM_INACTIF;
        return sortie_;
    }
    if (!e.sonde_valide) {
        // Sans mesure de surface : jamais de film. La limite est réarmée par prudence.
        lim_ = true;
        raisons_ |= FILM_DONNEES;
    } else if (e.t_film > defauts::T_FILM_MAX_REG) {
        lim_ = true;
    } else if (e.t_film <= defauts::T_FILM_MAX_REG - defauts::HYST_FILM) {
        lim_ = false;
    }
    if (lim_) raisons_ |= FILM_LIMITE;
    if (!e.soufflantes_ok) raisons_ |= FILM_SOUFFLANTES;
    if (!e.demande) raisons_ |= FILM_DEMANDE;
    sortie_ = raisons_ == FILM_AUCUNE;
    return sortie_;
}

}  // namespace tv
