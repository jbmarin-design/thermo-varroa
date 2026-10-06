// Tests unitaires — machine à états (env native, Unity)
#include <unity.h>

#include "../aides_test.h"
#include "parametres_defaut.h"

using namespace tv;
using namespace aides;

void setUp() {}
void tearDown() {}

// --- ATTENTE / AUTOTEST ---------------------------------------------------------------------

void test_demarrage_en_attente_sans_chauffe() {
    Banc b;
    const Mesures m = mesures(34.0f, 30.0f);
    b.tourner(m, 60000);
    TEST_ASSERT_EQUAL(Etat::ATTENTE, b.machine.etat());
    TEST_ASSERT_FALSE(b.s.chauffe);
    for (uint8_t v = 0; v < NB_VENTILOS; ++v) TEST_ASSERT_EQUAL_UINT8(0, b.s.pwm_pct[v]);
}

void test_depart_nominal_passe_par_autotest_puis_montee() {
    Banc b;
    const Mesures m = mesures(34.0f, 30.0f);
    b.depart(m);
    TEST_ASSERT_EQUAL(Etat::AUTOTEST, b.machine.etat());
    TEST_ASSERT_FALSE(b.s.chauffe);                 // jamais de chauffe en AUTOTEST
    TEST_ASSERT_EQUAL_UINT8(80, b.s.pwm_pct[VENTILO_TOIT]);
    b.tourner(m, defauts::DUREE_TEST_VENTILO_MS + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    TEST_ASSERT_TRUE(b.s.chauffe);                  // 34 °C < 42,3 - 0,2
}

void test_depart_refuse_peigne_absent() {
    Banc b;
    Mesures m = mesures(34.0f, 30.0f);
    m.embases[EMBASE_P2].presence = false;          // peigne P2 débranché
    consolider(m);
    b.depart(m);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_PEIGNE_P2);
    TEST_ASSERT_TRUE(b.secu.defauts() & DEF_AUTOTEST);
    TEST_ASSERT_FALSE(b.s.chauffe);
}

void test_depart_refuse_sonde_manquante_sur_p1() {
    Banc b;
    Mesures m = mesures(34.0f, 30.0f);
    m.embases[EMBASE_P1].nb_trouvees = 2;           // une des 3 sondes du peigne centre absente
    m.embases[EMBASE_P1].nb_sondes = 2;
    consolider(m);
    TEST_ASSERT_FALSE(m.couvain_complet);
    b.depart(m);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_PEIGNE_P1);
}

void test_depart_refuse_peigne_bord_sur_embase_centre() {
    Banc b;
    Mesures m = mesures(34.0f, 30.0f);
    m.embases[EMBASE_P1].sondes[1].position = Position::P2;  // sonde de bord sur l'embase centre
    consolider(m);
    TEST_ASSERT_TRUE(m.embases[EMBASE_P1].position_ko);
    b.depart(m);
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_PEIGNE_P1);
}

void test_depart_refuse_sonde_non_etalonnee() {
    Banc b;
    Mesures m = mesures(34.0f, 30.0f);
    m.embases[EMBASE_P3].sondes[0].etalonnee = false;
    b.depart(m);
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_NON_ETALONNEE);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
}

void test_depart_refuse_c4_ouverte() {
    Banc b;
    Mesures m = mesures(34.0f, 30.0f);
    m.c4_fermee = false;
    b.depart(m);
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_C4_OUVERTE);
}

void test_depart_refuse_couvain_hors_plage() {
    Banc b;
    const Mesures m = mesures(40.0f, 30.0f);        // > 39 °C : sonde mal placée / ruche déjà chaude
    b.depart(m);
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_COUVAIN_HORS_PLAGE);
}

void test_depart_refuse_parametres_corrompus() {
    Banc b;
    b.machine.initialiser(parametres_defaut(), false);
    const Mesures m = mesures(34.0f, 30.0f);
    b.depart(m);
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_PARAMETRES);
}

void test_depart_refuse_rtc_invalide() {
    Banc b;
    b.machine.definir_rtc_valide(false);
    const Mesures m = mesures(34.0f, 30.0f);
    b.depart(m);
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_RTC);
}

void test_autotest_ventilo_plancher_bloque() {
    Banc b;
    Mesures m = mesures(34.0f, 30.0f);
    m.rpm[VENTILO_PLANCHER_B] = 0;
    b.depart(m);
    TEST_ASSERT_EQUAL(Etat::AUTOTEST, b.machine.etat());
    b.tourner(m, defauts::DUREE_TEST_VENTILO_MS + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_VENTILO_PLANCHER_B);
    TEST_ASSERT_FALSE(b.s.chauffe);
}

void test_plancher_desactive_pour_essai() {
    Banc b;
    Parametres p = parametres_defaut();
    p.pwm_plancher_pct = 0;
    p.pwm_toit_pct = 0;                             // interdit : ramené à 30 %
    TEST_ASSERT_TRUE(b.machine.changer_parametres(p));
    TEST_ASSERT_EQUAL_UINT8(0, b.machine.parametres().pwm_plancher_pct);
    TEST_ASSERT_EQUAL_UINT8(defauts::PWM_BORNE_MIN_PCT, b.machine.parametres().pwm_toit_pct);
    Mesures m = mesures(34.0f, 30.0f);
    m.rpm[VENTILO_PLANCHER_A] = m.rpm[VENTILO_PLANCHER_B] = 0;
    m.rpm[VENTILO_TOIT] = 1200;                     // 100 % de 30 % x 4000
    b.demarrer(m);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    TEST_ASSERT_EQUAL_UINT8(0, b.s.pwm_pct[VENTILO_PLANCHER_A]);
    b.tourner(m, 2 * MIN);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
}

void test_avertissement_ambiance_non_bloquant() {
    Banc b;
    Mesures m = mesures(34.0f, 30.0f);
    m.sht_t = 35.0f;                                // > 32 °C
    b.demarrer(m);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.avertissements() & AV_AMBIANCE);
}

void test_arret_pendant_autotest_retour_attente() {
    Banc b;
    const Mesures m = mesures(34.0f, 30.0f);
    b.depart(m);
    Commandes c;
    c.arret = true;
    b.pas(m, c);
    TEST_ASSERT_EQUAL(Etat::ATTENTE, b.machine.etat());
}

// --- MONTEE -> PALIER -> REFROIDISSEMENT -> FIN ---------------------------------------------

void test_cycle_nominal_complet() {
    Banc b;
    Mesures m = mesures(34.0f, 38.0f);
    b.demarrer(m);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());

    // Toutes les sondes à 42,1 °C : 5 min de stabilité requises.
    m = mesures(42.1f, 43.0f);
    b.tourner(m, 4 * MIN);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    b.tourner(m, 1 * MIN + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::PALIER, b.machine.etat());

    // Palier : 120 min cumulées.
    b.tourner(m, 119 * MIN);
    TEST_ASSERT_EQUAL(Etat::PALIER, b.machine.etat());
    b.tourner(m, 1 * MIN + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::REFROIDISSEMENT, b.machine.etat());
    TEST_ASSERT_FALSE(b.s.chauffe);
    // Redescente pilotée : brassage permanent aux réglages du cycle.
    TEST_ASSERT_EQUAL_UINT8(80, b.s.pwm_pct[VENTILO_TOIT]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 34.0f, b.machine.resume().t_retour_cible);  // couvain au départ

    // Le couvain suit la rampe en restant 0,3 °C au-dessus de la consigne : jamais de chauffe.
    for (int i = 0; i < 150 * 30 && b.machine.etat() == Etat::REFROIDISSEMENT; ++i) {
        const float t = b.machine.consigne_courante() + 0.3f;
        m = mesures(t, t);
        b.pas(m);
        TEST_ASSERT_FALSE(b.s.chauffe);
        if (b.machine.etat() == Etat::REFROIDISSEMENT) TEST_ASSERT_EQUAL_UINT8(80, b.s.pwm_pct[VENTILO_TOIT]);
    }
    TEST_ASSERT_EQUAL(Etat::FIN, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.resume().palier_complet);
    TEST_ASSERT_EQUAL_UINT32(0, b.secu.defauts());

    // FIN : aucun redémarrage automatique.
    b.tourner(m, 10 * MIN);
    TEST_ASSERT_EQUAL(Etat::FIN, b.machine.etat());
    b.acquitter(m);
    TEST_ASSERT_EQUAL(Etat::ATTENTE, b.machine.etat());
}

void test_palier_exige_toutes_les_sondes() {
    Banc b;
    Mesures m = mesures(34.0f, 38.0f);
    b.demarrer(m);
    m = mesures(42.5f, 43.0f);
    fixer_couvain(m, 3, 41.9f);                     // P2 encore froide
    b.tourner(m, 20 * MIN);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
}

void test_stabilite_remise_a_zero_si_chute() {
    Banc b;
    Mesures chaud = mesures(42.1f, 43.0f);
    Mesures froid = mesures(41.8f, 43.0f);
    b.demarrer(mesures(34.0f, 38.0f));
    b.tourner(chaud, 4 * MIN);
    b.pas(froid);                                   // une chute : le compteur repart à zéro
    b.tourner(chaud, 4 * MIN);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    b.tourner(chaud, 1 * MIN + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::PALIER, b.machine.etat());
}

void test_palier_cumulatif_ne_compte_pas_hors_plage() {
    Banc b;
    Mesures dans = mesures(42.2f, 43.0f);
    Mesures sous = mesures(41.9f, 43.0f);
    b.demarrer(mesures(34.0f, 38.0f));
    b.tourner(dans, 5 * MIN + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::PALIER, b.machine.etat());
    const uint32_t avant = b.machine.palier_cumule_s();
    b.tourner(sous, 10 * MIN);
    TEST_ASSERT_UINT32_WITHIN(2, avant, b.machine.palier_cumule_s());
    TEST_ASSERT_UINT32_WITHIN(4, 600, b.machine.hors_plage_cumule_s());
    TEST_ASSERT_EQUAL(Etat::PALIER, b.machine.etat());
}

void test_palier_ne_compte_pas_si_sonde_chaude_au_dessus_43_5() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    b.tourner(mesures(42.2f, 43.0f), 5 * MIN + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::PALIER, b.machine.etat());
    Mesures m = mesures(42.2f, 43.0f);
    fixer_couvain(m, 1, 43.6f);
    const uint32_t avant = b.machine.palier_cumule_s();
    b.tourner(m, 5 * MIN);
    TEST_ASSERT_UINT32_WITHIN(2, avant, b.machine.palier_cumule_s());
}

void test_palier_perdu_apres_30_min_hors_plage() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    b.tourner(mesures(42.2f, 43.0f), 5 * MIN + PAS_MS);
    const Mesures sous = mesures(41.5f, 43.0f);
    b.tourner(sous, 30 * MIN);
    TEST_ASSERT_EQUAL(Etat::PALIER, b.machine.etat());
    b.tourner(sous, 2 * PAS_MS);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.secu.defauts() & DEF_PALIER_PERDU);
    TEST_ASSERT_FALSE(b.s.chauffe);
}

void test_timeout_montee() {
    Banc b;
    b.demarrer(mesures(34.0f, 43.0f));
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    const uint32_t debut = b.t;
    // Progression lente (0,72 °C / 20 min > 0,5 : pas de « chauffe inefficace »), jamais 42,0 °C.
    float t = 34.0f;
    while (b.machine.etat() == Etat::MONTEE && b.t - debut < 160 * MIN) {
        t += 0.0012f;
        b.pas(mesures(t, 43.0f));
    }
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_EQUAL_HEX32(DEF_TIMEOUT_MONTEE, b.secu.defauts());
    TEST_ASSERT_UINT32_WITHIN(2 * PAS_MS, defauts::TIMEOUT_MONTEE_MIN * MIN, b.t - debut);
}

void test_chauffe_inefficace() {
    Banc b;
    const Mesures m = mesures(35.0f, 40.0f);       // chauffe à 100 %, couvain qui ne bouge pas
    b.demarrer(m);
    b.tourner(m, defauts::FENETRE_INEFFICACE_MIN * MIN + 2 * PAS_MS);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.secu.defauts() & DEF_CHAUFFE_INEFFICACE);
}

void test_homogeneite_insuffisante() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Mesures m = mesures(41.0f, 43.0f);
    fixer_couvain(m, 0, 43.8f);                     // haut de P1 trop chaud, le reste < 42
    b.tourner(m, defauts::HOMOGENEITE_MAX_MIN * MIN + 2 * PAS_MS);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.secu.defauts() & DEF_HOMOGENEITE);
}

void test_arret_operateur_en_montee() {
    Banc b;
    const Mesures m = mesures(36.0f, 40.0f);
    b.demarrer(m);
    Commandes c;
    c.arret = true;
    b.pas(m, c);
    TEST_ASSERT_EQUAL(Etat::REFROIDISSEMENT, b.machine.etat());
    TEST_ASSERT_FALSE(b.s.chauffe);
}

void test_redescente_pilotee_freine_et_suit_la_rampe() {
    Banc b;
    Mesures m = mesures(34.0f, 38.0f);
    b.demarrer(m);
    m = mesures(42.1f, 43.0f);
    b.tourner(m, 5 * MIN + PAS_MS);
    b.tourner(m, 120 * MIN + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::REFROIDISSEMENT, b.machine.etat());
    // Après 20 min, la consigne a baissé d'environ 2 °C (0,1 °C/min).
    b.tourner(mesures(41.0f, 41.0f), 20 * MIN);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 42.1f - 2.0f, b.machine.consigne_courante());
    // Couvain qui chute plus vite que la rampe : la chauffe freine.
    b.pas(mesures(39.0f, 39.0f));
    TEST_ASSERT_TRUE(b.s.chauffe);
    // Couvain au-dessus de la consigne : pas de chauffe.
    b.pas(mesures(40.6f, 40.6f));
    TEST_ASSERT_FALSE(b.s.chauffe);
    // La consigne ne descend jamais sous la cible (couvain de départ, 34 °C).
    b.tourner(mesures(40.6f, 40.6f), 120 * MIN);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 34.0f, b.machine.consigne_courante());
}

void test_redescente_cible_bornee_a_33_sur_banc_froid() {
    Banc b;
    b.demarrer(mesures(20.0f, 20.0f));               // banc à vide, local à 20 °C
    TEST_ASSERT_FLOAT_WITHIN(0.01f, defauts::T_RETOUR_MIN, b.machine.resume().t_retour_cible);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, b.machine.resume().t_couvain_init);
}

void test_refroidissement_timeout_avertissement() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Commandes c;
    c.arret = true;
    b.pas(mesures(38.5f, 39.0f), c);
    TEST_ASSERT_EQUAL(Etat::REFROIDISSEMENT, b.machine.etat());
    // Décroissance très lente qui n'atteint jamais 37 °C.
    float t = 38.5f;
    for (uint32_t e = 0; e <= defauts::TIMEOUT_REFROID_MIN * MIN + PAS_MS; e += PAS_MS) {
        t -= 0.0002f;
        b.pas(mesures(t, t));
    }
    TEST_ASSERT_EQUAL(Etat::FIN, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.avertissements() & AV_REFROID_TIMEOUT);
}

// --- Régulation TOR dans la machine ------------------------------------------------------------

void test_regulation_sur_la_plus_froide() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Mesures m = mesures(42.6f, 43.0f);              // tout au-dessus de consigne + hyst_haut ...
    fixer_couvain(m, 4, 41.9f);                     // ... sauf P3 : la plus froide commande
    b.pas(m);
    TEST_ASSERT_TRUE(b.s.chauffe);
    m = mesures(42.6f, 43.0f);
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.chauffe);
}

void test_hysteresis_tor() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    b.pas(mesures(42.0f, 43.0f));                   // <= 42,1 : ON
    TEST_ASSERT_TRUE(b.s.chauffe);
    b.pas(mesures(42.3f, 43.0f));                   // dans la bande : reste ON
    TEST_ASSERT_TRUE(b.s.chauffe);
    b.pas(mesures(42.4f, 43.0f));                   // >= 42,4 : OFF
    TEST_ASSERT_FALSE(b.s.chauffe);
    b.pas(mesures(42.2f, 43.0f));                   // dans la bande : reste OFF
    TEST_ASSERT_FALSE(b.s.chauffe);
    b.pas(mesures(42.1f, 43.0f));                   // <= 42,1 : ON
    TEST_ASSERT_TRUE(b.s.chauffe);
}

void test_limite_air_coupe_la_chauffe_sans_defaut() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    b.pas(mesures(40.0f, 44.1f));
    TEST_ASSERT_FALSE(b.s.chauffe);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    b.pas(mesures(40.0f, 43.8f));                   // > 43,5 : toujours limité
    TEST_ASSERT_FALSE(b.s.chauffe);
    b.pas(mesures(40.0f, 43.4f));                   // reprise
    TEST_ASSERT_TRUE(b.s.chauffe);
    TEST_ASSERT_TRUE(b.machine.avertissements() & AV_LIMITE_AIR);
}

void test_limite_air_sur_valeur_brute() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Mesures m = mesures(40.0f, 43.0f);
    // Une lecture brute à 44,2 °C : la médiane reste à 43,0 mais la limite doit agir.
    appliquer_lecture(m.t_air, true, 44.2f);
    consolider(m);
    TEST_ASSERT_TRUE(m.t_air.t < 44.0f);
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.chauffe);
}

void test_limite_couvain_coupe_la_chauffe() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Mesures m = mesures(41.0f, 43.0f);
    fixer_couvain(m, 2, 43.6f);
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.chauffe);
    fixer_couvain(m, 2, 43.2f);                     // entre 43,0 et 43,5 : toujours limité
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.chauffe);
    fixer_couvain(m, 2, 42.9f);
    b.pas(m);
    TEST_ASSERT_TRUE(b.s.chauffe);
}

// --- DÉFAUT et acquittement ------------------------------------------------------------------------

void test_defaut_verrouille_jusqu_a_acquittement() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Mesures m = mesures(38.0f, 40.0f);
    m.embases[EMBASE_P3].presence = false;          // peigne débranché en cycle
    consolider(m);
    b.pas(m);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.secu.defauts() & DEF_PEIGNE_P3);
    // Le peigne revient : rien ne se passe sans acquittement.
    m = mesures(38.0f, 40.0f);
    b.tourner(m, 5 * MIN);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_FALSE(b.s.chauffe);
    b.acquitter(m);
    TEST_ASSERT_EQUAL(Etat::ATTENTE, b.machine.etat());  // jamais directement en chauffe
    TEST_ASSERT_FALSE(b.s.chauffe);
}

void test_acquittement_refuse_si_cause_presente() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Mesures m = mesures(38.0f, 40.0f);
    m.c4_fermee = false;
    b.pas(m);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    b.acquitter(m);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    m.c4_fermee = true;
    b.acquitter(m);
    TEST_ASSERT_EQUAL(Etat::ATTENTE, b.machine.etat());
}

void test_changer_parametres_seulement_en_attente() {
    Banc b;
    Parametres p = parametres_defaut();
    p.consigne = 42.0f;
    TEST_ASSERT_TRUE(b.machine.changer_parametres(p));
    b.demarrer(mesures(34.0f, 38.0f));
    p.consigne = 41.0f;
    TEST_ASSERT_FALSE(b.machine.changer_parametres(p));
    TEST_ASSERT_EQUAL_FLOAT(42.0f, b.machine.parametres().consigne);
}

void test_resume_cycle_en_defaut() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Mesures m = mesures(39.0f, 41.0f);
    b.pas(m);
    m.c4_fermee = false;
    b.pas(m);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.resume_pret());
    TEST_ASSERT_FALSE(b.machine.resume().palier_complet);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 39.0f, b.machine.resume().tmax_couvain[0]);
    TEST_ASSERT_TRUE(b.machine.resume().defauts & DEF_C4_OUVERTE);
}

// --- Variante plancher chauffant d'appoint (D21) -----------------------------------------------------

void test_plancher_chauffant_desactive_comportement_inchange() {
    Banc b;                                         // défaut : plancher_chauffant = 0
    TEST_ASSERT_EQUAL_UINT8(0, b.machine.parametres().plancher_chauffant);
    const Mesures m = mesures(34.0f, 38.0f);        // AUCUNE sonde de film
    b.demarrer(m);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    TEST_ASSERT_TRUE(b.s.chauffe);
    TEST_ASSERT_FALSE(b.s.film);
    TEST_ASSERT_EQUAL_UINT8(FILM_INACTIF, b.machine.film().raisons());
    b.tourner(m, 10 * MIN);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    TEST_ASSERT_EQUAL_UINT32(DEF_AUCUN, b.secu.defauts());
    TEST_ASSERT_TRUE(b.s.chauffe);
    // Une sonde de film branchée ne commande rien tant que la variante est désactivée.
    b.tourner(mesures_film(34.0f, 38.0f, 30.0f), MIN);
    TEST_ASSERT_TRUE(b.s.chauffe);
    TEST_ASSERT_FALSE(b.s.film);
}

void test_plancher_chauffant_suit_la_demande_du_toit() {
    Banc b;
    b.activer_plancher_chauffant();
    b.demarrer(mesures_film(34.0f, 38.0f, 30.0f));
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    TEST_ASSERT_TRUE(b.s.chauffe);
    TEST_ASSERT_TRUE(b.s.film);
    b.pas(mesures_film(42.6f, 43.0f, 40.0f));       // consigne atteinte (TOR) : les deux coupés
    TEST_ASSERT_FALSE(b.s.chauffe);
    TEST_ASSERT_FALSE(b.s.film);
    TEST_ASSERT_TRUE(b.machine.film().raisons() & FILM_DEMANDE);
    b.pas(mesures_film(42.0f, 43.0f, 40.0f));       // <= 42,1 : les deux repartent
    TEST_ASSERT_TRUE(b.s.chauffe);
    TEST_ASSERT_TRUE(b.s.film);
    Mesures m = mesures_film(41.0f, 43.0f, 40.0f);  // limite couvain 43,5 °C : coupe aussi le film
    fixer_couvain(m, 2, 43.6f);
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.chauffe);
    TEST_ASSERT_FALSE(b.s.film);
    b.pas(mesures_film(40.0f, 44.1f, 40.0f));       // limite air 44,0 °C : coupe aussi le film
    TEST_ASSERT_FALSE(b.s.chauffe);
    TEST_ASSERT_FALSE(b.s.film);
    b.pas(mesures_film(40.0f, 43.4f, 40.0f));
    TEST_ASSERT_TRUE(b.s.chauffe);
    TEST_ASSERT_TRUE(b.s.film);
}

void test_plancher_chauffant_limite_50_sans_couper_le_toit() {
    Banc b;
    b.activer_plancher_chauffant();
    b.demarrer(mesures_film(34.0f, 38.0f, 30.0f));
    b.pas(mesures_film(36.0f, 40.0f, 50.5f));
    TEST_ASSERT_TRUE(b.s.chauffe);                  // le toit continue
    TEST_ASSERT_FALSE(b.s.film);
    TEST_ASSERT_TRUE(b.machine.film().raisons() & FILM_LIMITE);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());  // limite non bloquante
    b.pas(mesures_film(36.0f, 40.0f, 49.0f));       // hystérésis : reste coupé
    TEST_ASSERT_FALSE(b.s.film);
    b.pas(mesures_film(36.0f, 40.0f, 47.9f));
    TEST_ASSERT_TRUE(b.s.film);
    // Limite évaluée aussi sur la valeur brute (filtre médian en retard).
    Mesures m = mesures_film(36.0f, 40.0f, 47.0f);
    appliquer_lecture(m.t_film, true, 50.4f);
    TEST_ASSERT_TRUE(m.t_film.t < 50.0f);
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.film);
}

void test_plancher_chauffant_defaut_55_verrouille() {
    Banc b;
    b.activer_plancher_chauffant();
    b.demarrer(mesures_film(34.0f, 38.0f, 30.0f));
    const Mesures m = mesures_film(36.0f, 40.0f, 55.5f);  // MOSFET collé simulé
    b.tourner(m, 8000);
    TEST_ASSERT_EQUAL(Etat::MONTEE, b.machine.etat());
    TEST_ASSERT_FALSE(b.s.film);                    // déjà coupé par la limite 50 °C
    b.tourner(m, 4000);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.secu.defauts() & DEF_FILM_SURTEMP);
    TEST_ASSERT_FALSE(b.s.chauffe);
    TEST_ASSERT_FALSE(b.s.film);
    TEST_ASSERT_TRUE(b.s.ouvrir_relais_serie);      // C4 ouverte -> K2 coupe le 24 V du film
    TEST_ASSERT_EQUAL_UINT8(defauts::PWM_BRASSAGE_PCT, b.s.pwm_pct[VENTILO_PLANCHER_A]);
}

void test_plancher_chauffant_interdit_si_soufflantes_plancher_arretees() {
    Banc b;
    b.activer_plancher_chauffant();
    b.demarrer(mesures_film(34.0f, 38.0f, 30.0f));
    TEST_ASSERT_TRUE(b.s.film);
    Mesures m = mesures_film(34.0f, 38.0f, 30.0f);
    m.rpm[VENTILO_PLANCHER_B] = 0;                  // soufflante arrière bloquée
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.film);                    // coupé dès le pas suivant ...
    TEST_ASSERT_TRUE(b.s.chauffe);                  // ... avant le DÉFAUT ventilateur (10 s)
    TEST_ASSERT_TRUE(b.machine.film().raisons() & FILM_SOUFFLANTES);
    m.rpm[VENTILO_PLANCHER_B] = 1500;               // lente : < 50 % de 4800
    b.pas(m);
    TEST_ASSERT_FALSE(b.s.film);
    b.tourner(m, 12000);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.secu.defauts() & DEF_VENTILO_PLANCHER_B);
    TEST_ASSERT_FALSE(b.s.film);
    // Variante active avec soufflantes de plancher désactivées : départ refusé.
    Banc b2;
    Parametres p = parametres_defaut();
    p.plancher_chauffant = 1;
    p.pwm_plancher_pct = 0;
    TEST_ASSERT_TRUE(b2.machine.changer_parametres(p));
    Mesures m2 = mesures_film(34.0f, 38.0f, 30.0f);
    m2.rpm[VENTILO_PLANCHER_A] = m2.rpm[VENTILO_PLANCHER_B] = 0;
    b2.depart(m2);
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b2.machine.etat());
    TEST_ASSERT_TRUE(b2.machine.dernier_autotest() & AT_FILM_SANS_SOUFFLANTES);
    TEST_ASSERT_FALSE(b2.s.film);
}

void test_plancher_chauffant_sonde_film_exigee_si_active() {
    // Départ refusé sans sonde de film (variante active).
    Banc b;
    b.activer_plancher_chauffant();
    b.depart(mesures(34.0f, 38.0f));
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b.machine.etat());
    TEST_ASSERT_TRUE(b.machine.dernier_autotest() & AT_SONDE_FILM);
    // Sonde perdue en cycle : DÉFAUT `sonde_film`, tout est coupé.
    Banc b2;
    b2.activer_plancher_chauffant();
    b2.demarrer(mesures_film(34.0f, 38.0f, 30.0f));
    TEST_ASSERT_EQUAL(Etat::MONTEE, b2.machine.etat());
    b2.pas(mesures(34.0f, 38.0f));
    TEST_ASSERT_EQUAL(Etat::DEFAUT, b2.machine.etat());
    TEST_ASSERT_TRUE(b2.secu.defauts() & DEF_SONDE_FILM);
    TEST_ASSERT_FALSE(b2.s.chauffe);
    TEST_ASSERT_FALSE(b2.s.film);
}

void test_plancher_chauffant_freine_la_redescente() {
    Banc b;
    b.activer_plancher_chauffant();
    b.demarrer(mesures_film(34.0f, 38.0f, 30.0f));
    const Mesures pal = mesures_film(42.1f, 43.0f, 45.0f);
    b.tourner(pal, 5 * MIN + PAS_MS);
    b.tourner(pal, 120 * MIN + PAS_MS);
    TEST_ASSERT_EQUAL(Etat::REFROIDISSEMENT, b.machine.etat());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 45.0f, b.machine.resume().tmax_film);  // T surface max (résumé)
    b.tourner(mesures_film(41.0f, 41.0f, 40.0f), 20 * MIN);
    b.pas(mesures_film(39.0f, 39.0f, 40.0f));       // chute plus rapide que la rampe : freinage
    TEST_ASSERT_TRUE(b.s.chauffe);
    TEST_ASSERT_TRUE(b.s.film);
    b.pas(mesures_film(40.6f, 40.6f, 40.0f));       // au-dessus de la rampe : rien
    TEST_ASSERT_FALSE(b.s.chauffe);
    TEST_ASSERT_FALSE(b.s.film);
}

void test_plancher_chauffant_modifiable_seulement_en_attente() {
    Banc b;
    b.demarrer(mesures(34.0f, 38.0f));
    Parametres p = b.machine.parametres();
    p.plancher_chauffant = 1;
    TEST_ASSERT_FALSE(b.machine.changer_parametres(p));
    TEST_ASSERT_EQUAL_UINT8(0, b.machine.parametres().plancher_chauffant);
    b.tourner(mesures_film(34.0f, 38.0f, 30.0f), MIN);
    TEST_ASSERT_FALSE(b.s.film);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_demarrage_en_attente_sans_chauffe);
    RUN_TEST(test_depart_nominal_passe_par_autotest_puis_montee);
    RUN_TEST(test_depart_refuse_peigne_absent);
    RUN_TEST(test_depart_refuse_sonde_manquante_sur_p1);
    RUN_TEST(test_depart_refuse_peigne_bord_sur_embase_centre);
    RUN_TEST(test_depart_refuse_sonde_non_etalonnee);
    RUN_TEST(test_depart_refuse_c4_ouverte);
    RUN_TEST(test_depart_refuse_couvain_hors_plage);
    RUN_TEST(test_depart_refuse_parametres_corrompus);
    RUN_TEST(test_depart_refuse_rtc_invalide);
    RUN_TEST(test_autotest_ventilo_plancher_bloque);
    RUN_TEST(test_plancher_desactive_pour_essai);
    RUN_TEST(test_avertissement_ambiance_non_bloquant);
    RUN_TEST(test_arret_pendant_autotest_retour_attente);
    RUN_TEST(test_cycle_nominal_complet);
    RUN_TEST(test_palier_exige_toutes_les_sondes);
    RUN_TEST(test_stabilite_remise_a_zero_si_chute);
    RUN_TEST(test_palier_cumulatif_ne_compte_pas_hors_plage);
    RUN_TEST(test_palier_ne_compte_pas_si_sonde_chaude_au_dessus_43_5);
    RUN_TEST(test_palier_perdu_apres_30_min_hors_plage);
    RUN_TEST(test_timeout_montee);
    RUN_TEST(test_chauffe_inefficace);
    RUN_TEST(test_homogeneite_insuffisante);
    RUN_TEST(test_arret_operateur_en_montee);
    RUN_TEST(test_refroidissement_timeout_avertissement);
    RUN_TEST(test_redescente_pilotee_freine_et_suit_la_rampe);
    RUN_TEST(test_redescente_cible_bornee_a_33_sur_banc_froid);
    RUN_TEST(test_regulation_sur_la_plus_froide);
    RUN_TEST(test_hysteresis_tor);
    RUN_TEST(test_limite_air_coupe_la_chauffe_sans_defaut);
    RUN_TEST(test_limite_air_sur_valeur_brute);
    RUN_TEST(test_limite_couvain_coupe_la_chauffe);
    RUN_TEST(test_defaut_verrouille_jusqu_a_acquittement);
    RUN_TEST(test_acquittement_refuse_si_cause_presente);
    RUN_TEST(test_changer_parametres_seulement_en_attente);
    RUN_TEST(test_resume_cycle_en_defaut);
    RUN_TEST(test_plancher_chauffant_desactive_comportement_inchange);
    RUN_TEST(test_plancher_chauffant_suit_la_demande_du_toit);
    RUN_TEST(test_plancher_chauffant_limite_50_sans_couper_le_toit);
    RUN_TEST(test_plancher_chauffant_defaut_55_verrouille);
    RUN_TEST(test_plancher_chauffant_interdit_si_soufflantes_plancher_arretees);
    RUN_TEST(test_plancher_chauffant_sonde_film_exigee_si_active);
    RUN_TEST(test_plancher_chauffant_freine_la_redescente);
    RUN_TEST(test_plancher_chauffant_modifiable_seulement_en_attente);
    return UNITY_END();
}
