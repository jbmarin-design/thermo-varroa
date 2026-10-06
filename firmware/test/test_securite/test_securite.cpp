// Tests unitaires — sécurité logicielle C2/C3, régulation, capteurs, paramètres (env native, Unity)
#include <unity.h>

#include <cmath>

#include "../aides_test.h"
#include "conversions.h"
#include "enable_ssr.h"
#include "parametres_defaut.h"
#include "regulation.h"

using namespace tv;
using namespace aides;

void setUp() {}
void tearDown() {}

ContexteSecurite ctx_cycle(bool ssr) {
    ContexteSecurite c;
    c.cycle_actif = true;
    c.ssr_commande = ssr;
    c.pwm_pct[0] = 80; c.pwm_pct[1] = 80; c.pwm_pct[2] = 80;
    c.rpm_nominal[0] = 4000; c.rpm_nominal[1] = 6000; c.rpm_nominal[2] = 6000;
    return c;
}

void evaluer_pendant(Securite& s, const Mesures& m, const ContexteSecurite& c, uint32_t& t, uint32_t duree) {
    for (uint32_t e = 0; e <= duree; e += PAS_MS) { s.evaluer(m, c, t); t += PAS_MS; }
}

// --- Seuils de surtempérature -----------------------------------------------------------------

void test_air_44_5_pendant_10s_defaut() {
    Securite s;
    uint32_t t = 0;
    const Mesures m = mesures(42.0f, 44.5f);
    evaluer_pendant(s, m, ctx_cycle(false), t, 8000);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    evaluer_pendant(s, m, ctx_cycle(false), t, 2000);
    TEST_ASSERT_TRUE(s.defauts() & DEF_AIR_SURTEMP);
    TEST_ASSERT_FALSE(s.autorise_chauffe());
    TEST_ASSERT_TRUE(s.ouvrir_relais_serie());
    TEST_ASSERT_TRUE(s.brassage_requis());
}

void test_air_surtemp_meme_hors_cycle() {
    Securite s;
    uint32_t t = 0;
    ContexteSecurite c;  // cycle inactif (ex. SSR collé en ATTENTE)
    evaluer_pendant(s, mesures(30.0f, 44.6f), c, t, 12000);
    TEST_ASSERT_TRUE(s.defauts() & DEF_AIR_SURTEMP);
}

void test_air_44_4_pas_de_defaut() {
    Securite s;
    uint32_t t = 0;
    evaluer_pendant(s, mesures(42.0f, 44.4f), ctx_cycle(false), t, 10 * MIN);
    TEST_ASSERT_EQUAL_UINT32(DEF_AUCUN, s.defauts() & DEF_AIR_SURTEMP);
}

void test_couvain_44_pendant_60s_defaut() {
    Securite s;
    uint32_t t = 0;
    Mesures m = mesures(42.0f, 43.0f);
    fixer_couvain(m, 3, 44.0f);
    evaluer_pendant(s, m, ctx_cycle(false), t, 58000);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    evaluer_pendant(s, m, ctx_cycle(false), t, 2000);
    TEST_ASSERT_TRUE(s.defauts() & DEF_COUVAIN_SURTEMP);
}

void test_couvain_compteur_remis_a_zero() {
    Securite s;
    uint32_t t = 0;
    Mesures chaud = mesures(42.0f, 43.0f);
    fixer_couvain(chaud, 0, 44.1f);
    const Mesures normal = mesures(42.0f, 43.0f);
    evaluer_pendant(s, chaud, ctx_cycle(false), t, 50000);
    s.evaluer(normal, ctx_cycle(false), t); t += PAS_MS;
    evaluer_pendant(s, chaud, ctx_cycle(false), t, 50000);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
}

void test_valeur_brute_45_trois_fois() {
    Securite s;
    uint32_t t = 0;
    Mesures m = mesures(42.0f, 43.0f);
    for (int i = 0; i < 2; ++i) {
        appliquer_lecture(m.t_air, true, 45.3f);
        consolider(m);
        s.evaluer(m, ctx_cycle(false), t); t += PAS_MS;
    }
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    appliquer_lecture(m.t_air, true, 45.3f);
    consolider(m);
    s.evaluer(m, ctx_cycle(false), t);
    TEST_ASSERT_TRUE(s.defauts() & DEF_BRUTE_45);
}

void test_duree_max_chauffe() {
    Securite s;
    uint32_t t = 0;
    const Mesures m = mesures(40.0f, 42.0f);
    evaluer_pendant(s, m, ctx_cycle(true), t, defauts::DUREE_CHAUFFE_MAX_MIN * MIN - 10000);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    evaluer_pendant(s, m, ctx_cycle(true), t, 12000);
    TEST_ASSERT_TRUE(s.defauts() & DEF_DUREE_MAX);
}

// --- Sondes / peignes --------------------------------------------------------------------------

void test_peigne_debranche_defaut_immediat() {
    Securite s;
    uint32_t t = 0;
    Mesures m = mesures(40.0f, 42.0f);
    m.embases[EMBASE_P1].presence = false;
    consolider(m);
    s.evaluer(m, ctx_cycle(true), t);
    TEST_ASSERT_TRUE(s.defauts() & DEF_PEIGNE_P1);
}

void test_sonde_air_invalide_defaut() {
    Securite s;
    uint32_t t = 0;
    Mesures m = mesures(40.0f, 42.0f);
    for (int i = 0; i < 3; ++i) appliquer_lecture(m.t_air, false, NAN);
    consolider(m);
    TEST_ASSERT_FALSE(m.t_air.valide);
    s.evaluer(m, ctx_cycle(true), t);
    TEST_ASSERT_TRUE(s.defauts() & DEF_SONDE_AIR);
}

void test_sonde_absente_tolere_deux_lectures() {
    Mesures m = mesures(40.0f, 42.0f);
    LectureSonde& l = m.embases[EMBASE_P2].sondes[0];
    appliquer_lecture(l, false, NAN);
    appliquer_lecture(l, false, NAN);
    consolider(m);
    TEST_ASSERT_TRUE(m.embases[EMBASE_P2].conforme);
    appliquer_lecture(l, false, NAN);
    consolider(m);
    TEST_ASSERT_FALSE(m.embases[EMBASE_P2].conforme);
}

void test_valeurs_usine_rejetees() {
    LectureSonde l;
    l.presente = true;
    appliquer_lecture(l, true, 85.0f);
    TEST_ASSERT_FALSE(l.valide);
    appliquer_lecture(l, true, -127.0f);
    appliquer_lecture(l, true, 61.0f);
    TEST_ASSERT_EQUAL_UINT8(3, l.echecs);
    TEST_ASSERT_FALSE(l.valide);
    appliquer_lecture(l, true, 30.0f);
    TEST_ASSERT_TRUE(l.valide);
}

void test_sonde_en_trop_non_conforme() {
    Mesures m = mesures(40.0f, 42.0f);
    m.embases[EMBASE_P2].nb_trouvees = 2;  // deux sondes sur un peigne de bord
    consolider(m);
    TEST_ASSERT_FALSE(m.embases[EMBASE_P2].conforme);
}

void test_fusion_sondes_detecte_nouvelle_et_absente() {
    LectureSonde slots[MAX_SONDES_PAR_EMBASE];
    uint8_t nb = 0;
    uint8_t roms[3][8];
    for (uint8_t i = 0; i < 3; ++i) rom_test(roms[i], static_cast<uint8_t>(i + 1));
    TEST_ASSERT_EQUAL_UINT8(3, fusionner_sondes(slots, nb, 3, roms, 3));
    TEST_ASSERT_EQUAL_UINT8(3, nb);
    for (uint8_t i = 0; i < 3; ++i) { slots[i].presente = true; appliquer_lecture(slots[i], true, 35.0f); }
    // La sonde 2 disparaît du balayage : conservée tant que < 3 échecs.
    uint8_t deux[2][8];
    rom_test(deux[0], 1); rom_test(deux[1], 3);
    fusionner_sondes(slots, nb, 3, deux, 2);
    TEST_ASSERT_EQUAL_UINT8(3, nb);
    TEST_ASSERT_FALSE(slots[1].presente);
    for (int k = 0; k < 3; ++k) {
        for (uint8_t i = 0; i < nb; ++i) appliquer_lecture(slots[i], slots[i].presente, 35.0f);
        fusionner_sondes(slots, nb, 3, deux, 2);
    }
    TEST_ASSERT_EQUAL_UINT8(2, nb);
}

void test_ordonner_sondes_par_position() {
    LectureSonde s[3];
    rom_test(s[0].rom, 9); s[0].position = Position::P1_BAS;
    rom_test(s[1].rom, 8); s[1].position = Position::P1_HAUT;
    rom_test(s[2].rom, 7); s[2].position = Position::P1_CENTRE;
    ordonner_sondes(s, 3);
    TEST_ASSERT_EQUAL(Position::P1_HAUT, s[0].position);
    TEST_ASSERT_EQUAL(Position::P1_CENTRE, s[1].position);
    TEST_ASSERT_EQUAL(Position::P1_BAS, s[2].position);
}

// --- Ventilateurs -------------------------------------------------------------------------------------

void test_ventilo_bloque_defaut_apres_delai() {
    Securite s;
    uint32_t t = 0;
    Mesures m = mesures(40.0f, 42.0f);
    evaluer_pendant(s, m, ctx_cycle(true), t, 10000);
    m.rpm[VENTILO_TOIT] = 0;
    evaluer_pendant(s, m, ctx_cycle(true), t, 6000);   // 4 pas = 8 s cumulées
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    evaluer_pendant(s, m, ctx_cycle(true), t, 0);      // 5e pas = 10 s
    TEST_ASSERT_TRUE(s.defauts() & DEF_VENTILO_TOIT);
    TEST_ASSERT_FALSE(s.brassage_requis());
}

void test_ventilo_lent_moins_de_50_pct() {
    TEST_ASSERT_TRUE(Securite::ventilo_ok(1600, 80, 4000));   // 50 % de 3200
    TEST_ASSERT_FALSE(Securite::ventilo_ok(1500, 80, 4000));
    TEST_ASSERT_TRUE(Securite::ventilo_ok(0, 0, 4000));       // arrêt volontaire
}

void test_delai_demarrage_ventilo_apres_changement_pwm() {
    Securite s;
    uint32_t t = 0;
    Mesures m = mesures(40.0f, 42.0f);
    m.rpm[VENTILO_PLANCHER_A] = 0;
    // Pendant les 5 s de démarrage puis 10 s de mesure : pas de défaut avant 15 s.
    evaluer_pendant(s, m, ctx_cycle(true), t, 12000);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
}

// --- C4 et SSR collé ----------------------------------------------------------------------------------

void test_c4_ouverte_en_cycle() {
    Securite s;
    uint32_t t = 0;
    Mesures m = mesures(40.0f, 42.0f);
    m.c4_fermee = false;
    s.evaluer(m, ctx_cycle(true), t);
    TEST_ASSERT_TRUE(s.defauts() & DEF_C4_OUVERTE);
    TEST_ASSERT_FALSE(s.ouvrir_relais_serie());  // pas une surtempérature
}

void test_ssr_colle_air_qui_remonte() {
    Securite s;
    uint32_t t = 0;
    s.evaluer(mesures(40.0f, 43.0f), ctx_cycle(true), t); t += PAS_MS;
    // Commande à 0 : délai de grâce (inertie élément), l'air peut encore monter.
    evaluer_pendant(s, mesures(40.0f, 43.8f), ctx_cycle(false), t, defauts::SSR_COLLE_GRACE_S * 1000u);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    // Après la grâce : l'air redescend puis remonte de > 0,5 °C sans commande.
    evaluer_pendant(s, mesures(40.0f, 42.0f), ctx_cycle(false), t, 20000);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    evaluer_pendant(s, mesures(40.0f, 42.6f), ctx_cycle(false), t, 4000);
    TEST_ASSERT_TRUE(s.defauts() & DEF_SSR_COLLE);
    TEST_ASSERT_TRUE(s.ouvrir_relais_serie());
}

void test_ssr_pas_de_faux_positif_si_decroissance() {
    Securite s;
    uint32_t t = 0;
    float ta = 43.5f;
    for (uint32_t e = 0; e < 30 * MIN; e += PAS_MS) {
        ta -= 0.002f;
        s.evaluer(mesures(41.0f, ta), ctx_cycle(false), t);
        t += PAS_MS;
    }
    TEST_ASSERT_TRUE(s.autorise_chauffe());
}

void test_ssr_colle_en_refroidissement_couvain() {
    Securite s;
    uint32_t t = 0;
    ContexteSecurite c = ctx_cycle(false);
    c.refroidissement = true;
    c.pwm_pct[0] = c.pwm_pct[1] = c.pwm_pct[2] = 0;
    evaluer_pendant(s, mesures(42.0f, 41.0f), c, t, defauts::REFROID_GRACE_COUVAIN_MIN * MIN);
    evaluer_pendant(s, mesures(41.5f, 41.0f), c, t, 5 * MIN);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
    evaluer_pendant(s, mesures(42.1f, 41.0f), c, t, 4000);
    TEST_ASSERT_TRUE(s.defauts() & DEF_SSR_COLLE);
}

void test_redescente_pilotee_chauffe_de_freinage_pas_ssr_colle() {
    // En REFROIDISSEMENT, une remontée du couvain PENDANT une chauffe commandée (freinage
    // de la redescente) n'est pas un SSR collé.
    Securite s;
    uint32_t t = 0;
    ContexteSecurite c = ctx_cycle(true);
    c.refroidissement = true;
    evaluer_pendant(s, mesures(40.0f, 41.0f), c, t, defauts::REFROID_GRACE_COUVAIN_MIN * MIN);
    evaluer_pendant(s, mesures(39.5f, 41.0f), c, t, 2 * MIN);
    evaluer_pendant(s, mesures(40.2f, 41.0f), c, t, 2 * MIN);
    TEST_ASSERT_EQUAL_UINT32(0, s.defauts() & DEF_SSR_COLLE);
    TEST_ASSERT_TRUE(s.autorise_chauffe());
}

// --- Acquittement ------------------------------------------------------------------------------------

void test_acquittement_surtemp_exige_retour_sous_limite() {
    Securite s;
    uint32_t t = 0;
    evaluer_pendant(s, mesures(42.0f, 44.8f), ctx_cycle(false), t, 12000);
    TEST_ASSERT_FALSE(s.acquitter(mesures(42.0f, 44.2f)));  // encore > 44,0
    TEST_ASSERT_TRUE(s.acquitter(mesures(42.0f, 43.9f)));
}

void test_parametres_jamais_acquittes() {
    Securite s;
    s.declarer(DEF_PARAMETRES);
    TEST_ASSERT_FALSE(s.acquitter(mesures(30.0f, 30.0f)));
}

// --- Régulation (unitaire) --------------------------------------------------------------------------

void test_regulation_sans_donnees_coupe() {
    RegulationTOR r;
    EntreeRegulation e;
    e.couvain_valide = false;
    e.air_valide = true;
    e.t_couvain_min = 30.0f;
    e.consigne = 42.3f;
    e.hyst_bas = 0.2f;
    e.hyst_haut = 0.1f;
    TEST_ASSERT_FALSE(r.calculer(e));
    TEST_ASSERT_TRUE(r.raisons() & COUPURE_DONNEES);
}

void test_fenetre_puissance() {
    TEST_ASSERT_TRUE(fenetre_puissance(9999, 100));
    TEST_ASSERT_TRUE(fenetre_puissance(0, 50));
    TEST_ASSERT_TRUE(fenetre_puissance(4999, 50));
    TEST_ASSERT_FALSE(fenetre_puissance(5000, 50));
    TEST_ASSERT_FALSE(fenetre_puissance(9999, 50));
    TEST_ASSERT_TRUE(fenetre_puissance(10000, 50));
}

// --- Enable dynamique (C3) -------------------------------------------------------------------------

void test_enable_dynamique_signal_carre() {
    EnableDynamique en;
    en.commande(true);
    bool a = en.tic(), b = en.tic(), c = en.tic();
    TEST_ASSERT_TRUE(a != b && b != c);
}

void test_enable_dynamique_coupe_si_jeton_perime() {
    EnableDynamique en;
    en.commande(true);
    for (uint32_t i = 0; i < defauts::JETON_SSR_MAX_AGE_MS - 1; ++i) en.tic();
    TEST_ASSERT_TRUE(en.actif());
    for (int i = 0; i < 5; ++i) en.tic();
    TEST_ASSERT_FALSE(en.actif());
    TEST_ASSERT_FALSE(en.tic());
    TEST_ASSERT_FALSE(en.tic());                    // niveau figé BAS : la pompe de charge se vide
}

void test_enable_dynamique_sans_commande() {
    EnableDynamique en;
    for (int i = 0; i < 10; ++i) TEST_ASSERT_FALSE(en.tic());
    en.commande(false);
    for (int i = 0; i < 10; ++i) TEST_ASSERT_FALSE(en.tic());
}

// --- Paramètres --------------------------------------------------------------------------------------

void test_parametres_bornes_figees() {
    Parametres p = parametres_defaut();
    TEST_ASSERT_TRUE(parametres_valides(p));
    p.consigne = 45.0f;
    TEST_ASSERT_TRUE(parametres_borner(p));
    TEST_ASSERT_TRUE(p.consigne + p.hyst_haut < defauts::T_COEUR_MAX_REG);
    p.consigne = NAN;
    parametres_borner(p);
    TEST_ASSERT_EQUAL_FLOAT(defauts::T_CONSIGNE_PALIER, p.consigne);
    p.puissance_max_pct = 5;
    parametres_borner(p);
    TEST_ASSERT_EQUAL_UINT8(defauts::PUISSANCE_BORNE_MIN_PCT, p.puissance_max_pct);
}

void test_parametres_crc() {
    Parametres p = parametres_defaut();
    p.duree_palier_min = 100;  // modifié sans sceller
    TEST_ASSERT_FALSE(parametres_valides(p));
    parametres_sceller(p);
    TEST_ASSERT_TRUE(parametres_valides(p));
}

void test_table_etalonnage() {
    TableEtalonnage t;
    uint8_t rom[8];
    rom_test(rom, 4);
    TEST_ASSERT_TRUE(t.definir(rom, 0.25f, Position::P2));
    TEST_ASSERT_FALSE(t.definir(rom, 1.5f, Position::P2));  // > 1,0 °C : sonde suspecte
    t.sceller();
    TEST_ASSERT_TRUE(t.valide());
    t.e[0].offset = 0.3f;
    TEST_ASSERT_FALSE(t.valide());
}

// --- Conversions ---------------------------------------------------------------------------------------

void test_ds18b20_decodage_et_crc() {
    // 0x0191 = 401 -> 25,0625 °C ; CRC calculé.
    uint8_t sp[9] = {0x91, 0x01, 0x4B, 0x46, 0x7F, 0xFF, 0x0F, 0x10, 0x00};
    sp[8] = crc8_dallas(sp, 8);
    float t = 0;
    TEST_ASSERT_TRUE(ds18b20_decoder(sp, t));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.0625f, t);
    sp[0] ^= 1;
    TEST_ASSERT_FALSE(ds18b20_decoder(sp, t));
}

void test_sht45_crc_exemple_datasheet() {
    const uint8_t mot[2] = {0xBE, 0xEF};
    TEST_ASSERT_EQUAL_HEX8(0x92, crc8_sensirion(mot, 2));
}

void test_ntc_25_degres() {
    const float t = ntc_temperature(3300.0f * 100000.0f / 147000.0f, 3300.0f, 47000.0f, 100000.0f, 3950.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 25.0f, t);
    TEST_ASSERT_TRUE(std::isnan(ntc_temperature(5.0f, 3300.0f, 47000.0f, 100000.0f, 3950.0f)));
}

void test_tachy_rpm() {
    TEST_ASSERT_EQUAL_UINT16(3000, tachy_rpm(100, 1000, 2));
    TEST_ASSERT_EQUAL_UINT16(0, tachy_rpm(100, 0, 2));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_air_44_5_pendant_10s_defaut);
    RUN_TEST(test_air_surtemp_meme_hors_cycle);
    RUN_TEST(test_air_44_4_pas_de_defaut);
    RUN_TEST(test_couvain_44_pendant_60s_defaut);
    RUN_TEST(test_couvain_compteur_remis_a_zero);
    RUN_TEST(test_valeur_brute_45_trois_fois);
    RUN_TEST(test_duree_max_chauffe);
    RUN_TEST(test_peigne_debranche_defaut_immediat);
    RUN_TEST(test_sonde_air_invalide_defaut);
    RUN_TEST(test_sonde_absente_tolere_deux_lectures);
    RUN_TEST(test_valeurs_usine_rejetees);
    RUN_TEST(test_sonde_en_trop_non_conforme);
    RUN_TEST(test_fusion_sondes_detecte_nouvelle_et_absente);
    RUN_TEST(test_ordonner_sondes_par_position);
    RUN_TEST(test_ventilo_bloque_defaut_apres_delai);
    RUN_TEST(test_ventilo_lent_moins_de_50_pct);
    RUN_TEST(test_delai_demarrage_ventilo_apres_changement_pwm);
    RUN_TEST(test_c4_ouverte_en_cycle);
    RUN_TEST(test_ssr_colle_air_qui_remonte);
    RUN_TEST(test_ssr_pas_de_faux_positif_si_decroissance);
    RUN_TEST(test_ssr_colle_en_refroidissement_couvain);
    RUN_TEST(test_redescente_pilotee_chauffe_de_freinage_pas_ssr_colle);
    RUN_TEST(test_acquittement_surtemp_exige_retour_sous_limite);
    RUN_TEST(test_parametres_jamais_acquittes);
    RUN_TEST(test_regulation_sans_donnees_coupe);
    RUN_TEST(test_fenetre_puissance);
    RUN_TEST(test_enable_dynamique_signal_carre);
    RUN_TEST(test_enable_dynamique_coupe_si_jeton_perime);
    RUN_TEST(test_enable_dynamique_sans_commande);
    RUN_TEST(test_parametres_bornes_figees);
    RUN_TEST(test_parametres_crc);
    RUN_TEST(test_table_etalonnage);
    RUN_TEST(test_ds18b20_decodage_et_crc);
    RUN_TEST(test_sht45_crc_exemple_datasheet);
    RUN_TEST(test_ntc_25_degres);
    RUN_TEST(test_tachy_rpm);
    return UNITY_END();
}
