# Protocole d'essai À VIDE — Phase 1 (prototype mono-ruche)

> **Aucune abeille en Phase 1.** Ruche Nicotplast Dadant 10 **vide de colonie**, garnie de cadres et d'une masse thermique d'eau. Règle d'or (`MARCHE_A_SUIVRE.md`) : pas d'essai avec abeilles avant validation à vide de la Phase 2.
> Références : `docs/architecture.md`, `hardware/cablage-phase1.md` (dont les **avertissements 230 V**), `hardware/peignes-sondes.md`, `hardware/module-toit-phase1.md`, `hardware/plancher-phase1.md`, `firmware/README.md`.
> Chaque essai a un identifiant **Ex**. Les résultats sont reportés dans `docs/journal-tests.md` (§ 15) et les fichiers CSV de la µSD sont copiés dans `tests/resultats/AAAA-MM-JJ_Ex/` (CSV volumineux dans `data/`, non versionné).
> Valeurs et critères marqués **[H]** : hypothèses à confirmer ; un critère [H] non atteint n'est pas un échec mais une **donnée** à analyser avant la Phase 2.

## 0. Organisation

| Essai | Objet | 230 V ? | Durée indicative |
|---|---|---|---|
| E0 | Relevé des cotes, inventaire | non | 2 h |
| E1 | Contrôles électriques **hors tension** | **non** | 1 h |
| E2 | Mise sous tension TBTS seule (USB / 12 V), firmware, entrées-sorties | non | 2 h |
| E3 | Étalonnage des sondes + réglage et test de la **chaîne C4** (bain, résistance 46 °C) | non | ½ journée |
| E4 | Première mise sous tension 230 V **avec lampe témoin** ; SSR, K1, enable dynamique, watchdog | oui (lampe) | 2 h |
| E5 | Élément seul, toit à l'air libre : profil de température, plafond de puissance | oui | 2 h |
| E6 | Réglage de C5 (bimétal) et vérification de la marge du TCO | oui | 2 h |
| E7 | Étanchéité de la boucle (fumée) et débits | non (12 V) | 1 h |
| E8 | **Cycle complet de référence** avec masse thermique | oui | 6–8 h |
| E9 | Cartographie des 5 sondes, T_air vs NTC C4, gradients | (données E8) | — |
| E10 | Défauts simulés en cycle (peigne débranché, ventilateur bloqué, arrêt opérateur) | oui | 2 h |
| E11 | Effet du **sens de la boucle** (toit retourné) | oui | 6–8 h |
| E12 | Effet des **proportions du cloisonnement** et des **soufflantes de plancher** (`pwm_plancher 0`) | oui | 3 × 6 h |
| E13 | Coupure secteur en palier | oui | 1 h |

Matériel : BOM §8 (bain, thermomètre de référence, lampe témoin, wattmètre, multimètre CAT III, contrôleur d'isolement 500 V, thermomètre IR, résistances étalons), ordinateur **sur batterie ou avec isolateur USB**, extincteur CO₂, support incombustible, arrêt d'urgence à portée de main.

Conditions ambiantes : local abrité, **12–32 °C** (architecture §2.4), sans soleil direct ni courant d'air ; noter T et HR ambiantes au début et à la fin de chaque essai (thermomètre indépendant posé à 1 m de la ruche, à l'ombre).

Console série : 115 200 bauds (`pio device monitor -e esp32`). Commandes utiles : `etat`, `sondes`, `param`, `depart`, `arret`, `acquit`, `test_trip`, `test_gel`, `rtc`, `etal`, `pos` (voir `firmware/README.md`).

---

## E0 — Cotes et inventaire
1. Relever toutes les cotes listées dans `module-toit-phase1.md` §8 (T1–T8, C1–C6) et `plancher-phase1.md` §6 (P1–P6), sur 2 exemplaires. Photos.
2. Mettre à jour les documents hardware si une cote invalide une hypothèse (en particulier : hauteur utile du toit T3, hauteur libre du plancher P2, largeur des ruelles C4).
3. Inventorier les composants reçus, noter les **références exactes** (soufflantes : débit et pression annoncés ; NTC C4 : table R/T et B ; bimétal : seuil ; TCO : Tf) dans le journal.

**Critère** : toutes les cotes renseignées ; aucune cote incompatible non traitée (ex. P2 < 12 mm → passer à l'alternative 30 × 30 × 7 du plancher).

## E1 — Contrôles électriques hors tension (prise **débranchée**)

| N° | Contrôle | Méthode | Critère |
|---|---|---|---|
| E1.1 | Continuité PE : broche terre de la fiche ↔ gaine de l'élément, gaine de chauffe alu, radiateur SSR, toute pièce métallique accessible | ohmmètre (4 fils si possible) | **< 0,1 Ω** (hors cordon) ; < 0,3 Ω câble inclus |
| E1.2 | Isolement L+N reliés ↔ PE, K1 forcé fermé à la main (**non** : K1 hors tension = ouvert → mesurer L+N ↔ PE en amont de K1, puis pontage provisoire de K1 et du SSR pour la partie aval) | contrôleur d'isolement **500 V DC**, PS1 **débranchée** (sinon fausse mesure) | **≥ 1 MΩ** (viser > 100 MΩ) |
| E1.3 | Isolement 230 V ↔ TBTS : L+N reliés ↔ 0 V TBTS | 500 V DC (PS1 et SSR **débranchés** côté TBTS si non conçus pour l'essai) ; à défaut 250 V DC | **≥ 2 MΩ** |
| E1.4 | Résistance de l'élément R1 entre ses bornes | ohmmètre, à froid | **≈ 212 Ω ± 10 %** (230² / 250) ; ∞ = coupé ; ≈ 0 = court-circuit |
| E1.5 | Continuité F2 (bimétal) et F3 (TCO) | ohmmètre | < 0,5 Ω chacun |
| E1.6 | Serrage des bornes 230 V, arrêts de traction, presse-étoupes serrés | visuel + traction 50 N | aucun fil ne sort, aucun brin libre |
| E1.7 | Aucun conducteur 230 V dans le boîtier TBTS ; séparation physique et câbles double isolation entre boîtiers | visuel | conforme `cablage-phase1.md` §1 |
| E1.8 | Rail 12 V ↔ 0 V, 3,3 V ↔ 0 V | ohmmètre | pas de court-circuit (> 100 Ω) |
| E1.9 | Pull-down 100 kΩ sur GPIO 12 et 17, **aucun** pull-up sur GPIO 12 | ohmmètre (carte ESP32 retirée) | conforme |
| E1.10 | Brochage J1–J4, couleurs des câbles M8/M12 réels | multimètre, tableau `cablage-phase1.md` §5 | conforme, noté au journal |
| E1.11 | Fil TACH de chaque ventilateur, alimenté **seul** en 12 V sur alimentation de labo, fil en l'air | voltmètre | **< 3,6 V** (sinon NE PAS raccorder à l'ESP32) |

**Critère d'E1** : 100 % conforme. Sinon : corriger, refaire E1 complet.

## E2 — TBTS seule (230 V **non branché**)

Alimenter en **12 V de laboratoire** (limitation 1,5 A) sur le rail 12 V, puis l'USB.

1. Courant à vide (ventilateurs arrêtés) : **< 300 mA** [H]. Rails : 12 V, 5,0 ± 0,25 V, 3,3 ± 0,1 V.
2. Flasher le firmware (`firmware/README.md`), régler l'horloge : `rtc AAAA-MM-JJTHH:MM:SS`. Redémarrer : la bannière s'affiche, **un fichier CSV** est créé sur la µSD (`/tv_….csv`), LED bleue.
3. `sondes` : 3 embases `presence=1`, nombre de sondes conforme, bus toit `trouvees=2`, SHT45 valide, NTC élément ≈ ambiante (± 3 °C).
4. **Détection de peigne absent** : débrancher P2 → `sondes` : `presence=0 conforme=0` ; `depart` → **DÉFAUT** `autotest`, `AUTOTEST_KO 0x0002` au journal ; rebrancher, `acquit` → ATTENTE. Répéter pour P1 (code 0x0001) et P3 (0x0004).
5. **Détrompage** : vérifier qu'une fiche 4 broches (P1) ne rentre pas dans J2/J3 et inversement.
6. Chaîne C4 : à la mise sous tension K1 est **retombé** (auto-maintien non armé) → `etat` : départ refusé `AT_C4_OUVERTE` (0x0040). Appuyer sur **S3** : K1 colle (clic), `C4 fermee=1`.
7. `test_trip` : K1 retombe et **reste** retombé après 2 s ; S3 le réarme.
8. Ventilateurs : `depart` (sondes non encore étalonnées → refus `AT_NON_ETALONNEE`, normal à ce stade). Vérifier les ventilateurs avec `param pwm_toit 100` puis `depart` **après** E3, ou en lisant les `rpm` de la ligne CSV en AUTOTEST. Relever **rpm à 30, 50, 80, 100 %** pour M1, M2, M3 (programmes de test : enchaîner `param pwm_toit X` / `depart` / `arret`) → reporter les vitesses nominales réelles par `param rpm_toit` et `param rpm_plancher` (le firmware suppose une vitesse proportionnelle au PWM [H] : noter l'écart).
9. **Enable dynamique, sans 230 V** : voltmètre sur l'entrée du SSR. ATTENTE : **< 1 V**. Forcer une demande de chauffe n'est possible qu'en cycle (après E3) : contrôle complet en E4.

## E3 — Étalonnage des sondes et réglage de la chaîne C4

### E3.a Étalonnage DS18B20 à 42 °C
Procédure complète : `hardware/peignes-sondes.md` §5 (bain 42,0 °C, 10 relevés, offsets par ROM, contrôles à 38 et 45 °C, constante de temps). Positions à déclarer : `P1_haut`, `P1_centre`, `P1_bas`, `P2`, `P3`, `air`, `retour`.
**Critères** : écart ≤ ±0,10 °C à 42 °C, ≤ ±0,15 °C à 38 et 45 °C ; dispersion ≤ 0,15 °C.

### E3.b NTC C4 au bain — point de basculement réel
1. Placer RT1 (dans son porte-sonde, sac étanche fin) contre la sonde de référence dans le bain ; nœud alimenté en 12 V **sans 230 V** ; K1 armé (S3).
2. Bain à **44,0 °C**, stabiliser 20 min : K1 doit rester collé.
3. Monter le bain par paliers de **0,2 °C** (10 min de stabilisation chacun) jusqu'à la retombée de K1. Noter **T_bascule** (référence).
4. **Critère : T_bascule = 45,0 ± 0,2 °C.** Sinon ajuster R_seuil (≈ 166 Ω/K) : **R_seuil plus grande → basculement plus froid** (ex. + 33 Ω en série ≈ −0,2 K) ; R_seuil plus petite → basculement plus chaud. Recommencer.
5. Redescendre à 44,5 °C : K1 reste **retombé** (verrouillage). S3 : K1 se réarme seulement sous 45,0 °C.
6. Répéter le point de basculement **3 fois** : répétabilité ≤ 0,1 °C.

### E3.c Test de la chaîne C4 avec résistance simulant 46 °C (test de pose)
1. RT1 à l'ambiante, K1 armé, `C4 fermee=1`.
2. Appuyer sur **S2 (TEST SÉCURITÉ)** : R_test (4,12 kΩ ≈ 46,2 °C) remplace la NTC → **K1 retombe** (clic), GPIO 16 HAUT, la console affiche `C4 fermee=0`.
3. Relâcher S2 : **K1 reste retombé**. Appuyer sur S3 : K1 se réarme.
4. Variante sans S2 : débrancher RT1 et brancher à sa place la résistance étalon **4,12 kΩ** (46,2 °C) puis **4,32 kΩ** (45,0 °C, cas limite : retombée attendue ou non, noter) puis **4,53 kΩ** (43,8 °C : K1 doit rester collé).
5. **NTC coupée** (RT1 débranchée, borne en l'air) → K1 retombe ; **NTC court-circuitée** → K1 retombe.
6. **Perte du 12 V** (débrancher 2 s puis rebrancher) → K1 reste retombé jusqu'à S3.
7. **S3 maintenu enfoncé pendant que S2 est pressé** : K1 doit retomber quand même (les comparateurs priment). Noter.
8. Mesurer le **temps de réaction** (S2 → ouverture du contact K1, à l'oscilloscope ou au chronomètre) : **< 100 ms** [H].

**Critère d'E3.c : 100 % des cas provoquent l'ouverture verrouillée de K1.** Tout écart = arrêt de la phase.

## E4 — Première mise sous tension 230 V avec **lampe témoin** (élément débranché)

> Avant : E1, E2, E3 conformes. DDR testé (bouton TEST). Arrêt d'urgence testé. Isolateur USB ou ordinateur sur batterie. Couvercle du boîtier 230 V **fermé** avant de brancher la prise.

1. Débrancher l'élément R1 (prise débranchée !) et raccorder à sa place la **lampe halogène 40–60 W** (en série F2/F3 conservés).
2. Brancher la prise. Mesurer la tension aux bornes de la lampe : **0 V** (K1 ouvert, SSR ouvert). S3 : K1 armé, lampe toujours **éteinte**.
3. Paramétrer le cycle pour la lampe : `param timeout_montee_min 60`. Sondes à l'ambiante (≈ 20 °C) < 39 °C : départ possible. `depart` → AUTOTEST (8 s, ventilateurs) → MONTÉE : **la lampe s'allume** au rythme du plafond de puissance (`puissance_max` = 60 % : 6 s allumée / 4 s éteinte par fenêtre de 10 s).
4. `param` ne doit pas être modifiable en cycle (message REFUS).
5. **Enable dynamique** : `test_gel` (la boucle principale se fige volontairement) → la lampe s'éteint en **≤ 3,5 s** (jeton périmé), puis le **watchdog** redémarre l'ESP32 en ≈ 5 s → ATTENTE, lampe éteinte, aucun redémarrage de cycle. Noter les deux temps.
6. Débrancher l'USB et le 12 V de l'ESP32 seul (si accessible) en MONTÉE : lampe éteinte immédiatement.
7. **Ouverture par le MCU** : en MONTÉE, `test_trip` → lampe éteinte, K1 verrouillé ouvert, et le firmware passe en DÉFAUT `C4_ouverte` au pas suivant. `acquit` refusé tant que K1 n'est pas réarmé ; S3 puis `acquit` → ATTENTE.
8. **C4 pendant la chauffe** : en MONTÉE, S2 → lampe éteinte en < 100 ms, DÉFAUT `C4_ouverte`.
9. **Arrêt d'urgence** et **DDR** (bouton TEST) en MONTÉE : lampe éteinte, nœud éteint ; au retour, ATTENTE + K1 ouvert.
10. Fuite du SSR : en ATTENTE, tension aux bornes de la lampe avec K1 armé : **< 10 V AC** [H] (courant de fuite du SSR dans une charge de 1 kΩ à froid).

**Critère d'E4** : tous les points conformes. La lampe n'a jamais été allumée hors MONTÉE/PALIER.

## E5 — Élément seul, toit hors ruche : profils et plafond de puissance

> Toit retourné sur un **support incombustible**, cloisonnement en place, aucune ruche dessous. Élément rebranché (prise débranchée pendant l'opération). Wattmètre en amont. Surveillance permanente.

1. Sondes : peignes posés à plat sur le support (température ambiante) → le TOR demandera la chauffe en continu ; c'est la **limite air (44,0 °C)** et le plafond de puissance qui la coupent.
2. `param puissance_max 60` (défaut), `param pwm_toit 80`. `depart`.
3. Relever toutes les 10 s (CSV) : T_air, T_retour, T_elem, puissance (wattmètre), rpm.
4. Observer : temps pour que T_air atteigne 44,0 °C ; **dépassement maximal de T_air après coupure** (inertie de l'élément) ; T_elem maximale ; aucun défaut `air_surtemp` (44,5 °C pendant 10 s) ni ouverture de C4 attendue.
5. Répéter avec `puissance_max` 100 puis 40 et `pwm_toit` 50 puis 100.
6. Arrêt : `arret` → REFROIDISSEMENT, brassage 15 min.

**Critères [H]** : dépassement de T_air après coupure **≤ 0,5 °C** (soit ≤ 44,5 °C) avec le couple (`puissance_max`, `pwm_toit`) retenu ; T_elem max en régime ≤ 60 °C ; puissance mesurée 240–260 W quand le SSR conduit. Le couple retenu devient la référence des essais suivants (journal). Si aucun réglage n'évite un dépassement > 0,5 °C : **arrêt**, revoir la gaine (débit, inertie de l'élément) avant E8.

## E6 — Réglage de C5 (bimétal) et marge du TCO

1. Relever **T_elem max en régime normal** (E5 et E8). Le seuil du bimétal F2 = **T_elem max + 10 °C**, arrondi au seuil disponible supérieur (70 / 80 / 90 °C).
2. Essai de déclenchement **sous surveillance**, toit à l'air libre sur support incombustible, **soufflante du toit débranchée** (air immobile : cas « soufflante bloquée + SSR collé »). Le firmware couperait en 10 s (DÉFAUT `ventilo_toit`) : l'essai se fait donc **sans l'ESP32** — carte retirée, K1 armé (S3), entrée du SSR alimentée directement par une **alimentation de labo 12 V** que l'opérateur coupe à la main. Élément alimenté en continu ; arrêt immédiat si une fumée ou une odeur apparaît.
3. Noter le temps et T_elem à l'ouverture de F2. **Critère : F2 s'ouvre avant que T_elem n'atteigne Tf(TCO) − 15 °C** (≈ 87 °C pour un TCO 102 °C) et la température de la grille de soufflage reste < 60 °C.
4. Réarmer F2 (bouton) après refroidissement. Le TCO ne doit **pas** avoir fondu (continuité).
5. Le TCO n'est pas testé destructivement sur le prototype (essai d'un TCO de rechange dans un four de cuisine à 95 puis 105 °C, optionnel).

## E7 — Étanchéité de la boucle et débits (12 V seulement, 230 V débranché)

1. Ruche complète (plancher fermé, corps garni de 10 cadres, porte Nicot à aérations, peignes, toit). Ventilateurs à 80 % (en AUTOTEST/MONTÉE avec l'élément débranché, ou alimentation de labo).
2. **Fumée** (bâton d'encens) autour de la jonction toit/corps, plancher/corps, presse-étoupes, encoches des câbles : noter toutes les fuites, les reprendre (joint, mastic).
3. **Court-circuit au toit** : toit posé sur un corps **sans cadres**, papier de soie sur la plaque centrale : il ne doit pas se soulever.
4. **Débit** : anémomètre à fil chaud sur la grille de soufflage (9 points en grille), débit = vitesse moyenne × surface libre ; idem aspiration. Comparer à la courbe de la soufflante. Répéter pour `pwm_toit` 50 / 80 / 100 et `pwm_plancher` 0 / 80. À défaut d'anémomètre : le débit sera estimé en E8 par bilan thermique `Q ≈ P / (1,1 × (T_air − T_retour))`.

## E8 — Cycle complet de référence (masse thermique)

**Montage** : ruche complète (plancher fermé + 2 soufflantes, corps, 4 cadres bâtis vides au centre, 8–10 bouteilles d'eau en rive, porte Nicot à aérations, toit) ; peignes : P1 dans la ruelle centrale (entre les cadres 5 et 6), P2 entre les cadres 2 et 3, P3 entre les cadres 8 et 9 [H]. Masse d'eau pesée (noter). Ruche **à température ambiante depuis ≥ 12 h**. Paramètres : défauts du firmware + couple retenu en E5 (`puissance_max`, `pwm_toit`), `pwm_plancher 80`. **Masse thermique : bouteilles d'eau réparties ; noter le nombre et la masse.**

**Déroulement** :
1. Copier l'en-tête de configuration (`param`) dans le journal. Wattmètre à zéro.
2. `depart`. Surveiller la première demi-heure en continu, puis toutes les 30 min (ne jamais laisser le banc sans surveillance en Phase 1).
3. Laisser le cycle aller jusqu'à **FIN** ou **DÉFAUT** (timeout montée **150 min** par défaut ; la masse froide peut l'imposer : relever avec `param timeout_montee_min 240` si nécessaire et le noter).
4. À la FIN : relever l'énergie consommée (kWh), copier le CSV, noter la ligne `RESUME`.

**Grandeurs à extraire** (tableau du journal) : durée de montée (départ → PALIER), temps de palier cumulé, temps hors plage, écart max (Tmax − Tmin) en palier, T max par sonde, T_air max, nombre de coupures par la limite air (`raisons_coupure` bit 2) et par la limite couvain (bit 1), taux de conduction du SSR en palier (= puissance moyenne / 250 W), énergie par cycle, T_elem max, HR max sous le toit (condensation ?), T ambiante.

**Critères [H]** :
| Critère | Seuil |
|---|---|
| Cycle sans DÉFAUT jusqu'à FIN | oui |
| Durée de montée | ≤ 150 min (sinon noter la valeur : base du dimensionnement) |
| Palier : 120 min cumulées, hors plage | ≤ 10 min cumulées |
| Écart entre les 5 sondes en palier (Tmax − Tmin) | **≤ 1,0 °C** (cible) ; ≤ 1,5 °C acceptable ; > 1,5 °C : circulation à revoir (E11–E12) |
| T_air max | ≤ 44,5 °C, **jamais** de déclenchement C4 |
| Puissance moyenne en palier | noter (estimation 95–150 W avec colonie, nettement moins à vide) |
| Énergie par cycle | noter (estimation 0,45–0,65 kWh) |

## E9 — Cartographie des 5 sondes et gradients (analyse des données E8, E11, E12)

1. Tracer les 5 courbes couvain + T_air + T_retour sur tout le cycle.
2. Tableau en palier (moyennes sur les 120 min) : T moyenne et écart-type par sonde ; classement de la plus froide à la plus chaude ; **gradient vertical** P1_haut − P1_bas ; **gradient latéral** P2 − P3 et (P2+P3)/2 − P1_centre.
3. Quelle sonde est la **plus froide** le plus souvent (c'est elle qui pilote) ? Reste-t-elle la même tout le palier ?
4. **T_air vs NTC C4** : en fin de E8, pendant le refroidissement, poser une sonde de référence contre le porte-sonde et comparer à T_air ; vérifier que la NTC C4 voit au moins la même température que T_air (même jet). Écart toléré ≤ 0,3 °C [H].
5. Identifier les 3 points les plus représentatifs pour la série (architecture §4.2).

## E10 — Défauts simulés en cycle (vérification physique de la sécurité logicielle)

Lancer un cycle (E8 raccourci : `param palier_min 90`) et provoquer, l'un après l'autre (acquitter et relancer entre chaque) :

| N° | Action | Attendu | Délai attendu |
|---|---|---|---|
| E10.1 | Débrancher P2 en MONTÉE | DÉFAUT `peigne_P2`, chauffe coupée, ventilateurs arrêtés | ≤ 2 pas (4 s) |
| E10.2 | Débrancher P1 en PALIER | DÉFAUT `peigne_P1` | ≤ 4 s |
| E10.3 | Bloquer la soufflante du toit (débrancher son connecteur) | DÉFAUT `ventilo_toit` | ≤ 12 s |
| E10.4 | Débrancher J4 (plancher) | DÉFAUT `ventilo_plancher_A` et `_B` | ≤ 12 s |
| E10.5 | Appui long (3 s) sur S1 en PALIER | REFROIDISSEMENT, chauffe coupée, brassage 15 min, puis FIN | immédiat |
| E10.6 | S2 (test C4) en MONTÉE | K1 ouvert, DÉFAUT `C4_ouverte` | < 100 ms (K1) / 2 s (firmware) |
| E10.7 | Débrancher la sonde du bus toit (T_air) si accessible | DÉFAUT `sonde_air` | ≤ 6 s |
| E10.8 | `acquit` alors que la cause est présente | reste en DÉFAUT | — |
| E10.9 | Après acquittement | retour en ATTENTE, **jamais** de chauffe sans nouveau `depart` | — |

Les défauts de surtempérature (44,0 / 44,5 °C, SSR collé) sont validés par les **tests unitaires** (`pio test -e native`) et seront simulés physiquement en Phase 2 (protocole `protocole-phase2-defauts.md`).

## E11 — Effet du sens de la boucle

1. Refaire E8 à l'identique (mêmes paramètres, même masse, ambiance à ± 2 °C près) avec le **sens inversé** : soufflage à l'AVANT, aspiration à l'ARRIÈRE (toit retourné de 180°, soufflantes de plancher retournées).
2. Comparer au cycle de référence : durée de montée, écart Tmax − Tmin en palier, sonde la plus froide, puissance moyenne, énergie, T à la porte (thermomètre devant les aérations de la porte Nicot : l'air chaud sort-il ?).
3. **Décision** : garder le sens qui minimise l'écart en palier ; à écart égal (± 0,2 °C), celui qui consomme le moins.

## E12 — Proportions du cloisonnement et soufflantes de plancher

Trois cycles (E8 à l'identique, sens retenu en E11) :

| Variante | Cloisonnement | `pwm_plancher` | Objet |
|---|---|---|---|
| E12.a | **1/4 – 1/2 – 1/4** (référence) | 80 | = E8/E11 |
| E12.b | **1/4 – 1/2 – 1/4** | **0** (soufflantes de plancher désactivées : `param pwm_plancher 0`) | apport réel des soufflantes de plancher |
| E12.c | **1/5 – 3/5 – 1/5** (réglettes) | 80 | bandes plus étroites : vitesse plus forte, air forcé plus profond ? |
| E12.d (option) | **1/3 – 1/3 – 1/3** | 80 | si E12.c dégrade |

Notes :
- Avec `pwm_plancher 0`, l'auto-test ne teste pas M2/M3 et la sécurité n'en surveille pas les tachymètres (le firmware le permet uniquement pour le plancher ; la soufflante du toit reste ≥ 30 %). **Remettre `pwm_plancher 80` après l'essai** et le vérifier par `param`.
- Comparer : écart Tmax − Tmin en palier, gradient vertical P1_haut − P1_bas (les soufflantes de plancher doivent surtout réchauffer le **bas**), durée de montée, puissance moyenne.
- **Décision** : proportions retenues ; soufflantes de plancher **conservées** si elles réduisent l'écart d'au moins 0,3 °C ou la montée d'au moins 10 % [H], sinon à supprimer en série (simplification).

## E13 — Coupure secteur en palier

1. En PALIER, actionner l'arrêt d'urgence 10 s puis réarmer.
2. Attendu (Phase 1, sans FRAM) : nœud éteint puis redémarré en **ATTENTE**, K1 **ouvert** (S3 nécessaire), **aucun redémarrage de la chauffe**, nouveau fichier CSV, événement `DEMARRAGE`. Le cycle interrompu est incomplet (le résumé n'est pas écrit : la reprise après coupure est une fonction de la Phase 2).

---

## 14. Critères de réussite de la Phase 1 (synthèse)

| Domaine | Critère | Seuil |
|---|---|---|
| Électrique | E1 conforme à 100 % ; DDR et arrêt d'urgence fonctionnels | obligatoire |
| Sécurité matérielle | C4 : basculement à **45,0 ± 0,2 °C**, verrouillé, réarmement manuel ; test 46 °C (S2) 100 % ; NTC coupée/court-circuitée détectée ; C5 seuil fixé et vérifié | obligatoire |
| Sécurité C3 | enable dynamique : SSR ouvert ≤ 3,5 s après gel de la boucle ; watchdog ≤ 6 s ; jamais de chauffe hors MONTÉE/PALIER | obligatoire |
| Sondes | étalonnage ±0,10 °C à 42 °C ; peigne absent → départ refusé / DÉFAUT en cycle | obligatoire |
| Firmware | `pio test -e native` vert ; journaux CSV complets et horodatés sur µSD et série | obligatoire |
| Thermique | cycle complet à vide sans DÉFAUT ; T_air ≤ 44,5 °C ; jamais de déclenchement C4 en fonctionnement normal | obligatoire |
| Homogénéité | Tmax − Tmin en palier ≤ 1,0 °C (cible), ≤ 1,5 °C (acceptable) | [H] |
| Énergie | puissance de palier et énergie par cycle mesurées et reportées | mesure |
| Configuration | sens de boucle, proportions, plancher ventilé ou non, `puissance_max`, `pwm_toit` : décidés et notés | décision |

## 15. Tableau à reporter dans `docs/journal-tests.md`

Ajouter une ligne par essai au tableau existant (`| Date | Phase | Test | Conditions | Résultat | Validé |`), en suivant ce format :

| Date | Phase | Test | Conditions | Résultat | Validé |
|---|---|---|---|---|---|
| AAAA-MM-JJ | 1 | E0 cotes | 2 toits, 2 corps, 1 plancher | T3 = … mm ; P2 = … mm ; C4 = … mm (détail en annexe) | oui/non |
| | 1 | E1 hors tension | prise débranchée | PE … Ω ; isolement L+N/PE … MΩ ; 230 V/TBTS … MΩ ; R1 … Ω ; TACH max … V | |
| | 1 | E2 TBTS | 12 V labo | I repos … mA ; rpm M1/M2/M3 à 100 % : … / … / … ; peigne absent détecté P1/P2/P3 : o/o/o | |
| | 1 | E3.a étalonnage | bain 42,0 °C, réf. n° … | offsets (ROM : offset) … ; écart max 42/38/45 °C : … / … / … ; τ = … s | |
| | 1 | E3.b C4 bain | bain 44 → 45,4 °C | T_bascule = … / … / … °C ; R_seuil = … Ω | |
| | 1 | E3.c C4 46 °C | S2, 4,12 / 4,32 / 4,53 kΩ, NTC ouverte/CC, perte 12 V | ouverture verrouillée : o/…/n/o/o/o ; t réaction … ms | |
| | 1 | E4 lampe témoin | lampe 60 W, DDR testé | gel → SSR ouvert en … s ; watchdog … s ; test_trip o ; S2 o ; fuite SSR … V | |
| | 1 | E5 élément seul | toit à l'air, T amb … °C, puissance_max … %, pwm_toit … % | T_air dépassement max … °C ; T_elem max … °C ; P … W | |
| | 1 | E6 C5 | air immobile | F2 (… °C) ouvert à T_elem … °C après … min ; grille max … °C | |
| | 1 | E7 étanchéité/débit | pwm_toit … %, pwm_plancher … % | fuites : … ; débit soufflage … m³/h | |
| | 1 | E8 cycle référence | masse … kg, T amb … °C / HR … %, sens … , 1/4-1/2-1/4, plancher 80 % | montée … min ; palier … min (hors plage … min) ; ΔT max palier … °C ; T_air max … °C ; P palier … W ; E … kWh ; défauts … | |
| | 1 | E9 cartographie | données E8 | plus froide : … ; gradient vertical … °C ; latéral … °C ; T_air − NTC C4 … °C | |
| | 1 | E10 défauts | cycle raccourci | E10.1…E10.9 : o/o/… | |
| | 1 | E11 sens inversé | idem E8, sens … | montée … ; ΔT max … ; E … kWh → sens retenu : … | |
| | 1 | E12.b plancher off | `pwm_plancher 0` | montée … ; ΔT max … ; P1_haut − P1_bas … °C | |
| | 1 | E12.c 1/5-3/5-1/5 | réglettes | montée … ; ΔT max … | |
| | 1 | E13 coupure secteur | AU 10 s en palier | ATTENTE, K1 ouvert, pas de chauffe : o/n | |

Conserver, pour chaque essai : le fichier CSV, la sortie de `param` et de `sondes`, des photos du montage, et la version du firmware (`# firmware=… version=…` en tête du CSV).
