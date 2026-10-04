# Peignes de sondes couvain — conception et étalonnage

> Décisions : D3 (sondes indépendantes du toit, connectables), architecture §4.2. Firmware : un **bus 1-Wire par embase** (`brochage.h` : P1 → GPIO 32, P2 → GPIO 33, P3 → GPIO 25), positions mémorisées par ID dans la table d'étalonnage (`etal`, `pos`), peigne débranché en cycle = **DÉFAUT immédiat**.

## 1. Les trois peignes

| Peigne | Emplacement | Sondes | Embase | Couleur | Connecteur |
|---|---|---|---|---|---|
| **P1 — centre** | ruelle centrale du nid à couvain | 3 : **haut** (≈ 50 mm sous la tête de cadre), **centre**, **bas** (≈ 50 mm au-dessus du bas du rayon) | J1 | **rouge** | M8 **4 broches** |
| **P2 — bord gauche** | dernière ruelle contenant du couvain, côté gauche | 1 à mi-hauteur | J2 | **jaune** | M8 **3 broches** |
| **P3 — bord droit** | idem côté droit | 1 à mi-hauteur | J3 | **vert** | M8 **3 broches** |

Gauche / droite : vu **de l'arrière** de la ruche (opérateur derrière la ruche, entrée en face). P2 et P3 sont électriquement interchangeables (le firmware tolère l'inversion et journalise la position de l'**embase**) ; P1 ne peut physiquement pas être branché sur J2/J3 (4 broches contre 3) et le firmware refuse le départ si une sonde déclarée `P1_*` apparaît sur J2/J3 ou une sonde `P2/P3` sur J1.

## 2. Lame

| Paramètre | Valeur | Justification |
|---|---|---|
| Matériau | **circuit imprimé FR4 1,0 mm**, finition ENIG, vernis épargne des deux faces | les pistes sont la lame ; rigide, fin, stable à 45 °C, bon marché à 5 exemplaires |
| Épaisseur totale aux sondes | **≤ 2,5 mm** (FR4 1,0 + DS18B20U µSOP 1,1 + époxy 0,3) | ruelle 8–9 mm entre rayons operculés **[H]** (relevé C4) : il reste ≥ 3 mm de chaque côté pour les abeilles |
| Largeur | **15 mm** | rigidité, place pour 3 pistes + composant |
| Longueur | P1 ≈ **320 mm** ; P2/P3 ≈ **200 mm** [H] | P1 : de la tête de cadre jusqu'à ≈ 50 mm du bas du rayon (cadre Dadant ≈ 300 mm [H], relevé C3) |
| Tête | languette élargie 15 → 25 mm, **épaulement** qui repose sur les têtes de cadres voisines (fixe la profondeur des sondes) ; trou Ø 3 pour un fil de retrait | le peigne ne peut pas tomber au fond, profondeur reproductible |
| Bout | arrondi R 7,5, chanfreiné | glisse dans la ruelle sans arracher d'opercules |
| Inox / fibre de verre (architecture) | non retenus en Phase 1 | une lame inox impose de coller un circuit souple dessus (épaisseur, décollement) ; à réévaluer si le FR4 se propolise mal |

Positions des sondes (centre du boîtier, depuis l'épaulement = dessus des têtes de cadres) **[H]**, à recaler sur le cadre réel :

```
   P1 (centre)                        P2 / P3 (bords)
   ┌─────┐  ← tête 25 mm, épaulement   ┌─────┐
   │  ○  │     trou de retrait         │  ○  │
   └┬───┬┘ ═══ dessus des têtes ═══    └┬───┬┘
    │ ▣ │  50 mm  → sonde HAUT          │   │
    │   │                               │   │
    │ ▣ │ 160 mm  → sonde CENTRE        │ ▣ │ 160 mm → sonde mi-hauteur
    │   │                               │   │
    │ ▣ │ 270 mm  → sonde BAS           └───┘  (≈ 200 mm)
    └───┘  (≈ 320 mm, bout arrondi)
   ▣ = DS18B20U face A, affleurante ; pistes GND / DQ / 3V3 sur face B
```

Sur chaque peigne : sérigraphie **« P1 »**, **« H / C / B »** et une flèche « vers le haut » ; condensateur 100 nF 0402 au pied de chaque DS18B20U.

## 3. Sonde et encapsulation

- Capteur : **DS18B20U** (µSOP-8) authentique (distributeur agréé), alimentation **externe 3,3 V** (pas de mode parasite : conversions fiables à 12 bits sur 1 m de câble).
- La face « sonde » est **affleurante** : on dépose une couche d'**époxy de classe électronique** de 0,3 mm maximum (pochoir de ruban adhésif), puis le **vernis silicone** de tropicalisation sur l'ensemble du peigne (2 couches, 24 h de séchage chacune). Pas de tube saillant (architecture §4.2).
- La jonction câble ↔ lame est noyée dans l'époxy puis couverte de **gaine thermorétractable à colle** 3:1 (soulagement de traction) ; le câble ne doit pas pouvoir tirer sur les pastilles.
- Masse thermique au droit de la sonde : minimale (pas de plan de cuivre sous le boîtier, pistes fines de 0,25 mm) pour une réponse rapide (constante de temps visée < 60 s dans l'eau agitée **[H]**, mesurée en E3).
- Nettoyage : les abeilles propoliseront les peignes ; ils se nettoient à l'alcool (pas d'acétone sur le vernis) ; **contrôle de l'étalonnage après nettoyage** (point de contrôle en E3).

## 4. Câble et brochage M8

- Câble **PUR** 0,25 mm², **1,0 m**, surmoulé côté connecteur mâle M8 (coupé côté peigne et soudé sur les pastilles).
- Longueur de bus : ≤ 1,2 m par embase (1 m de câble + 0,2 m dans le toit) : bien en deçà des limites 1-Wire ; **pull-up 2,2 kΩ** côté nœud (cf. `cablage-phase1.md`).

| Broche M8 | Couleur usuelle câble M8 [H] | P1 (4 broches, J1) | P2 / P3 (3 broches, J2 / J3) |
|---|---|---|---|
| 1 | marron | +3,3 V | +3,3 V |
| 3 | bleu | 0 V | 0 V |
| 4 | noir | DQ (1-Wire) | DQ (1-Wire) |
| 2 | blanc | **pont vers broche 3 (0 V) côté peigne** : permet de vérifier au multimètre que le bon câble est branché ; non lu par le firmware en Phase 1 | — (absente) |

> Les couleurs des câbles surmoulés M8 sont **à vérifier au multimètre** sur le câble acheté. Le brochage 1-3-4 pour les signaux est commun aux connecteurs 3 et 4 broches (codage A), ce qui permet de garder le même câblage côté nœud.

## 5. Étalonnage des DS18B20 au point 42 °C

Objectif : ramener chaque sonde (5 couvain + T_air + T_retour) à **±0,1 °C** de la référence au voisinage de 42 °C (DS18B20 brut : ±0,5 °C). L'offset est enregistré **par ROM** dans la table d'étalonnage (NVS, CRC) et journalisé dans l'en-tête de chaque fichier CSV. Le firmware **refuse un offset > 1,0 °C** (sonde suspecte) et **refuse le départ** si une sonde couvain ou air n'est pas étalonnée.

### Matériel
Bain thermostaté (thermoplongeur dans une glacière, ≥ 15 L d'eau), thermomètre de référence certifié (incertitude ≤ 0,1 °C entre 35 et 50 °C), nœud alimenté **en 5 V par l'USB seulement** (aucun 230 V branché), peignes et sondes du toit, sacs plastiques fins (zip) si les sondes du toit ne sont pas étanches, ruban adhésif, chronomètre.

### Procédure
1. **Préparation** : bain réglé à **42,0 °C**, couvercle percé, brassage du thermoplongeur en marche. Attendre **30 min** de stabilité : la référence ne doit pas varier de plus de ±0,03 °C sur 5 min.
2. **Groupage** : attacher tous les peignes (et les sondes du toit, dans un sac fin étanche, air chassé) **en faisceau autour de la sonde de référence**, sondes dans un rayon de 20 mm de la pointe de référence, à mi-profondeur. Les connecteurs M8 restent **hors de l'eau**.
3. **Branchement** : P1, P2, P3 sur leurs embases ; console série (115 200 bauds) ; `sondes` affiche, par embase, chaque ROM avec `lue=` (valeur **sans** offset).
4. **Stabilisation** : 15 min après immersion.
5. **Relevé** : pendant **10 min**, toutes les minutes, noter la référence et la colonne `lue` de chaque sonde (commande `sondes`). Calculer pour chaque sonde la **moyenne de (T_ref − lue)** = offset, et l'écart-type (doit être ≤ 0,03 °C ; sinon sonde ou bain instable).
6. **Enregistrement** (nœud en ATTENTE) : pour chaque sonde
   `etal <ROM> <offset> <position>` — positions : `P1_haut`, `P1_centre`, `P1_bas`, `P2`, `P3`, `air`, `retour`.
   Raccourci possible si le bain est très stable : `etalref <ROM> <T_ref>` calcule l'offset à partir de la dernière lecture.
   Pour P1, identifier les 3 ROM en réchauffant brièvement une sonde entre deux doigts (hors bain) et en observant laquelle monte.
7. **Vérification au même point** : après enregistrement, `sondes` doit afficher `t` à ±0,10 °C de la référence pour toutes les sondes.
8. **Vérification en deux points de contrôle** (sans recalcul) : bain à **38,0 °C** puis **45,0 °C**, 20 min de stabilisation chacun ; écart toléré **±0,15 °C** (la pente des DS18B20 est supposée correcte sur 7 K **[H]** ; si l'écart dépasse, noter la pente et passer à un étalonnage deux points en Phase 2).
9. **Constante de temps** : sortir le faisceau à l'air ambiant (≈ 20 °C) pendant 10 min, le replonger à 42,0 °C et noter le temps pour atteindre 63 % de l'écart (journal).
10. **Traçabilité** : reporter dans `docs/journal-tests.md` : date, n° de certificat de la référence, T bain, ROM, offset, écart-type, écarts aux points 38/45 °C, constante de temps. Ré-étalonner **à chaque phase**, après tout nettoyage agressif ou remplacement de sonde, et au moins **une fois par saison**.

### Critères d'acceptation
| Critère | Seuil |
|---|---|
| Offset brut par sonde | ≤ ±0,5 °C (au-delà de ±1,0 °C : refusé par le firmware, sonde rebutée) |
| Écart après correction, à 42,0 °C | ≤ ±0,10 °C |
| Écart après correction, à 38,0 et 45,0 °C | ≤ ±0,15 °C |
| Dispersion des 7 sondes à 42 °C après correction | ≤ 0,15 °C |
| Écart-type sur 10 min | ≤ 0,03 °C |

## 6. Mise en place (rappel de la procédure de pose, à vide en Phase 1)
1. Ruche ouverte, cadres en place. 2. Glisser P1 dans la ruelle centrale, sondes face au rayon le plus garni, épaulement sur les têtes de cadres. 3. P2, P3 dans les dernières ruelles « couvain » (à vide : ruelles 2 et 8 sur 10 **[H]**). 4. Faire passer les câbles par les encoches de la plaque centrale. 5. Poser le toit. 6. Brancher J1 (rouge), J2 (jaune), J3 (vert) — **visser les bagues M8 à la main jusqu'en butée** (étanchéité). 7. Vérifier `sondes` : les 3 embases `conforme=1`.
