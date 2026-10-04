// machine_etats.cpp — voir machine_etats.h (code portable, testé en native)
#include "machine_etats.h"

#include "parametres_defaut.h"

namespace tv {

namespace {
constexpr uint32_t MIN_MS = 60u * 1000u;

bool etat_chauffant(Etat e) { return e == Etat::MONTEE || e == Etat::PALIER; }
bool etat_cycle(Etat e) { return e == Etat::MONTEE || e == Etat::PALIER || e == Etat::REFROIDISSEMENT; }
}  // namespace

const char* etat_texte(Etat e) {
    switch (e) {
        case Etat::ATTENTE:         return "ATTENTE";
        case Etat::AUTOTEST:        return "AUTOTEST";
        case Etat::MONTEE:          return "MONTEE";
        case Etat::PALIER:          return "PALIER";
        case Etat::REFROIDISSEMENT: return "REFROIDISSEMENT";
        case Etat::FIN:             return "FIN";
        case Etat::DEFAUT:          return "DEFAUT";
        default:                    return "?";
    }
}

void MachineEtats::initialiser(const Parametres& p, bool parametres_ok) {
    *this = MachineEtats();
    p_ = p;
    if (!parametres_ok || parametres_borner(p_)) {
        // Paramètres corrompus ou hors bornes : valeurs compilées + refus de départ (AT_PARAMETRES)
        // jusqu'à ce que des paramètres valides soient réécrits (changer_parametres, en ATTENTE).
        p_ = parametres_defaut();
        parametres_ok_ = false;
    } else {
        parametres_sceller(p_);
        parametres_ok_ = true;
    }
    reg_.reinitialiser();
}

bool MachineEtats::changer_parametres(const Parametres& p) {
    if (etat_ != Etat::ATTENTE) return false;
    Parametres q = p;
    parametres_borner(q);
    parametres_sceller(q);
    p_ = q;
    parametres_ok_ = true;
    return true;
}

uint32_t MachineEtats::autotest_statique(const Mesures& m) const {
    uint32_t c = AT_OK;
    static const uint32_t code_peigne[NB_EMBASES] = {AT_PEIGNE_P1, AT_PEIGNE_P2, AT_PEIGNE_P3};
    for (uint8_t e = 0; e < NB_EMBASES; ++e) {
        if (!m.embases[e].conforme) c |= code_peigne[e];
        for (uint8_t s = 0; s < m.embases[e].nb_sondes; ++s) {
            if (!m.embases[e].sondes[s].etalonnee) c |= AT_NON_ETALONNEE;
        }
    }
    if (!m.t_air.valide) c |= AT_SONDE_AIR;
    else if (!m.t_air.etalonnee) c |= AT_NON_ETALONNEE;
    if (m.couvain_complet &&
        (m.t_couvain_min < defauts::T_COUVAIN_DEPART_MIN || m.t_couvain_max > defauts::T_COUVAIN_DEPART_MAX)) {
        c |= AT_COUVAIN_HORS_PLAGE;
    }
    if (!m.c4_fermee) c |= AT_C4_OUVERTE;
    if (!parametres_ok_) c |= AT_PARAMETRES;
    if (!rtc_valide_) c |= AT_RTC;
    return c;
}

void MachineEtats::aller(Etat e, uint32_t maintenant_ms) {
    if (e == etat_) return;
    etat_prec_ = etat_;
    etat_ = e;
    transition_ = true;
    entree_etat_ms_ = maintenant_ms;
}

void MachineEtats::ventilos(Sorties& s, uint8_t pct_toit, uint8_t pct_plancher) const {
    s.pwm_pct[VENTILO_TOIT] = pct_toit;
    s.pwm_pct[VENTILO_PLANCHER_A] = pct_plancher;
    s.pwm_pct[VENTILO_PLANCHER_B] = pct_plancher;
}

bool MachineEtats::regler(const Mesures& m) {
    EntreeRegulation e;
    bool embases_ok = true;
    for (uint8_t i = 0; i < NB_EMBASES; ++i) embases_ok = embases_ok && m.embases[i].conforme;
    e.couvain_valide = m.couvain_complet && embases_ok;
    e.t_couvain_min = m.t_couvain_min;
    e.t_couvain_max = m.t_couvain_max;
    e.air_valide = m.t_air.valide;
    // Limite air : on prend le PIRE de la valeur filtrée et de la valeur brute. Le filtre
    // médian 5 points retarde de ~4 s, inacceptable avec 1 °C de marge sous la coupure C4.
    e.t_air = (m.t_air.t_brute > m.t_air.t) ? m.t_air.t_brute : m.t_air.t;
    e.consigne = p_.consigne;
    e.hyst_bas = p_.hyst_bas;
    e.hyst_haut = p_.hyst_haut;
    const bool d = reg_.calculer(e);
    if (reg_.limite_couvain()) avert_ |= AV_LIMITE_COUVAIN;
    if (reg_.limite_air()) avert_ |= AV_LIMITE_AIR;
    return d;
}

void MachineEtats::suivre_resume(const Mesures& m) {
    float t[NB_SONDES_COUVAIN];
    bool v[NB_SONDES_COUVAIN];
    temperatures_couvain(m, t, v);
    for (uint8_t i = 0; i < NB_SONDES_COUVAIN; ++i) {
        if (v[i] && t[i] > resume_.tmax_couvain[i]) resume_.tmax_couvain[i] = t[i];
    }
    if (m.t_air.valide && m.t_air.t > resume_.tmax_air) resume_.tmax_air = m.t_air.t;
    if (etat_ == Etat::PALIER && m.couvain_complet) {
        const float ecart = m.t_couvain_max - m.t_couvain_min;
        if (ecart > resume_.ecart_max_palier) resume_.ecart_max_palier = ecart;
        if (ecart > defauts::ECART_SONDES_MAX) avert_ |= AV_ECART_SONDES;
    }
}

void MachineEtats::finaliser_resume(uint32_t maintenant_ms, uint32_t defauts_actifs) {
    if (!cycle_en_cours_) return;
    cycle_en_cours_ = false;
    resume_.palier_cumule_s = palier_cumule_ms_ / 1000u;
    resume_.hors_plage_s = hors_plage_ms_ / 1000u;
    resume_.duree_cycle_s = (maintenant_ms - debut_cycle_ms_) / 1000u;
    resume_.defauts = defauts_actifs;
    resume_.avertissements = avert_;
    resume_.palier_complet = palier_cumule_ms_ >= p_.duree_palier_min * MIN_MS;
    resume_pret_ = true;
}

Sorties MachineEtats::pas(const Mesures& m, const Commandes& cmd, Securite& secu, uint32_t maintenant_ms) {
    const uint32_t dt = premier_ ? 0u : (maintenant_ms - dernier_ms_);
    premier_ = false;
    dernier_ms_ = maintenant_ms;
    transition_ = false;
    resume_pret_ = false;

    // --- 1) Supervision (indépendante) avec le contexte réellement appliqué au pas précédent ---
    ContexteSecurite ctx;
    ctx.cycle_actif = etat_cycle(etat_);
    ctx.refroidissement = etat_ == Etat::REFROIDISSEMENT;
    ctx.chauffe_demandee = reg_.sortie();
    ctx.ssr_commande = derniere_chauffe_;
    for (uint8_t v = 0; v < NB_VENTILOS; ++v) ctx.pwm_pct[v] = derniers_pwm_[v];
    ctx.rpm_nominal[VENTILO_TOIT] = p_.rpm_nominal_toit;
    ctx.rpm_nominal[VENTILO_PLANCHER_A] = p_.rpm_nominal_plancher;
    ctx.rpm_nominal[VENTILO_PLANCHER_B] = p_.rpm_nominal_plancher;
    secu.evaluer(m, ctx, maintenant_ms);

    // --- 2) Séquence -------------------------------------------------------------------------
    switch (etat_) {
        case Etat::ATTENTE:
            if (cmd.depart) {
                autotest_code_ = autotest_statique(m);
                if (secu.defauts() != DEF_AUCUN) autotest_code_ |= AT_DEFAUT_PRESENT;
                if (autotest_code_ == AT_OK) {
                    avert_ &= AV_SD_ABSENTE;  // seul avertissement « matériel » conservé
                    if (!m.sht_valide) avert_ |= AV_SHT45;
                    else if (m.sht_t < defauts::T_AMBIANCE_MIN || m.sht_t > defauts::T_AMBIANCE_MAX) avert_ |= AV_AMBIANCE;
                    if (!m.t_retour.valide) avert_ |= AV_SONDE_RETOUR;
                    aller(Etat::AUTOTEST, maintenant_ms);
                } else {
                    secu.declarer(DEF_AUTOTEST);
                }
            }
            break;

        case Etat::AUTOTEST:
            if (cmd.arret) { aller(Etat::ATTENTE, maintenant_ms); break; }
            if (maintenant_ms - entree_etat_ms_ >= defauts::DUREE_TEST_VENTILO_MS) {
                uint32_t c = autotest_statique(m);
                static const uint32_t code_v[NB_VENTILOS] = {AT_VENTILO_TOIT, AT_VENTILO_PLANCHER_A,
                                                             AT_VENTILO_PLANCHER_B};
                const uint16_t nominal[NB_VENTILOS] = {p_.rpm_nominal_toit, p_.rpm_nominal_plancher,
                                                       p_.rpm_nominal_plancher};
                const uint8_t pwm[NB_VENTILOS] = {p_.pwm_toit_pct, p_.pwm_plancher_pct, p_.pwm_plancher_pct};
                for (uint8_t v = 0; v < NB_VENTILOS; ++v) {
                    if (pwm[v] == 0) continue;  // plancher désactivé pour essai (jamais le toit : borne min 30 %)
                    if (m.rpm[v] == 0 || !Securite::ventilo_ok(m.rpm[v], pwm[v], nominal[v])) c |= code_v[v];
                }
                autotest_code_ = c;
                if (c == AT_OK) {
                    // Début effectif du cycle.
                    secu.debut_cycle();
                    reg_.reinitialiser();
                    debut_cycle_ms_ = maintenant_ms;
                    stable_ms_ = palier_cumule_ms_ = hors_plage_ms_ = homogeneite_ms_ = 0;
                    fen_ms_ = fen_chauffe_ms_ = 0;
                    fen_tmin_debut_ = m.t_couvain_min;
                    resume_ = ResumeCycle();
                    cycle_en_cours_ = true;
                    aller(Etat::MONTEE, maintenant_ms);
                } else {
                    secu.declarer(DEF_AUTOTEST);
                }
            }
            break;

        case Etat::MONTEE: {
            if (cmd.arret) { aller(Etat::REFROIDISSEMENT, maintenant_ms); break; }
            // Entrée en palier : TOUTES les sondes couvain >= 42,0 °C pendant 5 min consécutives.
            if (m.couvain_complet && m.t_couvain_min >= defauts::T_PALIER_MIN) stable_ms_ += dt;
            else stable_ms_ = 0;
            if (stable_ms_ >= defauts::STABILITE_ENTREE_PALIER_S * 1000u) {
                resume_.duree_montee_s = (maintenant_ms - debut_cycle_ms_) / 1000u;
                aller(Etat::PALIER, maintenant_ms);
                break;
            }
            if (maintenant_ms - entree_etat_ms_ >= p_.timeout_montee_min * MIN_MS) {
                secu.declarer(DEF_TIMEOUT_MONTEE);
                break;
            }
            // Chauffe inefficace : fenêtres de 20 min.
            fen_ms_ += dt;
            if (derniere_chauffe_) fen_chauffe_ms_ += dt;
            if (fen_ms_ >= defauts::FENETRE_INEFFICACE_MIN * MIN_MS) {
                const bool sature = static_cast<float>(fen_chauffe_ms_) >=
                                    defauts::TAUX_CHAUFFE_INEFFICACE * static_cast<float>(fen_ms_);
                if (sature && (m.t_couvain_min - fen_tmin_debut_) < defauts::PROGRES_MIN_INEFFICACE) {
                    secu.declarer(DEF_CHAUFFE_INEFFICACE);
                }
                fen_ms_ = fen_chauffe_ms_ = 0;
                fen_tmin_debut_ = m.t_couvain_min;
            }
            break;
        }

        case Etat::PALIER:
            if (cmd.arret) { aller(Etat::REFROIDISSEMENT, maintenant_ms); break; }
            if (m.couvain_complet && m.t_couvain_min >= defauts::T_PALIER_MIN &&
                m.t_couvain_max <= defauts::T_COEUR_MAX_REG) {
                palier_cumule_ms_ += dt;
            } else {
                hors_plage_ms_ += dt;
            }
            if (palier_cumule_ms_ >= p_.duree_palier_min * MIN_MS) {
                aller(Etat::REFROIDISSEMENT, maintenant_ms);
                break;
            }
            if (hors_plage_ms_ > defauts::PALIER_PERDU_MAX_MIN * MIN_MS) secu.declarer(DEF_PALIER_PERDU);
            break;

        case Etat::REFROIDISSEMENT: {
            const uint32_t depuis = maintenant_ms - entree_etat_ms_;
            if (m.nb_couvain_valides > 0 && m.t_couvain_max <= defauts::T_FIN_REFROID) {
                aller(Etat::FIN, maintenant_ms);
            } else if (depuis >= defauts::TIMEOUT_REFROID_MIN * MIN_MS) {
                // Pas de remontée détectée (sinon `securite` aurait posé DEF_SSR_COLLE) : décroissance lente.
                avert_ |= AV_REFROID_TIMEOUT;
                aller(Etat::FIN, maintenant_ms);
            }
            break;
        }

        case Etat::FIN:
            if (cmd.acquit) aller(Etat::ATTENTE, maintenant_ms);
            break;

        case Etat::DEFAUT:
            if (cmd.acquit && secu.acquitter(m)) aller(Etat::ATTENTE, maintenant_ms);
            break;
    }

    // Homogénéité insuffisante (MONTÉE/PALIER) : la limite couvain coupe la chauffe alors
    // que la sonde la plus froide n'a pas atteint 42,0 °C (décision du pas précédent).
    if (etat_chauffant(etat_) && reg_.limite_couvain() && m.t_couvain_min < defauts::T_PALIER_MIN) {
        homogeneite_ms_ += dt;
        if (homogeneite_ms_ > defauts::HOMOGENEITE_MAX_MIN * MIN_MS) secu.declarer(DEF_HOMOGENEITE);
    }

    // Tout défaut verrouillé fait basculer en DÉFAUT, quel que soit l'état.
    if (secu.defauts() != DEF_AUCUN && etat_ != Etat::DEFAUT) aller(Etat::DEFAUT, maintenant_ms);

    if (etat_cycle(etat_)) suivre_resume(m);
    if (etat_ == Etat::FIN || etat_ == Etat::DEFAUT) finaliser_resume(maintenant_ms, secu.defauts());

    // --- 3) Sorties (calculées sur l'état FINAL de ce pas) -------------------------------------
    Sorties s;
    switch (etat_) {
        case Etat::AUTOTEST:
            ventilos(s, p_.pwm_toit_pct, p_.pwm_plancher_pct);
            break;
        case Etat::MONTEE:
        case Etat::PALIER: {
            ventilos(s, p_.pwm_toit_pct, p_.pwm_plancher_pct);
            const bool demande = regler(m);
            s.chauffe = demande && secu.autorise_chauffe();
            break;
        }
        case Etat::REFROIDISSEMENT:
            reg_.reinitialiser();
            if (maintenant_ms - entree_etat_ms_ < defauts::BRASSAGE_REFROID_MIN * MIN_MS) {
                ventilos(s, defauts::PWM_BRASSAGE_PCT, defauts::PWM_BRASSAGE_PCT);
            }
            break;
        case Etat::DEFAUT:
            reg_.reinitialiser();
            if (secu.brassage_requis()) ventilos(s, defauts::PWM_BRASSAGE_PCT, defauts::PWM_BRASSAGE_PCT);
            break;
        default:
            reg_.reinitialiser();
            break;
    }
    // Dernier mot à la sécurité, dans tous les états.
    if (!secu.autorise_chauffe()) s.chauffe = false;
    s.ouvrir_relais_serie = secu.ouvrir_relais_serie();

    derniere_chauffe_ = s.chauffe;
    for (uint8_t v = 0; v < NB_VENTILOS; ++v) derniers_pwm_[v] = s.pwm_pct[v];
    return s;
}

}  // namespace tv
