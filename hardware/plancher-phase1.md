# Plancher fermé — Phase 1 (plancher Nicot aéré existant)

> Décisions : D2 (plancher fermé = plénum de retour), D9 (boucle avant ↔ arrière), **D21 (variante plancher chauffant d'appoint, §7)**, architecture §1.2 « Plancher : fermer un plancher Nicot existant », « Variante : plancher chauffant d'appoint » et « Ventilateurs de plancher : recommandés ».
> Hauteur libre mesurée par l'apiculteur : **16 mm** entre la grille et le dessous du plancher. Les autres cotes sont **[H]** (§6).

## 1. Principe

```
   Coupe AVANT ↔ ARRIÈRE (dans l'axe d'une ruelle) — plancher seul

        AVANT (porte Nicot)                                           ARRIÈRE
   ┌──────────────────────────────── cadres ───────────────────────────────────┐
   │   ▲ remontée vers l'aspiration du toit          descente depuis le toit ▼   │
   ├───▲───────────────────────────────────────────────────────────────▼──────┤
   │ ##### grille Nicot (inchangée, les abeilles restent au-dessus) ########### │ ← 0
   │   ◄── M2 ◄──┐                                          ┌──── M3 ◄──       │
   │  sortie     │ ◄──────────── plénum de retour (16 mm) ◄──┘    entrée       │
   │  vers l'avant  cloison basse optionnelle (6 mm)                           │
   │ ═══════════ plaque d'obturation PVC 3 mm + joint mousse ════════════════ │ ← 16 mm
   └────────────────────────────────────────────────────────────────────────────┘
       Les deux soufflantes poussent l'air de l'ARRIÈRE vers l'AVANT (même sens que la boucle).
```

L'air descendu par les ruelles arrière traverse la grille, est aspiré par la soufflante arrière M3, parcourt le plénum et est poussé par la soufflante avant M2 vers les ruelles avant, où il remonte vers la bande d'aspiration du toit.

## 2. Fermeture par le dessous

Ordre de préférence :
1. **Glissière du lange de comptage** (si le plancher en a une) : une plaque **PVC expansé 3 mm** (ou polycarbonate 2 mm) découpée à la cote de la glissière, avec un **joint mousse EPDM 6 × 3** sur la périphérie supérieure ; obturation de la fente d'entrée de la glissière par une réglette + ruban alu. Démontable sans outil, aucune modification du plancher.
2. **Plaque vissée par le dessous** : même plaque, joint périphérique, vis inox auto-taraudeuses tous les 80 mm dans le cadre du plancher, en dehors de la grille. Réversible (trous rebouchables).

Dans les deux cas :
- la **hauteur utile** de plénum (grille → plaque) doit rester ≥ 12 mm partout (soufflantes 10 mm + 2 mm de jeu) ;
- toutes les autres ouvertures du plancher (fente d'entrée du lange, aérations latérales éventuelles) sont fermées : ruban aluminium adhésif + mastic silicone neutre ;
- l'étanchéité est contrôlée au protocole E10 (fumée, papier).

Une plaque en 2–4 segments imprimés 3D (architecture §1.3) n'est **pas** nécessaire en Phase 1.

## 3. Implantation des soufflantes dans 16 mm

```
   Vue de dessus du plancher (plaque d'obturation, côté plénum)     [H]

                              ARRIÈRE
   ┌──────────────────────────────────────────────────────────────────┐
   │                ┌──────┐                                           │
   │                │  M3  │ 40×40×10, sortie vers l'AVANT             │
   │                │  ↓   │ aspiration par le dessus (sous la grille) │
   │                └──────┘                                           │
   │                    ↓       ← cloison basse optionnelle            │
   │                    ↓         (6 mm, au milieu, n'empêche pas      │
   │  ══════════════════↓═══      le passage : oriente le flux)        │
   │                    ↓                                              │
   │                ┌──────┐                                           │
   │                │  M2  │ 40×40×10, sortie vers l'AVANT             │
   │                │  ↓   │                                           │
   │                └──────┘                                           │
   │          ○ passe-câbles (presse-étoupe M12 ×2 : J4 signaux, J5 film) │
   └──────────────────────────────────────────────────────────────────┘
                               AVANT
   Les soufflantes sont centrées en largeur (sous le nid à couvain), à ≈ 60 mm
   des bords avant et arrière de la grille.
```

Détails :
- **Hauteur** : soufflante 10 mm + support 1,5 mm = 11,5 mm ; il reste ≈ 4,5 mm d'air au-dessus de l'ouïe d'aspiration. Les soufflantes radiales 40 × 40 × 10 **aspirent par le dessus** (sous la grille) et **soufflent latéralement** : exactement l'usage voulu dans un plénum plat **[H]** (vérifier la courbe débit/pression : 4,5 mm d'aspiration étranglent le débit).
- **Fixation** : sur la plaque d'obturation (et non sous la grille), par un **support ASA ou PETG** imprimé ou un petit carré de PVC 1,5 mm, vis M2,5 + entretoises silentblocs ; démontage avec la plaque.
- **Protection** : la grille Nicot interdit déjà l'accès aux abeilles ; on ajoute un **grillage inox Ø 2,5 mm** sur l'ouïe d'aspiration de chaque soufflante contre la cire, la propolis et les débris (en Phase 1 à vide : pour valider l'encombrement).
- **Température** : l'air de retour est à ≈ 42 °C ; soufflantes tenue ≥ 70 °C.
- **Nettoyage** : accès par dépose de la plaque ; aucune pièce collée.

### Alternative si 16 mm ne suffisent pas
Si les cotes réelles (P2 ci-dessous) laissent < 12 mm utiles : soufflantes **axiales 30 × 30 × 7 mm** (architecture §1.2) posées à plat avec un **déflecteur à 90°** imprimé, ou une seule soufflante radiale 40 mm à l'arrière.

## 4. Câblage vers le toit

| Élément | Choix |
|---|---|
| Câble | **M12 8 broches** surmoulé PUR 8 × 0,25 mm², 1,0 m, côté plancher ; il sort du plancher par un **presse-étoupe M12** percé dans la plaque d'obturation (ou une encoche latérale du cadre du plancher garnie de silicone), remonte le long du corps **à l'extérieur**, et se branche sur l'embase **J4** en façade du toit. Variante D21 : second câble **M12 code T** vers **J5** (§7.4). |
| Dans le plénum | les 2 × 4 fils des soufflantes (et, variante D21, la sonde du film) arrivent sur une petite **plaquette de jonction** vernie (soudure + gaine thermo) fixée sur la plaque ; **les fils PWM de M2 et M3 y sont reliés** (PWM commun) ; aucun fil sous tension libre dans le plénum. |
| Tension | **12 V et 3,3 V** (TBTS) sur J4, 12 V commuté par Q4 (`ALIM_VENTILOS`, GPIO 12). Le 24 V du film passe **uniquement** par J5. |

Brochage de J4 (M12 code A, 8 broches) — **identique** au tableau de `cablage-phase1.md` §5 :

| Broche M12 | Couleur usuelle (câble surmoulé M12 8 br.) [H] | Signal | GPIO ESP32 |
|---|---|---|---|
| 1 | blanc | +12 V soufflantes (commuté) | via Q4 (GPIO 12) |
| 2 | marron | 0 V | — |
| 3 | vert | PWM **commun** M2 (avant) + M3 (arrière) | GPIO 27 (via Q7, collecteur ouvert) |
| 4 | jaune | TACH M2 | GPIO 35 |
| 5 | gris | +3,3 V sonde du film (D21) — *ex-PWM M3* | rail 3,3 V via 10 Ω |
| 6 | rose | TACH M3 | GPIO 15 |
| 7 | bleu | 0 V (retour, doublé) | — |
| 8 | rouge | DQ sonde du film (D21) — *ex-réserve* | GPIO 4 (bus `OW_TOIT`, via 100 Ω) |

Les soufflantes reçoivent toujours le même PWM : les relier sur une seule ligne libère GPIO 14 (commande du film) et une broche de J4. Les tachymètres restent séparés (un défaut par soufflante). Sans la variante, les broches 5 et 8 restent non câblées côté plancher.

**Vérifier les couleurs au multimètre** sur le câble acheté : elles ne sont pas normalisées entre fabricants.

Comportement firmware attendu (déjà implémenté) : les deux soufflantes du plancher reçoivent le même PWM (`pwm_plancher`, défaut 80 % [H]) ; un tachymètre < 50 % de la vitesse attendue pendant 10 s en cycle → **DÉFAUT** `ventilo_plancher_A` / `ventilo_plancher_B`. Pour l'essai comparatif sans soufflantes de plancher (protocole E12), `param pwm_plancher 0` les désactive (le toit, lui, ne peut pas descendre sous 30 %).

## 5. Points de vigilance

- **Retour d'air par la porte Nicot** : la porte à petites aérations (D5) est sur l'avant, là où le plénum pousse l'air vers le haut ; une légère surpression à l'avant fera sortir un peu d'air chaud par les aérations — à quantifier (bilan énergétique, E8).
- **Condensation** dans le plénum : le plancher est la partie la plus froide de la boucle. Au démontage après chaque essai, noter la présence d'eau sur la plaque (journal).
- **Varroas tombés** : le lange de comptage est condamné en Phase 1. En Phase 7, prévoir une plaque d'obturation graissée faisant office de lange (à étudier).

## 6. Cotes à relever sur le plancher réel

| N° | Cote | Pour quoi faire |
|---|---|---|
| P1 | Dimensions extérieures du plancher ; dimensions de la zone grillagée | plaque d'obturation, position des soufflantes |
| P2 | **Hauteur libre grille → dessous**, en au moins 5 points (coins + centre) : contrôle des 16 mm mesurés, recherche de nervures | implantation M2/M3, hauteur utile ≥ 12 mm |
| P3 | Présence, largeur, hauteur et profondeur de la **glissière du lange** ; position de son ouverture | choix de la méthode de fermeture (§2) |
| P4 | Nervures et renforts sous la grille : position, hauteur | obstacles dans le plénum |
| P5 | Maille et épaisseur de la grille | perte de charge, garde au-dessus des soufflantes |
| P6 | Autres ouvertures (aérations latérales, trou de vol du plancher, fentes de la porte) | obturation |
| P7 | **Surface utile de la plaque** (hors glissière, hors nervures P4) et distance entre les soufflantes M2/M3 et les bords | dimensions des films (§7.1), position des bimétaux et de la sonde |

## 7. Variante : plancher chauffant d'appoint (D21)

> Le chauffage **principal reste au toit** (inchangé). Le film est un **appoint** placé au bas de la boucle d'air, activé par `param plancher_chauffant 1` pour l'essai comparatif E14. **TBTS 24 V uniquement : jamais de 230 V dans le plancher.**

### 7.1 Implantation

```
   Vue de dessus de la plaque d'obturation (côté plénum)                 [H]

                                ARRIÈRE (entrée de l'air de retour)
   ┌────────────────────────────────────────────────────────────────────────┐
   │ ┌───────────────────────────┐  ┌──────┐  ┌───────────────────────────┐ │
   │ │ FILM A  24 V 35 W         │  │  M3  │  │ FILM B  24 V 35 W         │ │
   │ │ ≈ 400 × 145 mm [H]        │  └──────┘  │ ≈ 400 × 145 mm [H]        │ │
   │ │                           │   couloir  │                           │ │
   │ │   [BM_A]  [TCO_A]         │   central  │   [BM_B]  [TCO_B]         │ │
   │ │                           │   ≈ 50 mm  │                           │ │
   │ │                           │  (câbles)  │                           │ │
   │ │        [T_film] (aval)    │  ┌──────┐  │                           │ │
   │ └───────────────────────────┘  │  M2  │  └───────────────────────────┘ │
   │                                └──────┘   ○ J4   ○ J5 (presse-étoupes) │
   └────────────────────────────────────────────────────────────────────────┘
                                 AVANT (sortie vers les ruelles avant)
   Surface chauffée totale ≈ 2 × 400 × 145 ≈ 1 160 cm² ; 70 W → ≈ 0,06 W/cm².
```

- **Deux films côte à côte** (référence de base) de part et d'autre du couloir central où sont posées M2 et M3 : formats rectangulaires standard, aucune découpe. **Option** : un film unique ≈ 400 × 300 mm **sur mesure** avec deux réserves 50 × 50 mm pour les supports des soufflantes **[H]** (délai et prix à demander au fabricant).
- **Type** : film **polyimide** (épaisseur ≈ 0,2–0,3 mm, adhésif 3M 467 ou équivalent, tenue ≥ 150 °C) — à défaut tapis **silicone** 1,5 mm (plus épais, plus robuste). Tension **24 V DC**, 30–40 W par film, résistance ≈ 14–19 Ω **[H]** ; câbles silicone 0,5 mm² minimum.
- **Pose** : collé sur la **face supérieure** de la plaque d'obturation (PVC expansé 3 mm), donc **sous la grille**, dans le plénum de retour : inaccessible aux abeilles. Les soufflantes restent fixées sur la plaque (pas sur le film). Bords du film à ≥ 10 mm du joint périphérique.
- **Plaque** : le PVC expansé supporte en continu ≈ 60 °C **[H]** ; la surface du film est régulée à 50 °C et coupée à 55 °C : marge faible mais suffisante. **Préférer le polycarbonate 2 mm** (tenue ≈ 115 °C) si la variante est retenue, et vérifier en E14 l'absence de déformation de la plaque.
- **Flux** : l'air de retour (≈ 42 °C) lèche le film d'arrière en avant, aspiré par M3 et poussé par M2 : le point le plus chaud du film est **en aval (côté avant)** **[H]** — c'est là que vont la sonde et les bimétaux, à confirmer au thermomètre IR en E14.0.

### 7.2 Cotes dans les 16 mm

| Élément | Épaisseur au-dessus de la plaque | Commentaire |
|---|---|---|
| Film polyimide + adhésif | ≈ 0,3–0,5 mm (silicone : 1,5–2 mm) | sur toute la surface chauffée |
| Bimétal plat **KSD9700** (ou équivalent mini) | ≈ 5 mm | face sensible **contre** le film, pâte thermique, ruban aluminium par-dessus |
| TCO axial | Ø ≈ 4 mm | couché, collé contre le film (ruban alu + Kapton) |
| DS18B20 TO-92 couché | ≈ 4 mm | face plate contre le film, pâte thermique, ruban alu |
| Soufflantes M2/M3 + support | 11,5 mm | **hors film** (couloir central) |
| **Air libre restant** | **≥ 10 mm** au-dessus des capteurs ; **≥ 15 mm** au-dessus du film nu | hauteur utile ≥ 12 mm (§2) respectée sur toute la surface, sauf au droit des petits capteurs |

Contrôle en E0 (P2, P7) : si la hauteur réelle est < 14 mm, prendre le **polyimide** (pas de silicone) et loger les bimétaux dans des **lamages** de la plaque.

### 7.3 Sonde, bimétaux, fusibles thermiques

| Repère | Rôle | Choix [H] | Raccordement |
|---|---|---|---|
| T_film | mesure de surface, régulation (50 °C) et défaut (55 °C / 10 s) | **DS18B20** (comme les autres sondes, étalonnée au bain avec les autres : position `film`) collée sur le film A, côté aval | bus du toit via J4/5 (+3,3 V), J4/8 (DQ), J4/2 ou 7 (0 V) |
| BM_A, BM_B | coupure thermique **indépendante du MCU** | bimétal **NF 55 °C** (KSD9700 ou KSD301 si la hauteur le permet), 5 A ; réarmement automatique (le défaut logiciel, lui, est verrouillé) | **en série** dans le +24 V commun des deux films |
| TCO_A, TCO_B | dernier recours, non réarmable | fusible thermique **72 °C**, ≥ 5 A, serti | **en série** avec les bimétaux |

- Un organe par film : chaque film est protégé même si l'autre est froid (débris, flux mal réparti).
- Ordre en série : +24 V (J5/1-2) → BM_A → BM_B → TCO_A → TCO_B → films A et B en parallèle → retour (J5/3-4).
- **Le film ne peut chauffer que si les soufflantes tournent** : interverrouillage logiciel sur les tachymètres de M2 et M3 (coupure au pas suivant, 2 s) ; si le MOSFET est collé et les soufflantes arrêtées, les bimétaux s'ouvrent vers 55 °C, les TCO à 72 °C. Le défaut logiciel `film_surtemp` (55 °C / 10 s) ouvre aussi C4, donc K2, dont le second contact est en série dans le 24 V.

### 7.4 Connecteur : pourquoi deux embases (J4 + J5) plutôt qu'une M12 8 broches

| Critère | M12 8 broches code A seul | **J4 M12-A 8 br. (signaux) + J5 M12-T 4 br. (24 V)** — retenu |
|---|---|---|
| Courant du film | **2 A max par contact** (CEI 61076-2-101, 8 broches) : 3,3 A impossible sur une broche, et aucune broche libre pour doubler | code T : **12 A / 63 V DC par contact** ; 2 broches par pôle → ≈ 1,7 A par contact |
| Nombre de broches | il en faudrait 12 (8 signaux + 2 × 2 puissance) | J4 : 8 signaux (PWM commun) ; J5 : 4 puissance |
| Retour de 3 A | partagé avec le 0 V des tachymètres et du 1-Wire (perturbations, décalage de masse) | retour séparé, 0 V commun en un seul point (source de Q9) |
| Détrompage | — | code T et code A **non accouplables** : inversion impossible |
| Plancher sans film | — | J5 non câblé, capuchon : le même plancher sert aux essais avec et sans variante |

Option si l'apiculteur tient à **un seul geste** de branchement : connecteur circulaire **M16 12 broches IP67** (DIN EN 61076-2-106, ≈ 3–5 A par contact selon la série **[H]**), deux broches par pôle 24 V. Plus encombrant et plus cher, moins courant ; à décider après E14.

### 7.5 Comportement firmware (déjà implémenté, `plancher_chauffant`)

- Désactivé par défaut (`param plancher_chauffant 0`) : aucune exigence sur la sonde du film, comportement inchangé.
- Activé (`param plancher_chauffant 1`, en ATTENTE seulement) : départ refusé si la sonde du film est absente ou non étalonnée (`AT_SONDE_FILM`, 0x2000) ou si `pwm_plancher` = 0 (`AT_FILM_SANS_SOUFFLANTES`, 0x4000).
- En MONTÉE, PALIER et REFROIDISSEMENT : film = demande du toit (TOR sur la sonde couvain la plus froide, limites couvain 43,5 °C et air 44,0 °C comprises) **ET** T surface ≤ 50 °C (reprise à 48 °C) **ET** soufflantes de plancher en marche (tachymètres ≥ 50 % attendu).
- DÉFAUT verrouillé : `film_surtemp` (≥ 55 °C pendant 10 s, avec ouverture de C4) ; `sonde_film` (sonde invalide en cycle, variante active).
- Journal : colonnes `t_film`, `film`, `raisons_film` en fin de ligne (`firmware/README.md`), `tmax_film` dans le résumé.
