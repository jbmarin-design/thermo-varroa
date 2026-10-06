// =============================================================================
// journal.h — Formatage des journaux CSV (code portable, testé en native)
//
// Deux flux, écrits sur la µSD ET recopiés sur la liaison série :
//   - MESURES   : une ligne toutes les 10 s en cycle, 60 s en ATTENTE/FIN/DEFAUT ;
//   - EVENEMENTS: transitions d'état, défauts, avertissements, résumé de cycle.
// Les deux partagent le même fichier : la première colonne `type` vaut M ou E
// (un tableur filtre facilement ; un seul fichier à récupérer par cycle).
// Séparateur « ; », décimales avec un POINT (exploitable par Python/R sans
// conversion ; LibreOffice : importer avec « point » comme séparateur décimal).
// Les lignes d'en-tête commençant par « # » portent les métadonnées (version
// firmware, paramètres figés au départ, table d'étalonnage, ROM par position).
// =============================================================================
#pragma once

#include <cstddef>
#include <cstdint>

#include "machine_etats.h"
#include "parametres.h"
#include "securite.h"
#include "types_mesures.h"

namespace tv {

/// Horodatage ISO 8601 local (sans fuseau) ; valide = false -> "0000-00-00T00:00:00".
struct Horodatage {
    bool valide = false;
    uint16_t annee = 0;
    uint8_t mois = 0, jour = 0, heure = 0, minute = 0, seconde = 0;
};

void horodatage_texte(const Horodatage& h, char* buf, size_t n);

/// Ligne d'en-tête des colonnes (sans retour à la ligne). Retourne la longueur écrite.
int journal_entete_colonnes(char* buf, size_t n);

/// Données de contexte d'une ligne de mesures, en plus de Mesures.
struct ContexteLigne {
    Horodatage h;
    uint32_t t_ms = 0;
    Etat etat = Etat::ATTENTE;
    float consigne = 0;
    bool chauffe = false;           // commande SSR appliquée
    bool demande_regulation = false;
    uint8_t raisons_coupure = 0;
    uint8_t pwm_pct[NB_VENTILOS] = {0, 0, 0};
    uint32_t defauts = 0;
    uint32_t avertissements = 0;
    uint32_t palier_cumule_s = 0;
    uint32_t hors_plage_s = 0;
    uint32_t chauffe_cumulee_s = 0;
    bool relais_trip = false;       // le MCU demande l'ouverture de C4
    bool film = false;              // commande du film du plancher appliquée (variante D21)
    uint8_t raisons_film = 0;       // RaisonFilm : pourquoi le film est coupé (1 = variante désactivée)
};

/// Ligne de mesures (sans retour à la ligne). Les valeurs invalides sont laissées VIDES.
/// Colonnes de la variante plancher chauffant (D21), ajoutées EN FIN de ligne pour ne pas
/// décaler les colonnes existantes : t_film (°C, vide si sonde absente), film (0/1),
/// raisons_film (bits RaisonFilm, 1 = variante désactivée).
/// Retourne la longueur écrite (tronquée à n-1 au besoin).
int journal_ligne_mesures(const Mesures& m, const ContexteLigne& c, char* buf, size_t n);

/// Ligne d'événement : E;horodatage;t_ms;etat;code;detail
int journal_ligne_evenement(const Horodatage& h, uint32_t t_ms, Etat etat, const char* code, const char* detail,
                            char* buf, size_t n);

/// Détail texte des bits de défaut (ex. "air_surtemp|peigne_P2"), "aucun" si 0.
void defauts_detail(uint32_t defauts, char* buf, size_t n);

/// Ligne de résumé de cycle (événement « RESUME »).
int journal_resume(const Horodatage& h, uint32_t t_ms, const ResumeCycle& r, char* buf, size_t n);

/// Lignes de métadonnées « # ... » (version, paramètres). Appeler ligne par ligne :
/// retourne la longueur écrite, ou 0 quand il n'y a plus de ligne (index hors plage).
int journal_metadonnees(uint8_t index, const Parametres& p, const char* id_ruche, char* buf, size_t n);

}  // namespace tv
