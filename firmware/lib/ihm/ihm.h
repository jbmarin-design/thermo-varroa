// =============================================================================
// ihm.h — Bouton unique (départ / arrêt / acquittement) et couleur de la LED
// Code portable (testé en native).
//
// Bouton (actif BAS, GPIO 0) :
//   appui court (< 3 s), relâché  -> ATTENTE : départ ; FIN/DEFAUT : acquittement
//   appui long  (>= 3 s)          -> AUTOTEST/MONTEE/PALIER : arrêt opérateur
//                                    (émis dès que 3 s sont atteintes, sans attendre le relâchement)
// =============================================================================
#pragma once

#include <cstdint>

#include "machine_etats.h"

namespace tv {

enum class Appui : uint8_t { AUCUN = 0, COURT, LONG };

class Bouton {
public:
    /// niveau_appuye : vrai si le bouton est enfoncé (niveau électrique déjà inversé).
    /// À appeler souvent (>= toutes les 20 ms). Retourne l'appui détecté à cet instant.
    Appui mettre_a_jour(bool niveau_appuye, uint32_t maintenant_ms);

private:
    bool brut_ = false;
    bool stable_ = false;
    uint32_t changement_ms_ = 0;
    uint32_t debut_appui_ms_ = 0;
    bool long_emis_ = false;
};

/// Convertit un appui en commandes selon l'état courant.
Commandes commandes_depuis_appui(Appui a, Etat e);

struct Couleur { uint8_t r, g, b; };

/// Couleur de la LED d'état. `clignote` = phase de clignotement (bascule ~1 Hz).
/// ATTENTE : bleu (orange si avertissement SD/SHT), AUTOTEST : blanc, MONTEE : jaune clignotant,
/// PALIER : vert clignotant, REFROIDISSEMENT : cyan, FIN : vert fixe, DEFAUT : rouge (clignotant
/// rapide si surtempérature).
Couleur couleur_led(Etat e, uint32_t avertissements, bool surtemperature, bool clignote);

}  // namespace tv
