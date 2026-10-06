// Tests unitaires — journal CSV et IHM (env native, Unity)
#include <unity.h>

#include <cstring>

#include "../aides_test.h"
#include "ihm.h"
#include "journal.h"

using namespace tv;
using namespace aides;

void setUp() {}
void tearDown() {}

static int compter(const char* s, char c) {
    int n = 0;
    for (; *s; ++s) n += (*s == c);
    return n;
}

void test_entete_et_ligne_meme_nombre_de_colonnes() {
    char entete[640], ligne[640];
    journal_entete_colonnes(entete, sizeof entete);
    Mesures m = mesures(41.2f, 43.1f);
    ContexteLigne c;
    c.h.valide = true;
    c.h.annee = 2026; c.h.mois = 9; c.h.jour = 5; c.h.heure = 14; c.h.minute = 3; c.h.seconde = 7;
    c.etat = Etat::MONTEE;
    c.consigne = 42.3f;
    journal_ligne_mesures(m, c, ligne, sizeof ligne);
    TEST_ASSERT_EQUAL_INT(compter(entete, ';'), compter(ligne, ';'));
    TEST_ASSERT_NOT_NULL(std::strstr(ligne, "M;2026-09-05T14:03:07;"));
    TEST_ASSERT_NOT_NULL(std::strstr(ligne, ";MONTEE;42.30;41.20;41.20;"));
}

void test_valeur_invalide_champ_vide() {
    char ligne[640];
    Mesures m = mesures(41.0f, 43.0f);
    m.sht_valide = false;
    ContexteLigne c;
    journal_ligne_mesures(m, c, ligne, sizeof ligne);
    TEST_ASSERT_NOT_NULL(std::strstr(ligne, ";;"));
    TEST_ASSERT_NOT_NULL(std::strstr(ligne, "0000-00-00T00:00:00"));
}

void test_troncature_sans_debordement() {
    char petit[32];
    std::memset(petit, 'X', sizeof petit);
    Mesures m = mesures(41.0f, 43.0f);
    ContexteLigne c;
    const int n = journal_ligne_mesures(m, c, petit, sizeof petit);
    TEST_ASSERT_TRUE(n < 32);
    TEST_ASSERT_EQUAL_CHAR('\0', petit[n]);
}

void test_evenement_neutralise_les_separateurs() {
    char l[128];
    Horodatage h;
    journal_ligne_evenement(h, 1234, Etat::DEFAUT, "TEST", "a;b\nc", l, sizeof l);
    TEST_ASSERT_EQUAL_STRING("E;0000-00-00T00:00:00;1234;DEFAUT;TEST;a,b,c", l);
}

void test_detail_defauts() {
    char d[128];
    defauts_detail(DEF_AIR_SURTEMP | DEF_PEIGNE_P2, d, sizeof d);
    TEST_ASSERT_EQUAL_STRING("air_surtemp|peigne_P2", d);
    defauts_detail(0, d, sizeof d);
    TEST_ASSERT_EQUAL_STRING("aucun", d);
}

void test_metadonnees() {
    char l[256];
    const Parametres p = parametres_defaut();
    TEST_ASSERT_TRUE(journal_metadonnees(0, p, "x", l, sizeof l) > 0);
    TEST_ASSERT_EQUAL_CHAR('#', l[0]);
    TEST_ASSERT_TRUE(journal_metadonnees(2, p, "x", l, sizeof l) > 0);
    TEST_ASSERT_NOT_NULL(std::strstr(l, "consigne=42.30"));
    TEST_ASSERT_EQUAL_INT(0, journal_metadonnees(99, p, "x", l, sizeof l));
}

// --- IHM -----------------------------------------------------------------------------------------------

void test_bouton_appui_court() {
    Bouton b;
    uint32_t t = 0;
    TEST_ASSERT_EQUAL(Appui::AUCUN, b.mettre_a_jour(true, t));
    t += 60; TEST_ASSERT_EQUAL(Appui::AUCUN, b.mettre_a_jour(true, t));
    t += 300; TEST_ASSERT_EQUAL(Appui::AUCUN, b.mettre_a_jour(false, t));
    t += 60; TEST_ASSERT_EQUAL(Appui::COURT, b.mettre_a_jour(false, t));
}

void test_bouton_rebond_ignore() {
    Bouton b;
    uint32_t t = 0;
    b.mettre_a_jour(true, t);
    t += 10; b.mettre_a_jour(false, t);
    t += 10; b.mettre_a_jour(true, t);
    t += 10; TEST_ASSERT_EQUAL(Appui::AUCUN, b.mettre_a_jour(false, t));
    t += 100; TEST_ASSERT_EQUAL(Appui::AUCUN, b.mettre_a_jour(false, t));
}

void test_bouton_appui_long_sans_court() {
    Bouton b;
    uint32_t t = 0;
    Appui vu = Appui::AUCUN;
    for (; t < 3200; t += 20) {
        const Appui a = b.mettre_a_jour(true, t);
        if (a != Appui::AUCUN) vu = a;
    }
    TEST_ASSERT_EQUAL(Appui::LONG, vu);
    for (int i = 0; i < 10; ++i, t += 20) TEST_ASSERT_NOT_EQUAL(Appui::COURT, b.mettre_a_jour(false, t));
}

void test_commandes_selon_etat() {
    TEST_ASSERT_TRUE(commandes_depuis_appui(Appui::COURT, Etat::ATTENTE).depart);
    TEST_ASSERT_FALSE(commandes_depuis_appui(Appui::COURT, Etat::MONTEE).depart);
    TEST_ASSERT_TRUE(commandes_depuis_appui(Appui::COURT, Etat::DEFAUT).acquit);
    TEST_ASSERT_TRUE(commandes_depuis_appui(Appui::LONG, Etat::PALIER).arret);
    TEST_ASSERT_FALSE(commandes_depuis_appui(Appui::LONG, Etat::ATTENTE).arret);
}

void test_led_defaut_rouge() {
    const Couleur c = couleur_led(Etat::DEFAUT, 0, false, true);
    TEST_ASSERT_TRUE(c.r > 0 && c.g == 0 && c.b == 0);
}

void test_colonnes_plancher_chauffant() {
    char entete[640], ligne[640];
    const int ne = journal_entete_colonnes(entete, sizeof entete);
    TEST_ASSERT_TRUE(ne < static_cast<int>(sizeof entete) - 1);
    const char* fin = ";t_film;film;raisons_film";
    TEST_ASSERT_EQUAL_STRING(fin, entete + ne - static_cast<int>(std::strlen(fin)));
    Mesures m = mesures_film(41.2f, 43.1f, 47.25f);
    ContexteLigne c;
    c.film = true;
    c.raisons_film = 0;
    const int nl = journal_ligne_mesures(m, c, ligne, sizeof ligne);
    TEST_ASSERT_TRUE(nl < static_cast<int>(sizeof ligne) - 1);
    TEST_ASSERT_EQUAL_INT(compter(entete, ';'), compter(ligne, ';'));
    TEST_ASSERT_EQUAL_STRING(";47.25;1;0", ligne + nl - 10);
    // Variante désactivée, pas de sonde : champ vide, film 0, raison 1 (FILM_INACTIF).
    m = mesures(41.2f, 43.1f);
    c.film = false;
    c.raisons_film = FILM_INACTIF;
    const int n2 = journal_ligne_mesures(m, c, ligne, sizeof ligne);
    TEST_ASSERT_EQUAL_STRING(";;0;1", ligne + n2 - 5);
    // Métadonnées : paramètre et limites du film tracés en tête de fichier.
    char l[256];
    const Parametres p = parametres_defaut();
    journal_metadonnees(3, p, "x", l, sizeof l);
    TEST_ASSERT_NOT_NULL(std::strstr(l, "plancher_chauffant=0"));
    TEST_ASSERT_TRUE(journal_metadonnees(5, p, "x", l, sizeof l) > 0);
    TEST_ASSERT_NOT_NULL(std::strstr(l, "max_reg=50.0"));
    TEST_ASSERT_NOT_NULL(std::strstr(l, "defaut=55.0/10s"));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_entete_et_ligne_meme_nombre_de_colonnes);
    RUN_TEST(test_valeur_invalide_champ_vide);
    RUN_TEST(test_troncature_sans_debordement);
    RUN_TEST(test_evenement_neutralise_les_separateurs);
    RUN_TEST(test_detail_defauts);
    RUN_TEST(test_metadonnees);
    RUN_TEST(test_bouton_appui_court);
    RUN_TEST(test_bouton_rebond_ignore);
    RUN_TEST(test_bouton_appui_long_sans_court);
    RUN_TEST(test_commandes_selon_etat);
    RUN_TEST(test_led_defaut_rouge);
    RUN_TEST(test_colonnes_plancher_chauffant);
    return UNITY_END();
}
