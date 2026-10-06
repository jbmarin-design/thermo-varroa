// journal.cpp — voir journal.h (code portable)
#include "journal.h"

#include <cstdio>
#include <cstring>

#include "parametres_defaut.h"

namespace tv {

namespace {

/// Ajoute du texte formaté à buf à partir de pos, sans débordement.
struct Tampon {
    char* buf;
    size_t n;
    size_t pos = 0;
    Tampon(char* b, size_t taille) : buf(b), n(taille) { if (n) buf[0] = '\0'; }
    template <typename... A>
    void ajouter(const char* fmt, A... a) {
        if (pos + 1 >= n) return;
        const int k = std::snprintf(buf + pos, n - pos, fmt, a...);
        if (k > 0) pos = (pos + static_cast<size_t>(k) < n) ? pos + static_cast<size_t>(k) : n - 1;
    }
    void texte(const char* s) { ajouter("%s", s); }
    /// Température : 2 décimales, ou champ vide si invalide.
    void temp(bool valide, float t) {
        if (valide) ajouter(";%.2f", static_cast<double>(t));
        else texte(";");
    }
    int longueur() const { return static_cast<int>(pos); }
};

}  // namespace

void horodatage_texte(const Horodatage& h, char* buf, size_t n) {
    if (!h.valide) {
        std::snprintf(buf, n, "0000-00-00T00:00:00");
        return;
    }
    std::snprintf(buf, n, "%04u-%02u-%02uT%02u:%02u:%02u", h.annee, h.mois, h.jour, h.heure, h.minute, h.seconde);
}

int journal_entete_colonnes(char* buf, size_t n) {
    Tampon t(buf, n);
    t.texte("type;horodatage;t_ms;etat;consigne;t_couv_min;t_couv_max");
    for (uint8_t i = 0; i < NB_SONDES_COUVAIN; ++i) t.ajouter(";t_%s", NOMS_COUVAIN[i]);
    t.texte(";t_air;t_retour;t_elem;sht_t;sht_hr;rpm_toit;rpm_plA;rpm_plB;pwm_toit;pwm_plA;pwm_plB");
    t.texte(";chauffe;demande_reg;raisons_coupure;c4_fermee;relais_trip;defauts;avert;palier_s;hors_plage_s;chauffe_cum_s");
    return t.longueur();
}

int journal_ligne_mesures(const Mesures& m, const ContexteLigne& c, char* buf, size_t n) {
    Tampon t(buf, n);
    char h[24];
    horodatage_texte(c.h, h, sizeof h);
    t.ajouter("M;%s;%lu;%s;%.2f", h, static_cast<unsigned long>(c.t_ms), etat_texte(c.etat),
              static_cast<double>(c.consigne));
    t.temp(m.couvain_complet, m.t_couvain_min);
    t.temp(m.couvain_complet, m.t_couvain_max);
    float tc[NB_SONDES_COUVAIN];
    bool vc[NB_SONDES_COUVAIN];
    temperatures_couvain(m, tc, vc);
    for (uint8_t i = 0; i < NB_SONDES_COUVAIN; ++i) t.temp(vc[i], tc[i]);
    t.temp(m.t_air.valide, m.t_air.t);
    t.temp(m.t_retour.valide, m.t_retour.t);
    t.temp(m.t_elem_valide, m.t_elem);
    t.temp(m.sht_valide, m.sht_t);
    if (m.sht_valide) t.ajouter(";%.1f", static_cast<double>(m.sht_hr));
    else t.texte(";");
    for (uint8_t v = 0; v < NB_VENTILOS; ++v) t.ajouter(";%u", static_cast<unsigned>(m.rpm[v]));
    for (uint8_t v = 0; v < NB_VENTILOS; ++v) t.ajouter(";%u", static_cast<unsigned>(c.pwm_pct[v]));
    t.ajouter(";%u;%u;%u;%u;%u;0x%05lX;0x%03lX;%lu;%lu;%lu", c.chauffe ? 1u : 0u, c.demande_regulation ? 1u : 0u,
              static_cast<unsigned>(c.raisons_coupure), m.c4_fermee ? 1u : 0u, c.relais_trip ? 1u : 0u,
              static_cast<unsigned long>(c.defauts), static_cast<unsigned long>(c.avertissements),
              static_cast<unsigned long>(c.palier_cumule_s), static_cast<unsigned long>(c.hors_plage_s),
              static_cast<unsigned long>(c.chauffe_cumulee_s));
    return t.longueur();
}

int journal_ligne_evenement(const Horodatage& h, uint32_t t_ms, Etat etat, const char* code, const char* detail,
                            char* buf, size_t n) {
    Tampon t(buf, n);
    char hs[24];
    horodatage_texte(h, hs, sizeof hs);
    // Le détail ne doit pas contenir de « ; » ni de retour à la ligne : on les remplace.
    t.ajouter("E;%s;%lu;%s;%s;", hs, static_cast<unsigned long>(t_ms), etat_texte(etat), code);
    for (const char* p = detail ? detail : ""; *p && t.pos + 1 < n; ++p) {
        const char c = (*p == ';' || *p == '\n' || *p == '\r') ? ',' : *p;
        buf[t.pos++] = c;
        buf[t.pos] = '\0';
    }
    return t.longueur();
}

void defauts_detail(uint32_t defauts, char* buf, size_t n) {
    Tampon t(buf, n);
    if (defauts == 0) { t.texte("aucun"); return; }
    bool premier = true;
    for (uint8_t b = 0; b < 32; ++b) {
        const uint32_t bit = 1u << b;
        if (!(defauts & bit)) continue;
        if (!premier) t.texte("|");
        t.texte(defaut_texte(bit));
        premier = false;
    }
}

int journal_resume(const Horodatage& h, uint32_t t_ms, const ResumeCycle& r, char* buf, size_t n) {
    char detail[384];
    Tampon d(detail, sizeof detail);
    d.ajouter("montee_s=%lu palier_s=%lu hors_plage_s=%lu cycle_s=%lu complet=%u tmax_air=%.2f ecart_max=%.2f",
              static_cast<unsigned long>(r.duree_montee_s), static_cast<unsigned long>(r.palier_cumule_s),
              static_cast<unsigned long>(r.hors_plage_s), static_cast<unsigned long>(r.duree_cycle_s),
              r.palier_complet ? 1u : 0u, static_cast<double>(r.tmax_air), static_cast<double>(r.ecart_max_palier));
    for (uint8_t i = 0; i < NB_SONDES_COUVAIN; ++i) {
        d.ajouter(" tmax_%s=%.2f", NOMS_COUVAIN[i], static_cast<double>(r.tmax_couvain[i]));
    }
    d.ajouter(" t_init=%.2f hr_init=%.1f t_retour=%.2f", static_cast<double>(r.t_couvain_init),
              static_cast<double>(r.hr_init), static_cast<double>(r.t_retour_cible));
    d.ajouter(" defauts=0x%05lX avert=0x%03lX", static_cast<unsigned long>(r.defauts),
              static_cast<unsigned long>(r.avertissements));
    return journal_ligne_evenement(h, t_ms, r.defauts ? Etat::DEFAUT : Etat::FIN, "RESUME", detail, buf, n);
}

int journal_metadonnees(uint8_t index, const Parametres& p, const char* id_ruche, char* buf, size_t n) {
    Tampon t(buf, n);
    switch (index) {
        case 0: t.ajouter("# firmware=%s version=%s", FIRMWARE_NOM, FIRMWARE_VERSION); break;
        case 1: t.ajouter("# ruche=%s phase=1 banc=a_vide", id_ruche ? id_ruche : "?"); break;
        case 2:
            t.ajouter("# consigne=%.2f hyst_bas=%.2f hyst_haut=%.2f palier_min=%lu timeout_montee_min=%lu",
                      static_cast<double>(p.consigne), static_cast<double>(p.hyst_bas),
                      static_cast<double>(p.hyst_haut), static_cast<unsigned long>(p.duree_palier_min),
                      static_cast<unsigned long>(p.timeout_montee_min));
            break;
        case 3:
            t.ajouter("# pwm_toit=%u pwm_plancher=%u rpm_nom_toit=%u rpm_nom_plancher=%u",
                      static_cast<unsigned>(p.pwm_toit_pct), static_cast<unsigned>(p.pwm_plancher_pct),
                      static_cast<unsigned>(p.rpm_nominal_toit), static_cast<unsigned>(p.rpm_nominal_plancher));
            break;
        case 4:
            t.ajouter("# limites: palier_min=%.1f coeur_max_reg=%.1f coeur_defaut=%.1f air_max_reg=%.1f "
                      "air_defaut=%.1f c4_materiel=%.1f",
                      static_cast<double>(defauts::T_PALIER_MIN), static_cast<double>(defauts::T_COEUR_MAX_REG),
                      static_cast<double>(defauts::T_COEUR_DEFAUT), static_cast<double>(defauts::T_AIR_MAX_REG),
                      static_cast<double>(defauts::T_AIR_DEFAUT), static_cast<double>(defauts::T_C4_MATERIEL));
            break;
        default: return 0;
    }
    return t.longueur();
}

}  // namespace tv
