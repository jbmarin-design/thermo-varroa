// =============================================================================
// regulation.h — Régulation TOUT-OU-RIEN avec hystérésis (Phase 1)
//
// - Grandeur régulée : température de la sonde couvain la PLUS FROIDE (Tmin).
//     chauffe ON  si Tmin <= consigne - hyst_bas
//     chauffe OFF si Tmin >= consigne + hyst_haut
//     entre les deux : état précédent conservé.
// - Limites C1 (coupent la chauffe, non bloquantes, avec hystérésis) :
//     sonde couvain la plus CHAUDE > 43,5 °C  -> chauffe 0 jusqu'à < 43,0 °C
//     air soufflé                 > 44,0 °C  -> chauffe 0 jusqu'à <= 43,5 °C
// - Aucune donnée valide -> chauffe 0.
// Pas de PID en Phase 1 (prévu en Phase 2 : cascade cœur / air soufflé).
// La commande effective du SSR = sortie() ET autorisation de `securite`.
// Code portable (testé en native).
// =============================================================================
#pragma once

#include <cstdint>

namespace tv {

struct EntreeRegulation {
    bool couvain_valide = false;  // au moins une sonde couvain valide ET toutes les embases conformes
    float t_couvain_min = 0;
    float t_couvain_max = 0;
    bool air_valide = false;
    float t_air = 0;
    float consigne = 0;
    float hyst_bas = 0;
    float hyst_haut = 0;
};

enum RaisonCoupure : uint8_t {
    COUPURE_AUCUNE        = 0,
    COUPURE_DONNEES       = 1 << 0,  // données invalides
    COUPURE_LIMITE_COUVAIN = 1 << 1, // Tmax couvain > 43,5 °C
    COUPURE_LIMITE_AIR    = 1 << 2,  // T air soufflé > 44,0 °C
    COUPURE_CONSIGNE      = 1 << 3,  // consigne atteinte (hystérésis TOR)
};

/// Plafond de puissance : vrai si, à l'instant t_ms, le SSR peut conduire dans la fenêtre
/// de FENETRE_PUISSANCE_MS (début de fenêtre = conduction). pct >= 100 : toujours vrai.
bool fenetre_puissance(uint32_t t_ms, uint8_t pct);

class RegulationTOR {
public:
    void reinitialiser();
    /// Calcule la demande de chauffe. Appelée à chaque nouvelle mesure (2 s).
    bool calculer(const EntreeRegulation& e);

    bool sortie() const { return sortie_; }
    bool tor() const { return tor_; }
    bool limite_couvain() const { return lim_couvain_; }
    bool limite_air() const { return lim_air_; }
    /// Combinaison de RaisonCoupure expliquant pourquoi la sortie est à 0 (0 si chauffe).
    uint8_t raisons() const { return raisons_; }

private:
    bool tor_ = false;
    bool lim_couvain_ = false;
    bool lim_air_ = false;
    bool sortie_ = false;
    uint8_t raisons_ = COUPURE_DONNEES;
};

}  // namespace tv
