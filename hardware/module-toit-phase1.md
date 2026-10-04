# Module de toit — Phase 1 (toit Nicotplast Dadant 10 équipé)

> Décisions appliquées : D1, D2, D9, D10 (`docs/architecture.md`). Matériel : `hardware/bom-phase1.md`. Câblage : `hardware/cablage-phase1.md`.
> **Toutes les cotes ci-dessous sont des hypothèses [H] dérivées des fiches Nicotplast et du standard Dadant 10** ; elles seront remplacées par les cotes relevées sur le toit réel (§8) **avant** toute découpe. Travailler sur un **second toit** du stock pour pouvoir recommencer.

## 1. Principe

```
                 AVANT (porte Nicot)                                      ARRIÈRE
   ┌──────────────────────────────── toit Nicot équipé ───────────────────────────────┐
   │  ◄── aspiration ◄───────── gaine de chauffe (élément + soufflante) ◄───────     │
   │  bande d'ASPIRATION        plaque centrale ÉTANCHE           bande de SOUFFLAGE │
   ├────▲──────────────────────────────────────────────────────────────▼───────────┤
   │    ▲ l'air remonte par les ruelles      (rayons)       l'air descend ▼          │
   ├────▲──────────────────────────────────────────────────────────────▼───────────┤
   │    ◄─────────────── plénum du plancher fermé + 2 soufflantes 40 mm ◄────────────│
   └────────────────────────────────────────────────────────────────────────────────┘
```

Sens initial retenu : **soufflage à l'ARRIÈRE, aspiration à l'AVANT** (l'air le plus chaud descend côté arrière, loin de la porte à aérations, et remonte côté avant). Le sens inverse est testé au protocole E11 en **retournant le toit de 180°** : le cloisonnement étant symétrique, il suffit de retourner aussi le sens de la gaine (soufflante démontable, §4) **[H]**.

## 2. Point critique : le débit d'air (à lire avant de choisir la soufflante)

Bilan sur l'air de la boucle (ρ·cp ≈ 1,1 kJ/m³·K) — puissance transportée P = débit × 1,1 kJ/m³·K × ΔT (air soufflé − air de retour) :

| Débit de boucle | Puissance transportée si ΔT = 4 K | ΔT nécessaire pour transporter 250 W |
|---|---|---|
| 10 m³/h | 12 W | 81 K |
| 30 m³/h | 37 W | 27 K |
| 60 m³/h | 73 W | 14 K |
| 120 m³/h | 147 W | 7 K |

Conséquences **[H]** :
- Le débit de 5–15 m³/h envisagé dans l'architecture (§1.2) **ne permet pas** de transporter la puissance de palier estimée (95–150 W, §5.2) avec un air soufflé plafonné à 44,0 °C et un couvain à 42,3 °C (ΔT ≈ 2–4 K). Il faudrait ≈ 80–120 m³/h.
- Avec un débit plus faible, la limite air (44,0 °C) va **couper la chauffe en permanence** et la montée sera très lente (le firmware le détectera : « chauffe inefficace » ou « timeout montée »).
- Une partie de la chaleur passe aussi par conduction dans les parois et la masse, et à vide les pertes seront plus faibles qu'avec une colonie : **seule la mesure tranchera**. C'est l'objet des essais E7 à E12.

Choix pour la Phase 1 : une soufflante radiale **≥ 30 m³/h à l'air libre** (Delta BFB1012 ou BFB1212, §BOM 4) plutôt qu'un format 75 × 75, **pilotée en PWM** pour explorer 30–100 % ; mesurer les débits réels (anémomètre aux fentes, ou bilan thermique : `P_élément ≈ débit × 1,1 × (T_air − T_retour)` — c'est pour cela qu'une sonde **T_retour** a été ajoutée dans la bande d'aspiration).

Conséquence sécurité : plus le débit est élevé, plus l'écart élément ↔ air soufflé est faible et plus la marge entre la limite 44,0 °C et la coupure C4 à 45,0 °C est confortable.

## 3. Vue de dessus (toit retourné, côté cadres) — cotes [H]

Intérieur supposé du toit : **≈ 500 × 420 mm** (couvre-cadre), hauteur 100 mm. Intérieur du corps : ≈ 450 × 375 mm. Les cadres sont **perpendiculaires à l'entrée** : les ruelles vont de l'avant vers l'arrière (sens vertical du schéma).

```
                         ARRIÈRE (côté opposé à l'entrée)
    ┌──────────────────────────────── 500 ─────────────────────────────────┐
    │ ┌──────────────────── corps : 450 × 375 (intérieur) ───────────────┐ │
    │ │▓▓▓▓▓▓▓▓▓▓ BANDE DE SOUFFLAGE ≈ 94 mm (1/4) ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ │
    │ │▓  grille inox perforée Ø2  ·  T_air ◉  + NTC C4 ◉ (côte à côte) ▓│ │
    │ ├──────────────────────────────────────────────────────────────────┤ │
    │ │                                                                  │ │
    │ │       PLAQUE CENTRALE ÉTANCHE ≈ 187 mm (1/2)  PVC 5 mm           │ │
    │ │       posée sur les têtes de cadres, joint EPDM en périphérie    │ │
    │ │                                                                  │ │
    │ │   ○ fente passe-peignes P2   ○ P1 (centre)   ○ P3  (encoches     │ │
    │ │     dans la plaque, bouchées mousse autour des câbles)           │ │
    │ │                                                     SHT45 □      │ │
    │ ├──────────────────────────────────────────────────────────────────┤ │
    │ │░░░░░░░░░░ BANDE D'ASPIRATION ≈ 94 mm (1/4) ░░░░░░░░░░░░░░░░░░░░░░│ │
    │ │░  grille inox perforée Ø2  ·  T_retour ◉                         ░│ │
    │ └──────────────────────────────────────────────────────────────────┘ │
    └────────────────────────────────── 420 (profondeur) ──────────────────┘
                         AVANT (porte Nicot à aérations)
    Les ruelles (entre rayons) sont orientées AVANT ↕ ARRIÈRE : chaque ruelle reçoit
    l'air sur toute la largeur de la bande de soufflage et le rend sur la bande d'aspiration.
```

Proportions de départ : **1/4 – 1/2 – 1/4** de la profondeur intérieure du corps (375 mm → ≈ 94 / 187 / 94 mm). La plaque centrale est **réglable** : deux réglettes de 30 mm (PVC ou ASA) vissées sur ses bords permettent de passer à 1/5 – 3/5 – 1/5 (75 / 225 / 75 mm) ou 1/3 – 1/3 – 1/3 (protocole E12) sans refaire le toit.

## 4. Vue de dessus (dans le toit, au-dessus du cloisonnement) — implantation

```
                                  ARRIÈRE
   ┌────────────────────────────────────────────────────────────────────────────┐
   │ XPS 20 ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ XPS 20 │
   │ ▒ ┌──────────── PLÉNUM DE SOUFFLAGE (au-dessus de la bande arrière) ────┐ ▒ │
   │ ▒ │  air chaud ↓↓↓ vers la grille, déflecteur de répartition perforé    │ ▒ │
   │ ▒ └───────────────────────────▲─────────────────────────────────────────┘ ▒ │
   │ ▒                 ┌───────────┴───────────┐                               ▒ │
   │ ▒                 │  GAINE DE CHAUFFE     │  tôle alu 0,8, ≈150 × 45      ▒ │
   │ ▒  ┌────────────┐ │  ┌─────────────────┐  │  feutre céramique 6 mm autour ▒ │
   │ ▒  │ BOÎTIER    │ │  │ ÉLÉMENT 250 W   │  │  T_elem (NTC 100k) + F2 + F3  ▒ │
   │ ▒  │ 230 V      │ │  │ à ailettes      │  │  sur la semelle de l'élément  ▒ │
   │ ▒  │ F1 K1 SSR  │ │  └─────────────────┘  │                               ▒ │
   │ ▒  │ PS1        │ │  ┌─────────────────┐  │                               ▒ │
   │ ▒  └────────────┘ │  │ SOUFFLANTE M1   │  │  radiale ≈ 97×97×33 ou 120     ▒ │
   │ ▒  ┌────────────┐ │  │ (aspire en haut,│  │  à plat, sortie vers l'élément ▒ │
   │ ▒  │ BOÎTIER    │ │  │  souffle →)     │  │                               ▒ │
   │ ▒  │ TBTS       │ │  └────────▲────────┘  │                               ▒ │
   │ ▒  │ ESP32, C4, │ └───────────┼───────────┘                               ▒ │
   │ ▒  │ RTC, µSD   │ ┌───────────┴─────────── PLÉNUM D'ASPIRATION ─────────┐ ▒ │
   │ ▒  └────────────┘ │  air de retour ↑↑↑ depuis la grille avant           │ ▒ │
   │ ▒                 └──────────────────────────────────────────────────────┘ ▒ │
   │ XPS 20 ▒▒▒  ●J1 P1(rouge) ●J2 P2(jaune) ●J3 P3(vert) ●J4 M12 plancher ▒▒▒▒▒ │
   │             (embases sur la face AVANT, sous l'auvent du toit)              │
   └────────────────────────────────────────────────────────────────────────────┘
                                   AVANT
     Presse-étoupes : M20 câble 230 V (face arrière, côté boîtier 230 V) ;
     boutons S1 départ / S2 test sécurité / S3 réarmement C4 + LED : face latérale.
```

Règles d'implantation :
1. **Côté froid / côté chaud** : les deux boîtiers électroniques sont **hors du flux d'air**, séparés de la gaine par ≥ 30 mm de XPS. L'électronique voit au plus ≈ 45 °C (air ambiant du toit) : composants choisis ≥ 70 °C.
2. **Élément en aval de la soufflante** (soufflante → élément → plénum de soufflage) : la soufflante aspire de l'air de retour à ≈ 42 °C, jamais l'air chaud, et l'élément est toujours balayé.
3. **Élément inaccessible** (C0) : deux grilles inox Ø 2 mm séparent l'élément des abeilles ; le plénum de soufflage contient un **déflecteur perforé** qui répartit l'air sur toute la largeur et casse le jet chaud.
4. **NTC C4 et T_air côte à côte** dans un porte-sonde collé à la grille de soufflage, au **point le plus chaud accessible aux abeilles** (sortie du plénum, face au jet). Leur écart est mesuré au protocole E9.
5. **T_retour** au milieu de la bande d'aspiration (diagnostic : bilan thermique et débit).
6. **SHT45** sous la plaque centrale, **hors flux direct**, membrane PTFE vers le bas, à 30 mm d'une encoche passe-peignes.
7. Le **boîtier 230 V et le boîtier TBTS sont distincts** (cf. `cablage-phase1.md` §1). Aucun conducteur 230 V ne traverse le boîtier TBTS.

## 5. Coupe AVANT ↔ ARRIÈRE (dans l'axe d'une ruelle) — hauteurs [H]

```
   hauteur (mm, depuis le bas du toit)
   100 ┌─ coque Nicot ──────────────────────────────────────────────────────────┐
    95 │▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ XPS 20 (dessus) ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
    75 │  ┌────────┐    ┌─────────── gaine 45 mm ───────────┐                    │
       │  │boîtiers│    │ M1 ──► élément ──► (feutre céram.) ├─► plénum soufflage│
    30 │  └────────┘    └───────────────────────────────────┘     │              │
       │ ◄── plénum d'aspiration (≥ 20 mm) ──────────────────     ▼ déflecteur   │
    10 │══ PVC 5 mm : bande ASPI. ═╪═══ PLAQUE CENTRALE ═══╪═ bande SOUFFLAGE ══ │
     5 │░░ grille Ø2 ░░░░░░░░░░░░░ │ joint EPDM 10×5      │ ▓▓▓ grille Ø2 ▓▓▓▓▓ │
     0 └───────────────────────────┴──────────────────────┴─────────────────────┘
       ▲ remontée (ruelles avant)    têtes de cadres          ▼ descente (arrière)
       AVANT                                                               ARRIÈRE
```

- Le **plan de joint** du cloisonnement doit être **au ras des têtes de cadres** (espace « bee space » ≈ 8 mm au-dessus des cadres dans un toit Nicot posé sur le corps **[H]**). Si l'espace sous le couvre-cadre est plus haut, rehausser la plaque centrale par des tasseaux pour qu'elle **appuie** sur les têtes de cadres (joint EPDM 5 mm comprimé de 30 à 50 %).
- Gaine : section **≥ 150 × 45 mm** (≥ 65 cm²) pour rester < 2 m/s à 50 m³/h ; plénums ≥ 20 mm de haut.
- Isolant : **XPS 20 mm** sur le dessus et les quatre côtés intérieurs ; **feutre céramique 6 mm + tôle alu** dans un rayon de 30 mm autour de l'élément (XPS limité à 75 °C).
- Le **couvre-cadre isolant Nicot** (500 × 420) est **retiré** : il est remplacé par le cloisonnement.

## 6. Cloisonnement — réalisation

| Pièce | Matériau | Cote [H] | Remarques |
|---|---|---|---|
| Cadre-support périphérique | PVC expansé 5 mm, tasseaux 10 × 10 | intérieur du toit | vissé/collé dans le toit, porte les trois zones |
| Grille de soufflage | inox perforé Ø 2 mm | ≈ 375 × 94 | encadrée PVC, porte-sonde T_air + NTC C4 |
| Plaque centrale | PVC expansé 5 mm + joint EPDM 10 × 5 en périphérie | ≈ 375 × 187 | 3 encoches 25 × 6 mm passe-câbles des peignes, bouchées mousse |
| Réglettes de réglage | PVC 5 mm ou ASA | 2 × (375 × 30) | changent les proportions sans démontage du toit |
| Grille d'aspiration | inox perforé Ø 2 mm | ≈ 375 × 94 | porte-sonde T_retour |
| Joint toit / corps | EPDM cellules fermées 10 × 5 | périphérie | évite que l'air ne court-circuite par la jonction toit/corps |
| Joint latéral | mousse EPDM | côtés de la plaque centrale | ferme les espaces entre cadres de rive et parois (court-circuit latéral) |

Étanchéité du court-circuit : avec la soufflante en marche et le toit posé sur le corps **vide**, une feuille de papier fin posée sur la plaque centrale ne doit pas bouger ; un bâton d'encens aux jonctions montre les fuites (protocole E10).

## 7. Passages de câbles

| Passage | Moyen | Remarques |
|---|---|---|
| Câble d'alimentation 230 V (H07RN-F 3G1,5) | **presse-étoupe M20 IP68** dans la face arrière, directement dans le boîtier 230 V ; **arrêt de traction** (collier + bride) | boucle d'égouttage à l'extérieur (le câble descend avant de remonter) |
| Peignes P1, P2, P3 | **embases M8** sur la face avant, sous l'auvent ; les câbles des peignes remontent par les 3 encoches de la plaque centrale puis sortent par une **fente de 6 × 40 mm** entre toit et corps, garnie de mousse | P1 = 4 broches (rouge), P2/P3 = 3 broches (jaune, vert) |
| Plancher (2 soufflantes) | **embase M12 8 broches** face avant ; câble descendant le long du corps à l'extérieur, plaqué par 2 adhésifs | voir `plancher-phase1.md` |
| TBTS ↔ 230 V (12 V de PS1, bobine K1, commande SSR, élément ↔ F2/F3) | **presse-étoupe M12** entre les deux boîtiers, câbles double isolation | jamais de 230 V dans le boîtier TBTS |
| Sondes intégrées (T_air, T_retour, NTC C4, NTC élément, SHT45) | fils silicone le long de la gaine, **colliers inox**, entrée dans le boîtier TBTS par presse-étoupe M12 | NTC élément : fils fibre de verre (250 °C) jusqu'à 30 mm de l'élément |
| USB (console) | câble USB passant par le même presse-étoupe que les sondes, bouchon quand inutilisé | Phase 1 seulement |

## 8. Cotes à relever sur le toit, le corps et le plancher réels

À reporter dans `docs/journal-tests.md` (ligne « Cotes Phase 1 ») **avant** découpe. Mesurer au pied à coulisse (± 0,5 mm) sur **2 exemplaires** de chaque pièce.

| N° | Cote | Pour quoi faire |
|---|---|---|
| T1 | Intérieur du toit : longueur × largeur au niveau du bord inférieur | cadre-support, cloisonnement |
| T2 | Intérieur du toit : longueur × largeur à mi-hauteur et sous le plafond (dépouille) | isolant, implantation des boîtiers |
| T3 | Hauteur intérieure utile (du bord inférieur au plafond) | empilement isolant + gaine + plénums (100 mm annoncés) |
| T4 | Épaisseur des parois et du plafond du toit, matériau (PP, PVC ?) | perçages, tenue en température, choix du forêt |
| T5 | Hauteur et profondeur de l'**emboîtement** toit ↔ corps (recouvrement) | position du plan de joint, joint périphérique |
| T6 | Position, forme et surface des **aérations** existantes du toit | à obturer (ruban alu) ou à garder hors boucle |
| T7 | Forme du dessous du plafond : nervures, renforts, bossages | fixation du cadre-support |
| T8 | Hauteur de l'**auvent** en façade, zone plane disponible pour les embases M8/M12 et les boutons | perçages Ø 12–16 mm |
| C1 | Intérieur du corps : longueur (avant ↔ arrière) × largeur, en haut et en bas | proportions des bandes 1/4 – 1/2 – 1/4 |
| C2 | Hauteur intérieure du corps ; distance têtes de cadres ↔ bord supérieur du corps | plan d'appui de la plaque centrale |
| C3 | Cadres : longueur hors tout, longueur de la tête (avec oreilles), épaisseur de la tête, largeur des montants, hauteur du cadre | encoches passe-peignes, position des sondes |
| C4 | Pas des cadres (entraxe) et largeur réelle des ruelles avec cadres bâtis | épaisseur max des peignes (cf. `peignes-sondes.md`) |
| C5 | Espace entre cadres de rive et parois (gauche, droite), entre bout des cadres et parois avant/arrière | court-circuits latéraux, joints |
| C6 | Feuillures de suspension des cadres : profondeur, matériau | butée des peignes |
| P1–P6 | Plancher : voir `plancher-phase1.md` §6 | |

Photos à prendre (à joindre au journal) : dessous du toit, dessus du corps garni de cadres (vue de dessus avec réglet), plancher vu de dessous, glissière du lange.

## 9. Assemblage (ordre conseillé)

1. Relever les cotes (§8), mettre à jour ce document et les gabarits.
2. Découper le cadre-support, les grilles et la plaque centrale ; **essai à blanc** sur un corps garni de cadres : le toit doit se poser à fond et la plaque centrale appuyer sur les têtes de cadres.
3. Fabriquer la gaine (tôle pliée, rivets), y fixer l'élément (vis inox + rondelles, **pas de colle**), F2 et F3 serrés contre la semelle de l'élément avec pâte thermique, la NTC élément sous une patte de cuivre.
4. Poser l'isolant (feutre céramique autour de la gaine, XPS ailleurs).
5. Monter les boîtiers 230 V et TBTS, les presse-étoupes, les embases, les boutons.
6. Câbler selon `cablage-phase1.md`, **contrôles hors tension** (protocole E1) avant toute mise sous tension.
