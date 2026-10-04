// =============================================================================
// enable_ssr.h — Logique de l'« enable dynamique » du SSR (couche C3)
// Code portable (testé en native) ; le basculement physique du GPIO est fait par
// un esp_timer périodique de 1 ms dans src/noeud/hal_esp32.cpp.
//
// Principe matériel (hardware/cablage-phase1.md §6) : le SSR n'est alimenté que
// si le GPIO 13 délivre un SIGNAL CARRÉ (500 Hz). Une pompe de charge le
// transforme en courant de base pour le transistor qui commande le SSR ; niveau
// figé (0 ou 1) = plus de courant en quelques dizaines de ms = SSR ouvert. Un MCU
// planté, en reset ou bloqué ne peut donc pas maintenir la chauffe.
//
// Côté logiciel, le timer ne bascule le GPIO que si :
//   1. la boucle principale a demandé la chauffe (commande(true)),
//   2. ET elle a rafraîchi son « jeton » depuis moins de JETON_SSR_MAX_AGE_MS tics (1 tic = 1 ms).
// Une boucle principale bloquée arrête donc aussi le signal, même si le timer tourne.
// Le timer lui-même tourne dans une tâche : s'il est affamé, le signal s'arrête (sûr).
// =============================================================================
#pragma once

#include <cstdint>

#include "parametres_defaut.h"

namespace tv {

class EnableDynamique {
public:
    /// Boucle principale, à chaque pas : commande voulue + rafraîchissement du jeton.
    void commande(bool chauffe) {
        jeton_ = ticks_;
        jeton_valide_ = true;
        chauffe_ = chauffe;
    }

    /// Force l'arrêt immédiat.
    void couper() { chauffe_ = false; }

    /// Appelé toutes les 1 ms par le timer : retourne le niveau à appliquer au GPIO.
    bool tic() {
        ++ticks_;
        if (actif()) niveau_ = !niveau_;
        else niveau_ = false;
        return niveau_;
    }

    /// Vrai si le signal carré doit être produit.
    bool actif() const {
        return chauffe_ && jeton_valide_ && (ticks_ - jeton_) < defauts::JETON_SSR_MAX_AGE_MS;
    }

private:
    volatile uint32_t ticks_ = 0;
    volatile uint32_t jeton_ = 0;
    volatile bool jeton_valide_ = false;
    volatile bool chauffe_ = false;
    volatile bool niveau_ = false;
};

}  // namespace tv
