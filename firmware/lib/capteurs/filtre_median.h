// =============================================================================
// filtre_median.h — Filtre médian glissant sur 5 points (architecture §2.1)
// Élimine une valeur aberrante isolée sans retarder la mesure de plus de 2 points.
// =============================================================================
#pragma once

#include <cstdint>

namespace tv {

class FiltreMedian5 {
public:
    static constexpr uint8_t TAILLE = 5;

    void reinitialiser() { n_ = 0; idx_ = 0; }

    void ajouter(float v) {
        buf_[idx_] = v;
        idx_ = static_cast<uint8_t>((idx_ + 1) % TAILLE);
        if (n_ < TAILLE) ++n_;
    }

    uint8_t taille() const { return n_; }

    /// Médiane des valeurs disponibles (1 à 5). Ne pas appeler si taille() == 0.
    float valeur() const {
        float t[TAILLE];
        for (uint8_t i = 0; i < n_; ++i) t[i] = buf_[i];
        // tri par insertion (5 éléments max)
        for (uint8_t i = 1; i < n_; ++i) {
            float x = t[i];
            int8_t j = static_cast<int8_t>(i) - 1;
            while (j >= 0 && t[j] > x) { t[j + 1] = t[j]; --j; }
            t[j + 1] = x;
        }
        if (n_ % 2 == 1) return t[n_ / 2];
        return 0.5f * (t[n_ / 2 - 1] + t[n_ / 2]);
    }

private:
    float buf_[TAILLE] = {0, 0, 0, 0, 0};
    uint8_t n_ = 0;
    uint8_t idx_ = 0;
};

}  // namespace tv
