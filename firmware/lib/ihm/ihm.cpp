// ihm.cpp — voir ihm.h
#include "ihm.h"

#include "parametres_defaut.h"

namespace tv {

Appui Bouton::mettre_a_jour(bool niveau_appuye, uint32_t maintenant_ms) {
    if (niveau_appuye != brut_) {
        brut_ = niveau_appuye;
        changement_ms_ = maintenant_ms;
    }
    Appui res = Appui::AUCUN;
    if (brut_ != stable_ && maintenant_ms - changement_ms_ >= defauts::BOUTON_ANTIREBOND_MS) {
        stable_ = brut_;
        if (stable_) {
            debut_appui_ms_ = maintenant_ms;
            long_emis_ = false;
        } else if (!long_emis_) {
            res = Appui::COURT;
        }
    }
    if (stable_ && !long_emis_ && maintenant_ms - debut_appui_ms_ >= defauts::BOUTON_APPUI_LONG_MS) {
        long_emis_ = true;
        res = Appui::LONG;
    }
    return res;
}

Commandes commandes_depuis_appui(Appui a, Etat e) {
    Commandes c;
    if (a == Appui::COURT) {
        if (e == Etat::ATTENTE) c.depart = true;
        if (e == Etat::FIN || e == Etat::DEFAUT) c.acquit = true;
    } else if (a == Appui::LONG) {
        if (e == Etat::AUTOTEST || e == Etat::MONTEE || e == Etat::PALIER) c.arret = true;
    }
    return c;
}

Couleur couleur_led(Etat e, uint32_t avertissements, bool surtemperature, bool clignote) {
    const Couleur eteint{0, 0, 0};
    switch (e) {
        case Etat::ATTENTE:
            if (avertissements & (AV_SD_ABSENTE | AV_SHT45)) return Couleur{40, 20, 0};  // orange
            return Couleur{0, 0, 40};
        case Etat::AUTOTEST:        return Couleur{30, 30, 30};
        case Etat::MONTEE:          return clignote ? Couleur{40, 30, 0} : eteint;
        case Etat::PALIER:          return clignote ? Couleur{0, 40, 0} : eteint;
        case Etat::REFROIDISSEMENT: return Couleur{0, 30, 30};
        case Etat::FIN:             return Couleur{0, 40, 0};
        case Etat::DEFAUT:
            if (surtemperature) return clignote ? Couleur{60, 0, 0} : eteint;
            return Couleur{60, 0, 0};
        default:                    return eteint;
    }
}

}  // namespace tv
