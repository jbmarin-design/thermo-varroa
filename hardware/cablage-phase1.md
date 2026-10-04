# Câblage — Phase 1 (nœud ruche + module de toit + plancher)

> Référence logicielle : `firmware/include/brochage.h` (le tableau §4 en est la copie exacte ; **toute modification doit être faite aux deux endroits**). Nomenclature : `hardware/bom-phase1.md`. Implantation : `hardware/module-toit-phase1.md`.
> Valeurs marquées **[H]** : à valider au banc (protocole `tests/protocole-phase1.md`).

---

## ⚠️ AVERTISSEMENTS DE SÉCURITÉ 230 V — À LIRE AVANT TOUT TRAVAIL

1. **Le 230 V tue.** Le module de toit contient du 230 V AC (élément de 250 W, relais statique, relais de sécurité, alimentation). Tout câblage, toute modification et toute mesure à l'intérieur du boîtier 230 V se font **prise débranchée** (le câble H07RN-F visiblement débranché et la fiche sous les yeux), **jamais seulement « interrupteur coupé »**.
2. **Un SSR coupé laisse passer un courant de fuite** (quelques mA) et un SSR défaillant est en court-circuit : **la sortie d'un SSR n'est jamais considérée comme hors tension**.
3. Alimenter le banc **uniquement** par une prise protégée par un **disjoncteur différentiel 30 mA testé** (bouton TEST pressé le jour même), un **disjoncteur 10 A** et un **arrêt d'urgence** à portée de main (BOM §3). Pas de rallonge enroulée, pas de multiprise en cascade.
4. L'élément à ailettes est **classe I** : sa gaine métallique, la gaine de chauffe en aluminium et toute pièce métallique accessible sont **reliées à la terre (PE vert/jaune)**. Continuité < 0,1 Ω vérifiée avant chaque mise sous tension (E1).
5. **Séparation TBTS / 230 V** : le boîtier TBTS (ESP32, sondes, C4) ne contient **aucun** conducteur 230 V. Les seules liaisons entre les deux boîtiers sont TBTS (12 V de l'alimentation, bobine de K1, commande du SSR), en câble à **double isolation**, par presse-étoupe. Les sondes et peignes, touchés par l'apiculteur, sont en TBTS.
6. Composants traversant la frontière 230 V / TBTS — ils doivent offrir une **isolation renforcée** (≥ 4 kV) entre les deux côtés : **alimentation PS1** (classe II, IRM-30), **SSR** (isolation entrée/sortie 4 kV), **relais K1** (bobine/contacts) **[H] vérifier la fiche technique du modèle acheté** ; à défaut, choisir un relais à isolation renforcée bobine/contacts.
7. **Mesures sous tension** (E4 et suivantes) : multimètre **CAT III 300 V** minimum, une seule main, cordons à pointes protégées, couvercle du boîtier 230 V **refermé** dès que possible ; les mesures de température se font par la console et par thermomètre IR, **pas en ouvrant le boîtier**.
8. **Ordinateur portable relié en USB pendant que le 230 V est branché** : utiliser un **isolateur USB** (galvanique, ≥ 2,5 kV, type ADuM3160/ADuM4160) — en cas de défaut d'isolement, l'USB relierait l'ordinateur au montage. À défaut, ordinateur **sur batterie**, chargeur débranché.
9. L'élément atteint **> 100 °C** en air immobile : ne **jamais** l'alimenter sans la soufflante (essais de C5 compris : ils se font sous surveillance, sur support incombustible, extincteur CO₂ à proximité).
10. Ce montage est un **prototype d'atelier**, non conforme à une norme produit. Il ne quitte pas le banc avant validation de la Phase 2. Faire relire le câblage 230 V par un électricien qualifié avant la première mise sous tension.

---

## 1. Vue d'ensemble

```
 SECTEUR (banc) ── DDR 30 mA type A ── Disj. 10 A C ── ARRÊT D'URGENCE ── prise IP44
                                                                              │ H07RN-F 3G1,5
 ┌─────────────────────────── BOÎTIER 230 V (dans le toit) ───────────────────┴──────────────┐
 │  L ── F1 2 A T ──┬──────────────────────────────────────────── PS1 L    PS1 230 V → 12 V  │
 │                  │                                                       (classe II)       │
 │                  └── K1/1 (NO) ── SSR1 (1→2) ── F2 bimétal ── F3 TCO ── R1 élément 250 W  │
 │  N ──────────────┬──────────────────────────────────────────── PS1 N           │          │
 │                  └── K1/2 (NO) ───────────────────────────────────────────────┘          │
 │  PE ── bornier PE ── gaine élément, gaine de chauffe alu, tôles                            │
 │                                                                                            │
 │  PS1 +12 V / 0 V ────────────┐   bobine K1 ◄──────┐   SSR1 entrée 3–32 V DC ◄──┐          │
 └──────────────────────────────┼────────────────────┼─────────────────────────────┼──────────┘
              câble double isolation, presse-étoupe M12 (TBTS uniquement)
 ┌──────────────────────────────┼─── BOÎTIER TBTS ───┼─────────────────────────────┼──────────┐
 │  12 V ─ D 1N5819 ─ rail 12 V ├─ U2 12→5 V ─ ESP32 DevKitC (5 V → LDO 3,3 V)    │          │
 │                              ├─ chaîne C4 : NTC RT1 + LM393 + TL431 ─ Q1 ─ K1 + K2         │
 │                              ├─ Q4 (12 V ventilateurs) ─ M1 toit, J4 → M2/M3 plancher       │
 │                              └─ pompe de charge (GPIO 13) ─ Q3 ───────────────────┘        │
 │  ESP32 : 1-Wire ×4, I²C (SHT45, DS3231), µSD, PWM ×3, TACH ×3, LED, boutons S1             │
 │  Façade : S1 départ/acquit · S2 TEST SÉCURITÉ · S3 RÉARMEMENT C4 · LED · J1–J3 M8 · J4 M12  │
 └────────────────────────────────────────────────────────────────────────────────────────────┘
```

Chaîne de coupure de l'élément (toutes **en série**, la chauffe n'existe que si **toutes** sont fermées) :

| Ordre | Organe | Ouvert par | Indépendant du MCU ? | Réarmement |
|---|---|---|---|---|
| 1 | DDR 30 mA + disjoncteur + arrêt d'urgence (banc) | défaut d'isolement, surintensité, opérateur | oui | manuel |
| 2 | F1 2 A T | court-circuit | oui | remplacement |
| 3 | **K1** (2 pôles, phase **et** neutre) | **C4** : NTC > 45,0 °C, NTC coupée/en court-circuit, perte du 12 V, bouton TEST ; ou **MCU** (GPIO 17, ouverture seulement) | **oui** (C4) | **manuel** (S3) |
| 4 | **SSR1** | régulation TOR × sécurité logicielle × enable dynamique | non (sauf enable : C3) | automatique |
| 5 | **F2 bimétal** sur l'élément | T élément > seuil (70–90 °C, fixé en E6) | oui | manuel (bouton du bimétal) |
| 6 | **F3 TCO** sur l'élément | T élément > 102 °C | oui | **remplacement** |

## 2. Câblage 230 V (boîtier 230 V)

| Liaison | Section / type | Remarques |
|---|---|---|
| Entrée H07RN-F 3G1,5 → borniers L, N, PE | 1,5 mm² | presse-étoupe M20 + arrêt de traction ; PE plus long que L/N (dernier arraché) |
| L → F1 (2 A temporisé) | 1 mm² H05V2-K | F1 protège câblage interne, PS1, élément |
| F1 → PS1 L ; N → PS1 N | 1 mm² | PS1 encapsulé, fixé par vis, entrées vissées |
| F1 → K1 contact 11–14 (NO) → SSR1 borne 1 | 1 mm² | K1 coupe la **phase** |
| SSR1 borne 2 → F2 → F3 → R1 | **1 mm² silicone H05SS-F** à partir de la gaine | F2 et F3 serrés sur la semelle de l'élément ; F3 **serti** (manchons non isolés + gaine silicone), jamais soudé |
| R1 → K1 contact 21–24 (NO) → N | 1 mm² silicone puis H05V2-K | K1 coupe aussi le **neutre** (coupure bipolaire : l'élément est isolé même si L/N sont inversés à la prise) |
| Varistance 275 V (S14K275) | aux bornes 1–2 du SSR (si non intégrée) | absorbe les surtensions de coupure |
| PE → gaine de l'élément, gaine de chauffe, tôles, radiateur du SSR (si métallique) | 1,5 mm² vert/jaune, cosses à œillet + rondelle éventail | continuité < 0,1 Ω (E1) |

Le SSR est monté sur son radiateur **hors de la gaine** (dans le boîtier 230 V, côté froid) ; dissipation ≈ 1,2 W à 1,1 A.

## 3. Câblage TBTS (boîtier TBTS)

### 3.1 Alimentations

```
 PS1 +12 V ── D1 1N5819 ──┬── rail +12 V ── C 10 µF + 100 nF
                          ├── U2 (12 V → 5 V 1 A) ── broche « 5V » de la DevKitC ── LDO 3,3 V intégré
                          │                          └─ 1N4148 ─ +4,3 V LED WS2812B (VIH abaissé pour 3,3 V)
                          ├── chaîne C4, bobines K1 + K2
                          ├── Q4 → +12 V ventilateurs (M1, J4 broche 1)
                          └── entrée « + » du SSR1
 PS1 0 V ── 0 V commun TBTS (flottant : non relié à la PE)
```

Budget 12 V **[H]** : soufflante du toit 0,4–0,9 A, plancher 2 × 0,1 A, K1 ≈ 45 mA, K2 ≈ 20 mA, SSR ≈ 12 mA, nœud ≈ 0,25 A → **≈ 1,5 A max**, PS1 2,5 A.
Rail 3,3 V (LDO de la DevKitC, 800 mA) : ESP32 (pointes 250 mA), µSD (≤ 100 mA), DS18B20 ×7, SHT45, DS3231 : < 450 mA.

**USB et 5 V simultanés** : la DevKitC peut être alimentée à la fois par l'USB et par la broche 5 V. Pour éviter un retour de courant vers l'ordinateur, utiliser en Phase 1 l'**isolateur USB** (avertissement 8), qui alimente son côté isolé à partir du montage, ou couper le fil 5 V du câble USB **[H]**.

### 3.2 Sondes 1-Wire (×4 bus)

```
   +3,3 V ── 2,2 kΩ ──┬── 100 Ω ── broche M8 « 4 » (DQ) ── peigne
                      │      └── TVS ESD vers 0 V (côté connecteur)
   GPIO 32 (P1) ──────┘           M8 « 1 » = +3,3 V (via 10 Ω + 100 nF, limite le courant d'un court-circuit)
                                  M8 « 3 » = 0 V
   identique : GPIO 33 → J2 (P2), GPIO 25 → J3 (P3), GPIO 4 → bus interne du toit (T_air + T_retour)
```

Les DS18B20 sont alimentés en **3,3 V externe** (pas de mode parasite). Un bus = une embase : un court-circuit sur un peigne n'affecte que ce peigne (détecté : « peigne Pn » non conforme).

### 3.3 I²C, µSD, LED, bouton

| Fonction | Câblage |
|---|---|
| I²C (SHT45 0x44 + DS3231 0x68) | SDA GPIO 21, SCL GPIO 22, pull-up **4,7 kΩ** vers 3,3 V (supprimer les pull-up des modules s'ils en ont plusieurs) ; SHT45 sur câble 4 fils torsadé ≤ 40 cm ; 100 kHz |
| µSD (SPI) | SCK 18, MISO 19, MOSI 23, CS 5 + **pull-up 10 kΩ** (GPIO 5 = broche de démarrage) ; 10 µF + 100 nF au plus près du module |
| LED WS2812B | DIN ← GPIO 2 via 330 Ω ; VDD = +4,3 V (5 V moins une diode 1N4148) ; 100 nF. GPIO 2 = broche de démarrage : la LED n'impose aucun niveau, sans conséquence |
| S1 départ / acquittement | GPIO 0 → S1 → 0 V, pull-up 10 kΩ, 100 nF anti-rebond. En parallèle du bouton BOOT de la carte : appui pendant un reset = mode téléchargement (sans danger) |
| NTC élément (diagnostic) | +3,3 V → **47 kΩ 1 %** → GPIO 36 → NTC 100 kΩ (verre, fils fibre de verre) → 0 V ; 100 nF sur GPIO 36. La NTC est collée sous une patte de cuivre sur la gaine **reliée à la PE** de l'élément, avec un manchon Kapton : TBTS au contact d'une masse métallique mise à la terre, jamais d'une partie active |

### 3.4 Ventilateurs (M1 toit, M2/M3 plancher)

```
   Alimentation commune commutée :
   rail +12 V ──┬────────────── Q4 source (P-MOSFET AO3401)
                ├─ 10 kΩ ─┬──── Q4 grille           Q4 drain ── +12 V ventilateurs (M1 + J4/1)
                          └─ 10 kΩ ── Q5 collecteur (NPN)    (Vgs = −6 V : dans les limites de l'AO3401)
   GPIO 12 ── 4,7 kΩ ── Q5 base ; 100 kΩ GPIO 12 → 0 V (strapping MTDI : DOIT rester BAS au boot)

   PWM (×3, logique inversée, cf. PWM_VENTILO_INVERSE) :
   GPIO 26 / 27 / 14 ── 1 kΩ ── base Q6 / Q7 / Q8 (NPN) ; collecteur → fil PWM du ventilateur ; émetteur → 0 V
   (le ventilateur a son propre pull-up interne sur PWM : GPIO HAUT = PWM tiré au 0 V)

   Tachymètres (×3) :
   fil TACH ── 1 kΩ ──┬── GPIO 34 / 35 / 15
   +3,3 V ── 10 kΩ ───┤
                      └── 10 nF → 0 V
```

**Vérification obligatoire avant raccordement à l'ESP32** (E1) : ventilateur alimenté en 12 V, fil TACH en l'air : la tension du fil TACH doit être **< 3,6 V** (sortie collecteur ouvert). Un ventilateur dont le TACH monte à 12 V détruirait l'entrée : refuser ou ajouter un pont diviseur.

Brochage connecteurs 4 fils standard (PC) : 1 = 0 V (noir), 2 = +12 V (jaune/rouge), 3 = TACH (vert), 4 = PWM (bleu) — **à vérifier sur la fiche du modèle acheté** (les radiales Delta utilisent souvent d'autres couleurs).

### 3.5 Enable dynamique du SSR (couche C3)

```
   GPIO 13 ── 220 Ω ── C1 470 nF ──┬── D3 (1N4148) ──►|──┬─────────── 4,7 kΩ ── base Q3 (BC547)
     (carré 500 Hz,               │                      │                      émetteur → 0 V
      esp_timer 1 ms)       D2 1N4148 (anode 0 V)     C2 1 µF ─ 0 V              collecteur → SSR « − »
                                  │                   100 kΩ ─ 0 V   rail +12 V ─────────── SSR « + »
                                 0 V
```

- C1 ne laisse passer que les **variations** : GPIO bloqué HAUT ou BAS (MCU planté, en reset, boucle figée, jeton périmé) → plus de charge transférée → C2 se vide dans la base de Q3 et la 100 kΩ en **quelques dizaines de ms** → SSR ouvert.
- Courant disponible ≈ C1 × ΔV × f ≈ 470 nF × 2 V × 500 Hz ≈ 0,5 mA → courant de base ≈ 0,3 mA → Q3 sature pour le courant d'entrée du SSR (≈ 10–15 mA) **[H]** (à mesurer en E2 : tension aux bornes d'entrée du SSR ≥ 4 V avec signal, < 1 V sans signal ; temps de retombée).
- Côté logiciel (déjà implémenté) : le timer ne bascule GPIO 13 que si la boucle principale a demandé la chauffe **et** a rafraîchi son jeton depuis < 3 s ; la boucle est en outre surveillée par le **watchdog de tâche (5 s)**. Commande console `test_gel` pour le vérifier (E4).
- Phase 2 : watchdog **externe** (TPL5010/STWD100) en série avec cette chaîne (architecture C3) — non câblé en Phase 1.

### 3.6 Chaîne de sécurité matérielle C4 (indépendante du MCU)

```
   rail +12 V ── 2,2 kΩ ──┬── VREF = 2,495 V (U7 TL431, réf. reliée à la cathode) ── 1 µF
                          │
          ┌───────────────┼──────────────────────────┬───────────────────────────┐
        10,0 kΩ 0,1 %   10,0 kΩ 0,1 %               10 kΩ 1 %                     │
          │ nœud A        │ nœud B                   │ nœud D (≈ 2,20 V)          │
          │               │                          │                            │
    S2 (TEST, inverseur)  R_seuil ≈ 4,32 kΩ 0,1 %    75 kΩ 1 %                    │
     NF → RT1 NTC 10 kΩ   (= R(45,0 °C) de la NTC,   │                            │
          (grille de      ajustée en E3)             0 V                          │
          soufflage)      │                                                       │
     NO → R_test 4,12 kΩ  0 V                                                     │
          (≈ 46,2 °C)                                                             │
          │   10 nF                                                               │
          0 V                                                                     │
                                                                                  │
   U6A (LM393) : IN+ = A, IN− = B  → sortie ouverte (OK) si A > B  (NTC < 45,0 °C)
   U6B (LM393) : IN+ = D, IN− = A  → sortie ouverte (OK) si A < D  (NTC présente, > ≈ −15 °C)
   Sorties U6A et U6B (collecteur ouvert) reliées au nœud G (grille de Q1)  = ET câblé

   Auto-maintien et réarmement :
   rail +12 V ──┬── K2 contact NO ──┐
                └── S3 RÉARMEMENT ──┴── nœud P ── 10 kΩ ── nœud G ── grille Q1 (BS170)
                                                              │
   GPIO 17 ── 4,7 kΩ ── base Q2 (NPN) ; collecteur Q2 → G ;   100 kΩ GPIO 17 → 0 V
   Q1 drain ── bobines K1 et K2 en parallèle (diode 1N4148 de roue libre) ── rail +12 V
           └── 2,2 kΩ ── LED de U3 (PC817) ── rail +12 V   (en parallèle sur la bobine K1)
   U3 phototransistor : collecteur → GPIO 16 (+ 10 kΩ vers 3,3 V, 10 nF vers 0 V), émetteur → 0 V
```

Fonctionnement :
- **Seuil ratiométrique** : A et B sont alimentés par la même référence ; la décision ne dépend que du rapport R_NTC / R_seuil → la précision de la TL431 et de l'alimentation n'intervient pas. À 45 °C : V_A ≈ 0,752 V, pente ≈ −20,5 mV/K.
- **NTC en court-circuit** → A ≈ 0 → « chaud » → coupure. **NTC coupée** → A ≈ 2,495 V > D (2,20 V) → coupure (U6B). **Perte du 12 V** → K1/K2 retombent.
- **Aucune hystérésis** sur U6A : la retombée est **verrouillée** par K2 (une hystérésis par résistance de réaction décalerait le seuil de plus de 1 K via le pull-up partagé).
- **Auto-maintien** : Q1 n'est commandé que si le nœud P est alimenté, c'est-à-dire si K2 est déjà collé **ou** si S3 est pressé. Une fois K1/K2 retombés (dépassement, NTC débranchée, coupure secteur, ouverture par le MCU, test), **ils restent ouverts** même si la cause disparaît, jusqu'à un appui sur **S3**, qui n'a d'effet que si les deux comparateurs sont « OK » et si le MCU ne demande pas l'ouverture.
- **Le MCU peut ouvrir, jamais fermer** : Q2 ne peut que tirer G à 0 V. Au démarrage, GPIO 17 flottant est maintenu bas par 100 kΩ.
- **Retour d'état** : GPIO 16 BAS = K1 excité = chaîne armée (lu par le firmware : `c4_fermee`) ; fil coupé = HAUT = « ouverte » → départ refusé (`AT_C4_OUVERTE`) ou DÉFAUT en cycle (`C4_ouverte`).
- **Test à la pose** : S2 substitue R_test (≈ 46,2 °C) à la NTC → K1 doit retomber et **rester** retombé après relâchement → réarmer par S3.
- **Limite connue** : S3 maintenu enfoncé court-circuite l'auto-maintien (pas les comparateurs) → bouton à collerette, et contrôle dans E3.

Sélection de R_seuil : lire dans la table R/T du fabricant de RT1 la valeur R(45,0 °C) (≈ 4,31 kΩ pour B25/85 = 3977, NTCLE100E3103) ; monter la valeur 0,1 % la plus proche, puis **ajuster au bain** (E3 : point de basculement visé 45,0 ± 0,2 °C ; ≈ 166 Ω/K : une résistance série de 33 Ω décale d'environ +0,2 K vers le froid).

## 4. Brochage ESP32 (copie exacte de `firmware/include/brochage.h`)

Carte : **ESP32-DevKitC-32E (module WROOM-32E)**. Ne **pas** utiliser un module **WROVER** : ses GPIO 16 et 17 sont réservés à la PSRAM.

| GPIO | Constante (`broche::`) | Fonction | Sens | Électronique externe | Remarque de démarrage |
|---|---|---|---|---|---|
| 32 | `OW_P1` | 1-Wire embase P1 (peigne centre, 3 sondes) | E/S | pull-up 2,2 kΩ, 100 Ω série, TVS | — |
| 33 | `OW_P2` | 1-Wire embase P2 (bord gauche, 1 sonde) | E/S | idem | — |
| 25 | `OW_P3` | 1-Wire embase P3 (bord droit, 1 sonde) | E/S | idem | — |
| 4 | `OW_TOIT` | 1-Wire interne : T_air (grille de soufflage) + T_retour (aspiration) | E/S | idem | — |
| 21 | `I2C_SDA` | I²C SDA (SHT45 0x44, DS3231 0x68) | E/S | pull-up 4,7 kΩ | — |
| 22 | `I2C_SCL` | I²C SCL | S | pull-up 4,7 kΩ | — |
| 18 | `SD_SCK` | µSD SCK | S | — | — |
| 19 | `SD_MISO` | µSD MISO | E | — | — |
| 23 | `SD_MOSI` | µSD MOSI | S | — | — |
| 5 | `SD_CS` | µSD CS | S | **pull-up 10 kΩ** | strapping : HAUT au boot |
| 13 | `SSR_ENABLE` | enable dynamique : carré 500 Hz → pompe de charge → Q3 → SSR | S | 220 Ω, C1 470 nF, D2/D3, C2 1 µF | — |
| 17 | `RELAIS_TRIP` | HAUT = le MCU **ouvre** la chaîne C4 (Q2) | S | 4,7 kΩ, **pull-down 100 kΩ** | flottant au boot = pas d'ouverture |
| 16 | `C4_ETAT` | retour d'état C4 (U3 PC817) : BAS = armée | E | pull-up 10 kΩ + 10 nF | — |
| 26 | `PWM_TOIT` | PWM soufflante du toit M1 (inversé) | S | 1 kΩ → Q6 | — |
| 27 | `PWM_PLANCHER_A` | PWM soufflante plancher M2 (avant) (inversé) | S | 1 kΩ → Q7 | — |
| 14 | `PWM_PLANCHER_B` | PWM soufflante plancher M3 (arrière) (inversé) | S | 1 kΩ → Q8 | bref signal au boot : sans conséquence |
| 34 | `TACH_TOIT` | tachymètre M1 | E (entrée seule) | **pull-up 10 kΩ externe** + 1 kΩ/10 nF | pas de pull-up interne |
| 35 | `TACH_PLANCHER_A` | tachymètre M2 | E (entrée seule) | idem | idem |
| 15 | `TACH_PLANCHER_B` | tachymètre M3 | E | pull-up 10 kΩ + 1 kΩ/10 nF | strapping MTDO : HAUT au boot = normal |
| 12 | `ALIM_VENTILOS` | HAUT = 12 V sur les 3 soufflantes (Q5 → Q4) | S | 4,7 kΩ, **pull-down 100 kΩ** | **strapping MTDI : DOIT être BAS au boot — jamais de pull-up** |
| 36 | `NTC_ELEMENT` | NTC élément (ADC1_CH0), **diagnostic seulement** | E analogique | 47 kΩ 1 % vers 3,3 V, 100 nF | — |
| 2 | `LED_ETAT` | LED WS2812B (1 pixel) | S | 330 Ω | strapping (download) : sans conséquence |
| 0 | `BOUTON` | S1 départ / acquittement, actif BAS | E | pull-up 10 kΩ + 100 nF | appui au reset = mode téléchargement |
| 39 | — | **LIBRE, ne pas utiliser** | — | — | errata ESP32 3.11 : fausses impulsions avec l'ADC actif sur GPIO 36 |

`PWM_VENTILO_INVERSE = true` : étage collecteur ouvert, un GPIO HAUT tire le fil PWM au 0 V. Réseau NTC élément : `R_SERIE_OHM = 47 kΩ`, `R0_OHM = 100 kΩ`, `BETA = 3950` **[H]**.

## 5. Connecteurs de façade

**J1 – P1, M8 4 broches (rouge)** · **J2 – P2, M8 3 broches (jaune)** · **J3 – P3, M8 3 broches (vert)** (détails : `peignes-sondes.md` §4)

| Broche M8 | Signal | Côté nœud |
|---|---|---|
| 1 | +3,3 V (via 10 Ω) | rail 3,3 V |
| 3 | 0 V | 0 V |
| 4 | DQ 1-Wire | GPIO 32 / 33 / 25 (via 100 Ω) |
| 2 (J1 seulement) | pont vers 0 V côté peigne (identification au multimètre) | non connecté en Phase 1 |

**J4 – plancher, M12 8 broches** (détails : `plancher-phase1.md` §4)

| Broche M12 | Signal | Côté nœud |
|---|---|---|
| 1 | +12 V ventilateurs (commuté) | drain Q4 |
| 2 | 0 V | 0 V |
| 3 | PWM M2 | collecteur Q7 (GPIO 27) |
| 4 | TACH M2 | GPIO 35 (pull-up, RC) |
| 5 | PWM M3 | collecteur Q8 (GPIO 14) |
| 6 | TACH M3 | GPIO 15 (pull-up, RC) |
| 7 | 0 V (doublé) | 0 V |
| 8 | réserve | non connecté |

**Boutons et voyant (face latérale)** : S1 noir (départ/acquit, GPIO 0), S2 jaune (TEST SÉCURITÉ, C4), S3 bleu à collerette (RÉARMEMENT C4), LED WS2812B (état firmware). Code couleur de la LED : bleu = ATTENTE (orange : µSD/SHT45 absents), blanc = AUTOTEST, jaune clignotant = MONTÉE, vert clignotant = PALIER, cyan = REFROIDISSEMENT, vert fixe = FIN, rouge = DÉFAUT (clignotant rapide : surtempérature).

## 6. Sondes intégrées au toit (non connectables)

| Repère | Capteur | Emplacement | Raccordement | Lu par |
|---|---|---|---|---|
| T_air | DS18B20 (fourreau inox) | porte-sonde de la grille de soufflage, face au jet | bus `OW_TOIT` (GPIO 4), position `air` | MCU (limite 44,0 / défaut 44,5 °C) |
| T_retour | DS18B20 (fourreau inox) | milieu de la bande d'aspiration | bus `OW_TOIT`, position `retour` | MCU (diagnostic, bilan de débit) |
| RT1 (S) | NTC 10 kΩ 1 % | **même porte-sonde que T_air**, à ≤ 10 mm | chaîne C4 uniquement | **non lue par le MCU** |
| T_elem | NTC 100 kΩ verre | semelle de l'élément | GPIO 36 | MCU (diagnostic, réglage C5) |
| H1 | SHT45 + membrane PTFE | sous la plaque centrale, hors flux | I²C | MCU (journal ; avertissement si absent) |
| F2, F3 | bimétal + TCO | semelle de l'élément, pâte thermique | série 230 V | — |

## 7. Contrôles de câblage

Les contrôles hors tension, la première mise sous tension (sans élément, avec lampe témoin) et les tests de la chaîne C4 sont décrits dans `tests/protocole-phase1.md` (E1 à E4). **Aucune mise sous tension 230 V avant que E1 soit entièrement conforme.**
