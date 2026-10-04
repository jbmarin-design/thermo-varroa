// =============================================================================
// validation_sonde.h — Validation d'une lecture DS18B20 brute (code portable)
// Règles (architecture §3.1) : CRC KO, valeur « usine » 85,0 °C (conversion non
// faite) ou −127 °C (sonde déconnectée), valeur hors [−10 ; 60] °C = échec.
// 3 échecs consécutifs = sonde invalide.
// =============================================================================
#pragma once

#include <cmath>

#include "parametres_defaut.h"
#include "types_mesures.h"

namespace tv {

/// Applique une lecture brute (avant offset) à une sonde.
/// lecture_ok = false si CRC KO ou sonde muette.
inline void appliquer_lecture(LectureSonde& s, bool lecture_ok, float t_brute_sans_offset) {
    bool ok = lecture_ok && !std::isnan(t_brute_sans_offset);
    if (ok) {
        // 85,0 °C exact = valeur de power-on reset (conversion non effectuée) ; −127 = déconnectée.
        if (std::fabs(t_brute_sans_offset - 85.0f) < 0.001f) ok = false;
        if (t_brute_sans_offset <= -127.0f + 0.001f) ok = false;
    }
    if (ok) {
        const float t = t_brute_sans_offset + s.offset;
        if (t < defauts::T_PLAUSIBLE_MIN || t > defauts::T_PLAUSIBLE_MAX) ok = false;
        if (ok) {
            s.t_brute = t;
            s.filtre.ajouter(t);
            s.t = s.filtre.valeur();
            s.echecs = 0;
        }
    }
    if (!ok && s.echecs < 255) ++s.echecs;
    s.valide = s.presente && s.filtre.taille() > 0 && s.echecs < defauts::ECHECS_AVANT_INVALIDE;
}

}  // namespace tv
