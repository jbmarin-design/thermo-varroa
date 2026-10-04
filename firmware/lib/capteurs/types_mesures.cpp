// types_mesures.cpp — consolidation des mesures (code portable, testé en native)
#include "types_mesures.h"

#include <cctype>
#include <cstdio>
#include <cstring>

#include "parametres_defaut.h"

namespace tv {

// Ordre fixé par le pilote capteurs : les sondes de P1 sont triées par position
// d'étalonnage (haut, centre, bas) ; à défaut, ordre du balayage ROM.
const char* const NOMS_COUVAIN[NB_SONDES_COUVAIN] = {"P1_haut", "P1_centre", "P1_bas", "P2", "P3"};

bool position_compatible(Embase e, Position p) {
    if (p == Position::INCONNUE) return true;  // position non déclarée : l'embase fait foi
    switch (e) {
        case EMBASE_P1: return p == Position::P1_HAUT || p == Position::P1_CENTRE || p == Position::P1_BAS;
        // Les deux peignes de bord sont identiques : on tolère l'inversion gauche/droite
        // (la position journalisée reste celle de l'embase).
        case EMBASE_P2:
        case EMBASE_P3: return p == Position::P2 || p == Position::P3;
        default: return false;
    }
}

uint8_t fusionner_sondes(LectureSonde* slots, uint8_t& nb, uint8_t max, const uint8_t (*roms)[8], uint8_t n_trouves) {
    // 1) Sondes connues : retrouvées ou non.
    for (uint8_t i = 0; i < nb; ++i) {
        slots[i].presente = false;
        for (uint8_t k = 0; k < n_trouves; ++k) {
            if (std::memcmp(slots[i].rom, roms[k], 8) == 0) { slots[i].presente = true; break; }
        }
    }
    // 2) Retrait des sondes absentes depuis trop longtemps (compactage).
    uint8_t j = 0;
    for (uint8_t i = 0; i < nb; ++i) {
        const bool garder = slots[i].presente || slots[i].echecs + 1u < defauts::ECHECS_AVANT_INVALIDE;
        if (garder) {
            if (j != i) slots[j] = slots[i];
            ++j;
        }
    }
    for (uint8_t i = j; i < nb; ++i) slots[i] = LectureSonde();
    nb = j;
    // 3) Nouvelles sondes.
    for (uint8_t k = 0; k < n_trouves; ++k) {
        bool connue = false;
        for (uint8_t i = 0; i < nb; ++i) {
            if (std::memcmp(slots[i].rom, roms[k], 8) == 0) { connue = true; break; }
        }
        if (connue) continue;
        uint8_t place = nb;
        if (nb >= max) {
            // Place occupée par une sonde absente : la nouvelle (présente) la remplace.
            place = max;
            for (uint8_t i = 0; i < nb; ++i) if (!slots[i].presente) { place = i; break; }
            if (place == max) continue;  // bus plein de sondes présentes : sonde en trop (comptée)
        } else {
            ++nb;
        }
        slots[place] = LectureSonde();
        std::memcpy(slots[place].rom, roms[k], 8);
        slots[place].presente = true;
    }
    return n_trouves;
}

void ordonner_sondes(LectureSonde* slots, uint8_t nb) {
    // Tri par insertion (3 éléments max) : position déclarée puis ROM.
    for (uint8_t i = 1; i < nb; ++i) {
        LectureSonde x = slots[i];
        int8_t j = static_cast<int8_t>(i) - 1;
        auto avant = [&](const LectureSonde& a, const LectureSonde& b) {
            const uint8_t pa = a.position == Position::INCONNUE ? 255 : static_cast<uint8_t>(a.position);
            const uint8_t pb = b.position == Position::INCONNUE ? 255 : static_cast<uint8_t>(b.position);
            if (pa != pb) return pa < pb;
            return std::memcmp(a.rom, b.rom, 8) < 0;
        };
        while (j >= 0 && avant(x, slots[j])) { slots[j + 1] = slots[j]; --j; }
        slots[j + 1] = x;
    }
}

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
    // Statut de conformité de chaque embase :
    //  - impulsion de présence 1-Wire (absente = peigne débranché : non conforme IMMÉDIATEMENT) ;
    //  - exactement le nombre de sondes attendu, aucune sonde en trop au balayage ;
    //  - toutes valides (une sonde absente d'un balayage isolé reste valide 3 lectures) ;
    //  - positions déclarées compatibles avec l'embase (peigne bord branché sur P1, etc.).
    for (uint8_t e = 0; e < NB_EMBASES; ++e) {
        EtatEmbase& eb = m.embases[e];
        eb.position_ko = false;
        for (uint8_t s = 0; s < eb.nb_sondes; ++s) {
            if (!position_compatible(static_cast<Embase>(e), eb.sondes[s].position)) eb.position_ko = true;
        }
        bool ok = eb.presence && eb.nb_trouvees <= SONDES_ATTENDUES[e] && eb.nb_sondes == SONDES_ATTENDUES[e] &&
                  !eb.position_ko;
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
