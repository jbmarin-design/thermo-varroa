// =============================================================================
// aides_test.h — Fabrication de mesures simulées pour les tests unitaires (native)
// =============================================================================
#pragma once

#include <cstring>

#include "machine_etats.h"
#include "parametres.h"
#include "securite.h"
#include "types_mesures.h"
#include "validation_sonde.h"

namespace aides {

constexpr uint32_t PAS_MS = 2000;  // période d'acquisition
constexpr uint32_t MIN = 60u * 1000u;

inline void rom_test(uint8_t rom[8], uint8_t id) {
    std::memset(rom, 0, 8);
    rom[0] = 0x28;
    rom[1] = id;
}

/// Positionne une sonde valide, étalonnée, avec un filtre rempli à la valeur t.
inline void sonde(tv::LectureSonde& s, uint8_t id, float t, tv::Position pos) {
    s = tv::LectureSonde();
    rom_test(s.rom, id);
    s.presente = true;
    s.etalonnee = true;
    s.position = pos;
    for (int i = 0; i < 5; ++i) tv::appliquer_lecture(s, true, t);
}

/// Mesures complètes et conformes : 5 sondes couvain à t_couvain, air à t_air.
inline tv::Mesures mesures(float t_couvain, float t_air) {
    tv::Mesures m;
    static const tv::Position pos_p1[3] = {tv::Position::P1_HAUT, tv::Position::P1_CENTRE, tv::Position::P1_BAS};
    for (uint8_t e = 0; e < tv::NB_EMBASES; ++e) {
        tv::EtatEmbase& eb = m.embases[e];
        eb.presence = true;
        eb.nb_trouvees = tv::SONDES_ATTENDUES[e];
        eb.nb_sondes = tv::SONDES_ATTENDUES[e];
        for (uint8_t s = 0; s < eb.nb_sondes; ++s) {
            const tv::Position p = e == 0 ? pos_p1[s] : (e == 1 ? tv::Position::P2 : tv::Position::P3);
            sonde(eb.sondes[s], static_cast<uint8_t>(10 * e + s + 1), t_couvain, p);
        }
    }
    sonde(m.t_air, 50, t_air, tv::Position::AIR);
    sonde(m.t_retour, 51, t_couvain, tv::Position::RETOUR);
    m.sht_valide = true;
    m.sht_t = 22.0f;
    m.sht_hr = 55.0f;
    m.c4_fermee = true;
    m.rpm[0] = 3200;  // 80 % de 4000
    m.rpm[1] = 4800;  // 80 % de 6000
    m.rpm[2] = 4800;
    tv::consolider(m);
    return m;
}

/// Fixe une sonde couvain (index 0..4 dans l'ordre P1_haut, P1_centre, P1_bas, P2, P3).
inline void fixer_couvain(tv::Mesures& m, uint8_t index, float t) {
    uint8_t e = index < 3 ? 0 : index - 2;
    uint8_t s = index < 3 ? index : 0;
    tv::LectureSonde& l = m.embases[e].sondes[s];
    const tv::Position p = l.position;
    sonde(l, l.rom[1], t, p);
    tv::consolider(m);
}

inline void fixer_tout_couvain(tv::Mesures& m, float t) {
    for (uint8_t i = 0; i < tv::NB_SONDES_COUVAIN; ++i) fixer_couvain(m, i, t);
}

inline void fixer_air(tv::Mesures& m, float t) {
    sonde(m.t_air, 50, t, tv::Position::AIR);
    tv::consolider(m);
}

/// Banc complet : machine + sécurité + horloge simulée.
struct Banc {
    tv::MachineEtats machine;
    tv::Securite secu;
    tv::Sorties s;
    uint32_t t = 1000;

    Banc() {
        machine.initialiser(tv::parametres_defaut(), true);
        secu.reinitialiser();
    }

    tv::Sorties pas(const tv::Mesures& m, tv::Commandes c = tv::Commandes()) {
        s = machine.pas(m, c, secu, t);
        t += PAS_MS;
        return s;
    }

    /// Fait tourner pendant `duree_ms` avec des mesures fixes.
    void tourner(const tv::Mesures& m, uint32_t duree_ms) {
        for (uint32_t ecoule = 0; ecoule < duree_ms; ecoule += PAS_MS) pas(m);
    }

    void depart(const tv::Mesures& m) {
        tv::Commandes c;
        c.depart = true;
        pas(m, c);
    }

    /// ATTENTE -> AUTOTEST -> MONTEE avec des mesures conformes.
    void demarrer(const tv::Mesures& m) {
        depart(m);
        tourner(m, defauts::DUREE_TEST_VENTILO_MS + PAS_MS);
    }

    void acquitter(const tv::Mesures& m) {
        tv::Commandes c;
        c.acquit = true;
        pas(m, c);
    }
};

}  // namespace aides
