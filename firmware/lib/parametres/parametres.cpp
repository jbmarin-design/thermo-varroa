// parametres.cpp — voir parametres.h (code portable)
#include "parametres.h"

#include <cmath>
#include <cstring>

#include "parametres_defaut.h"

namespace tv {

namespace {

template <typename T>
bool borner(T& v, T mini, T maxi) {
    if (v < mini) { v = mini; return true; }
    if (v > maxi) { v = maxi; return true; }
    return false;
}

bool borner_float(float& v, float mini, float maxi, float defaut) {
    if (std::isnan(v)) { v = defaut; return true; }  // NaN : jamais accepté
    return borner(v, mini, maxi);
}

// CRC calculé champ par champ : indépendant des octets de remplissage des structures.
template <typename T>
uint32_t crc_champ(uint32_t crc, const T& v) {
    return crc32(reinterpret_cast<const uint8_t*>(&v), sizeof(T), crc);
}

uint32_t crc_parametres(const Parametres& p) {
    uint32_t c = 0;
    c = crc_champ(c, p.version);
    c = crc_champ(c, p.consigne);
    c = crc_champ(c, p.hyst_bas);
    c = crc_champ(c, p.hyst_haut);
    c = crc_champ(c, p.duree_palier_min);
    c = crc_champ(c, p.timeout_montee_min);
    c = crc_champ(c, p.pwm_toit_pct);
    c = crc_champ(c, p.pwm_plancher_pct);
    c = crc_champ(c, p.rpm_nominal_toit);
    c = crc_champ(c, p.rpm_nominal_plancher);
    return c;
}

uint32_t crc_table(const TableEtalonnage& t) {
    uint32_t c = 0;
    c = crc_champ(c, t.version);
    c = crc_champ(c, t.n);
    for (uint8_t i = 0; i < t.n && i < TableEtalonnage::CAPACITE; ++i) {
        c = crc32(t.e[i].rom, 8, c);
        c = crc_champ(c, t.e[i].offset);
        uint8_t pos = static_cast<uint8_t>(t.e[i].position);
        c = crc_champ(c, pos);
    }
    return c;
}

}  // namespace

uint32_t crc32(const uint8_t* d, size_t n, uint32_t crc_init) {
    uint32_t crc = ~crc_init;
    for (size_t i = 0; i < n; ++i) {
        crc ^= d[i];
        for (int b = 0; b < 8; ++b) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

Parametres parametres_defaut() {
    Parametres p;
    p.version = VERSION_SCHEMA_PARAMETRES;
    p.consigne = defauts::T_CONSIGNE_PALIER;
    p.hyst_bas = defauts::HYST_BAS;
    p.hyst_haut = defauts::HYST_HAUT;
    p.duree_palier_min = defauts::DUREE_PALIER_MIN;
    p.timeout_montee_min = defauts::TIMEOUT_MONTEE_MIN;
    p.pwm_toit_pct = defauts::PWM_TOIT_CHAUFFE_PCT;
    p.pwm_plancher_pct = defauts::PWM_TOIT_CHAUFFE_PCT;
    p.rpm_nominal_toit = defauts::RPM_NOMINAL_TOIT;
    p.rpm_nominal_plancher = defauts::RPM_NOMINAL_PLANCHER;
    parametres_sceller(p);
    return p;
}

bool parametres_borner(Parametres& p) {
    using namespace defauts;
    bool m = false;
    m |= borner_float(p.consigne, T_CONSIGNE_BORNE_MIN, T_CONSIGNE_BORNE_MAX, T_CONSIGNE_PALIER);
    m |= borner_float(p.hyst_bas, HYST_BORNE_MIN, HYST_BORNE_MAX, HYST_BAS);
    m |= borner_float(p.hyst_haut, HYST_BORNE_MIN, HYST_BORNE_MAX, HYST_HAUT);
    // La consigne + hystérésis haute ne doit jamais atteindre la limite couvain.
    if (p.consigne + p.hyst_haut >= T_COEUR_MAX_REG) {
        p.hyst_haut = HYST_BORNE_MIN;
        if (p.consigne + p.hyst_haut >= T_COEUR_MAX_REG) p.consigne = T_COEUR_MAX_REG - 2 * HYST_BORNE_MIN;
        m = true;
    }
    m |= borner<uint32_t>(p.duree_palier_min, DUREE_PALIER_BORNE_MIN, DUREE_PALIER_BORNE_MAX);
    m |= borner<uint32_t>(p.timeout_montee_min, TIMEOUT_MONTEE_BORNE_MIN, TIMEOUT_MONTEE_BORNE_MAX);
    m |= borner<uint8_t>(p.pwm_toit_pct, PWM_BORNE_MIN_PCT, PWM_BORNE_MAX_PCT);
    m |= borner<uint8_t>(p.pwm_plancher_pct, PWM_BORNE_MIN_PCT, PWM_BORNE_MAX_PCT);
    m |= borner<uint16_t>(p.rpm_nominal_toit, 500, 20000);
    m |= borner<uint16_t>(p.rpm_nominal_plancher, 500, 20000);
    return m;
}

void parametres_sceller(Parametres& p) {
    p.version = VERSION_SCHEMA_PARAMETRES;
    p.crc = crc_parametres(p);
}

bool parametres_valides(const Parametres& p) {
    if (p.version != VERSION_SCHEMA_PARAMETRES) return false;
    if (p.crc != crc_parametres(p)) return false;
    Parametres copie = p;
    return !parametres_borner(copie);
}

// -----------------------------------------------------------------------------

const char* position_texte(Position p) {
    switch (p) {
        case Position::P1_HAUT:   return "P1_haut";
        case Position::P1_CENTRE: return "P1_centre";
        case Position::P1_BAS:    return "P1_bas";
        case Position::P2:        return "P2";
        case Position::P3:        return "P3";
        case Position::AIR:       return "air";
        default:                  return "inconnue";
    }
}

bool position_depuis_texte(const char* t, Position& p) {
    struct { const char* nom; Position pos; } table[] = {
        {"haut", Position::P1_HAUT}, {"centre", Position::P1_CENTRE}, {"bas", Position::P1_BAS},
        {"P1_haut", Position::P1_HAUT}, {"P1_centre", Position::P1_CENTRE}, {"P1_bas", Position::P1_BAS},
        {"P2", Position::P2}, {"P3", Position::P3}, {"air", Position::AIR}, {"inconnue", Position::INCONNUE},
    };
    for (const auto& x : table) {
        if (std::strcmp(t, x.nom) == 0) { p = x.pos; return true; }
    }
    return false;
}

int8_t TableEtalonnage::chercher(const uint8_t rom[8]) const {
    for (uint8_t i = 0; i < n && i < CAPACITE; ++i) {
        if (std::memcmp(e[i].rom, rom, 8) == 0) return static_cast<int8_t>(i);
    }
    return -1;
}

bool TableEtalonnage::definir(const uint8_t rom[8], float offset, Position pos) {
    if (std::isnan(offset) || std::fabs(offset) > OFFSET_ETALONNAGE_MAX) return false;
    int8_t i = chercher(rom);
    if (i < 0) {
        if (n >= CAPACITE) return false;
        i = static_cast<int8_t>(n++);
        std::memcpy(e[i].rom, rom, 8);
        e[i].position = Position::INCONNUE;
    }
    e[i].offset = offset;
    if (pos != Position::INCONNUE) e[i].position = pos;
    return true;
}

bool TableEtalonnage::definir_position(const uint8_t rom[8], Position pos) {
    int8_t i = chercher(rom);
    if (i < 0) return false;
    e[i].position = pos;
    return true;
}

void TableEtalonnage::sceller() {
    version = VERSION_SCHEMA_PARAMETRES;
    crc = crc_table(*this);
}

bool TableEtalonnage::valide() const {
    if (version != VERSION_SCHEMA_PARAMETRES || n > CAPACITE) return false;
    return crc == crc_table(*this);
}

}  // namespace tv
