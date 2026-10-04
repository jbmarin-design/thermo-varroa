// =============================================================================
// conversions.h — Conversions et CRC des capteurs (code portable, testé en native)
// =============================================================================
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace tv {

/// CRC8 Dallas/Maxim (polynôme x^8+x^5+x^4+1, réfléchi 0x8C), utilisé par le DS18B20.
/// Un bloc valide incluant son CRC donne 0.
inline uint8_t crc8_dallas(const uint8_t* d, size_t n) {
    uint8_t crc = 0;
    for (size_t i = 0; i < n; ++i) {
        uint8_t octet = d[i];
        for (uint8_t b = 0; b < 8; ++b) {
            const uint8_t mix = static_cast<uint8_t>((crc ^ octet) & 0x01u);
            crc = static_cast<uint8_t>(crc >> 1);
            if (mix) crc ^= 0x8Cu;
            octet = static_cast<uint8_t>(octet >> 1);
        }
    }
    return crc;
}

/// CRC8 Sensirion (polynôme 0x31, init 0xFF), utilisé par le SHT45 (par mot de 2 octets).
inline uint8_t crc8_sensirion(const uint8_t* d, size_t n) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < n; ++i) {
        crc ^= d[i];
        for (uint8_t b = 0; b < 8; ++b) {
            crc = (crc & 0x80u) ? static_cast<uint8_t>((crc << 1) ^ 0x31u) : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

/// Scratchpad DS18B20 (9 octets) -> °C. Retourne false si CRC KO ou scratchpad nul.
/// Résolution 12 bits : 0,0625 °C/LSB.
inline bool ds18b20_decoder(const uint8_t sp[9], float& t) {
    bool tout_zero = true, tout_ff = true;
    for (uint8_t i = 0; i < 9; ++i) {
        if (sp[i] != 0x00) tout_zero = false;
        if (sp[i] != 0xFF) tout_ff = false;
    }
    if (tout_zero || tout_ff) return false;  // bus muet ou court-circuit
    if (crc8_dallas(sp, 8) != sp[8]) return false;
    const int16_t brut = static_cast<int16_t>(static_cast<uint16_t>(sp[1]) << 8 | sp[0]);
    t = static_cast<float>(brut) / 16.0f;
    return true;
}

/// Trame SHT45 (6 octets : T msb, lsb, crc, HR msb, lsb, crc) -> °C et %HR.
inline bool sht45_decoder(const uint8_t r[6], float& t, float& hr) {
    if (crc8_sensirion(r, 2) != r[2] || crc8_sensirion(r + 3, 2) != r[5]) return false;
    const uint16_t st = static_cast<uint16_t>(r[0] << 8 | r[1]);
    const uint16_t srh = static_cast<uint16_t>(r[3] << 8 | r[4]);
    t = -45.0f + 175.0f * static_cast<float>(st) / 65535.0f;
    hr = -6.0f + 125.0f * static_cast<float>(srh) / 65535.0f;
    if (hr < 0.0f) hr = 0.0f;
    if (hr > 100.0f) hr = 100.0f;
    return true;
}

/// NTC (modèle bêta) montée en bas d'un pont : R_serie vers Vcc, NTC vers 0 V.
/// Retourne NAN si la tension est hors plage (NTC ouverte ou en court-circuit).
inline float ntc_temperature(float v_mv, float vcc_mv, float r_serie, float r0, float beta) {
    if (v_mv <= 20.0f || v_mv >= vcc_mv - 20.0f) return NAN;
    const float r_ntc = r_serie * v_mv / (vcc_mv - v_mv);
    const float inv_t = 1.0f / 298.15f + std::log(r_ntc / r0) / beta;
    return 1.0f / inv_t - 273.15f;
}

/// Impulsions tachymètre -> tr/min.
inline uint16_t tachy_rpm(uint32_t impulsions, uint32_t duree_ms, uint8_t impulsions_par_tour) {
    if (duree_ms == 0 || impulsions_par_tour == 0) return 0;
    const uint32_t rpm = static_cast<uint32_t>(
        (static_cast<uint64_t>(impulsions) * 60000u) / (static_cast<uint64_t>(duree_ms) * impulsions_par_tour));
    return rpm > 65535u ? 65535u : static_cast<uint16_t>(rpm);
}

}  // namespace tv
