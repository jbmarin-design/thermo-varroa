# Plancher fermé — Phase 1 (plancher Nicot aéré existant)

> Décisions : D2 (plancher fermé = plénum de retour), D9 (boucle avant ↔ arrière), architecture §1.2 « Plancher : fermer un plancher Nicot existant » et « Ventilateurs de plancher : recommandés ».
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
   │          ○ passe-câble (presse-étoupe M12 dans la plaque)         │
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
| Câble | **M12 8 broches** surmoulé PUR 8 × 0,25 mm², 1,0 m, côté plancher ; il sort du plancher par un **presse-étoupe M12** percé dans la plaque d'obturation (ou une encoche latérale du cadre du plancher garnie de silicone), remonte le long du corps **à l'extérieur**, et se branche sur l'embase **J4** en façade du toit. |
| Dans le plénum | les 2 × 4 fils des soufflantes arrivent sur une petite **plaquette de jonction** vernie (soudure + gaine thermo) fixée sur la plaque ; aucun fil sous tension libre dans le plénum. |
| Tension | **12 V uniquement** (TBTS), commutée par Q4 (`ALIM_VENTILOS`, GPIO 12). |

Brochage de J4 (M12 code A, 8 broches) — **identique** au tableau de `cablage-phase1.md` §5 :

| Broche M12 | Couleur usuelle (câble surmoulé M12 8 br.) [H] | Signal | GPIO ESP32 |
|---|---|---|---|
| 1 | blanc | +12 V soufflantes (commuté) | via Q4 (GPIO 12) |
| 2 | marron | 0 V | — |
| 3 | vert | PWM M2 (plancher A, avant) | GPIO 27 (via Q7, collecteur ouvert) |
| 4 | jaune | TACH M2 | GPIO 35 |
| 5 | gris | PWM M3 (plancher B, arrière) | GPIO 14 (via Q8, collecteur ouvert) |
| 6 | rose | TACH M3 | GPIO 15 |
| 7 | bleu | 0 V (retour, doublé) | — |
| 8 | rouge | non connecté (réserve : sonde de plénum Phase 2) | — |

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
