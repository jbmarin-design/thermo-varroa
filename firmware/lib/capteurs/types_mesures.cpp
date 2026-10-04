// types_mesures.cpp — consolidation des mesures (code portable, testé en native)
#include "types_mesures.h"

#include <cctype>
#include <cstdio>

namespace tv {

// Ordre fixé par le pilote capteurs : les sondes de P1 sont triées par position
// d'étalonnage (haut, centre, bas) ; à défaut, ordre du balayage ROM.
const char* const NOMS_COUVAIN[NB_SONDES_COUVAIN] = {"P1_haut", "P1_centre", "P1_bas", "P2", "P3"};

void temperatures_couvain(const Mesures& m, float tab[NB_SONDES_COUVAIN], bool valide[NB_SONDES_COUVAIN]) {
    uint8_t k = 0;
    for (uint8_t e = 0; e < NB_EMBASES; ++e) {
        for (uint8_t s = 0; s < SONDES_ATTENDUES[e]; ++s) {
            const LectureSonde& l = m.embases[e].sondes[s];
            const bool ok = s < m.embases[e].nb_sondes && l.valide;
            tab[k] = ok ? l.t : 0.0f;
            valide[k] = ok;
            ++k;
        }
    }
}

void consolider(Mesures& m) {
    // Statut de conformité de chaque embase : nombre exact de sondes, toutes valides.
    for (uint8_t e = 0; e < NB_EMBASES; ++e) {
        EtatEmbase& eb = m.embases[e];
        bool ok = eb.presence && eb.nb_trouvees == SONDES_ATTENDUES[e] && eb.nb_sondes == SONDES_ATTENDUES[e];
        for (uint8_t s = 0; ok && s < eb.nb_sondes; ++s) ok = eb.sondes[s].valide;
        eb.conforme = ok;
    }

    float t[NB_SONDES_COUVAIN];
    bool v[NB_SONDES_COUVAIN];
    temperatures_couvain(m, t, v);

    m.nb_couvain_valides = 0;
    m.t_couvain_min = 1000.0f;
    m.t_couvain_max = -1000.0f;
    for (uint8_t i = 0; i < NB_SONDES_COUVAIN; ++i) {
        if (!v[i]) continue;
        ++m.nb_couvain_valides;
        if (t[i] < m.t_couvain_min) m.t_couvain_min = t[i];
        if (t[i] > m.t_couvain_max) m.t_couvain_max = t[i];
    }
    m.couvain_complet = (m.nb_couvain_valides == NB_SONDES_COUVAIN);
    if (m.nb_couvain_valides == 0) { m.t_couvain_min = 0.0f; m.t_couvain_max = 0.0f; }

    // Maximum des valeurs brutes (défaut immédiat si > 45 °C répété).
    m.t_brute_max = -1000.0f;
    for (uint8_t e = 0; e < NB_EMBASES; ++e) {
        for (uint8_t s = 0; s < m.embases[e].nb_sondes; ++s) {
            const LectureSonde& l = m.embases[e].sondes[s];
            if (l.presente && l.filtre.taille() > 0 && l.t_brute > m.t_brute_max) m.t_brute_max = l.t_brute;
        }
    }
    if (m.t_air.presente && m.t_air.filtre.taille() > 0 && m.t_air.t_brute > m.t_brute_max) {
        m.t_brute_max = m.t_air.t_brute;
    }
}

void rom_vers_texte(const uint8_t rom[8], char* texte) {
    for (uint8_t i = 0; i < 8; ++i) std::snprintf(texte + 2 * i, 3, "%02X", rom[i]);
    texte[16] = '\0';
}

bool texte_vers_rom(const char* texte, uint8_t rom[8]) {
    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t octet = 0;
        for (uint8_t j = 0; j < 2; ++j) {
            const char c = static_cast<char>(std::toupper(static_cast<unsigned char>(texte[2 * i + j])));
            uint8_t v;
            if (c >= '0' && c <= '9') v = static_cast<uint8_t>(c - '0');
            else if (c >= 'A' && c <= 'F') v = static_cast<uint8_t>(c - 'A' + 10);
            else return false;
            octet = static_cast<uint8_t>((octet << 4) | v);
        }
        rom[i] = octet;
    }
    return true;
}

}  // namespace tv
