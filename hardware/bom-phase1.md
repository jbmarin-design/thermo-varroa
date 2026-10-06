# Nomenclature — Phase 1 (prototype mono-ruche, banc à vide)

> Référence : `docs/architecture.md` (révision 2, décisions D1–D21), `firmware/include/brochage.h`.
> Toutes les **références** sont des **types** à titre d'exemple (équivalents acceptés s'ils respectent les caractéristiques) et tous les **prix** sont **indicatifs, TTC, octobre 2026, non vérifiés : [H]**. Ils servent à estimer un budget, pas à commander.
> Repères (K1, Q3, U2…) : ceux de `hardware/cablage-phase1.md`.

## 0. Récapitulatif budgétaire [H]

| Bloc | Montant indicatif |
|---|---|
| 1. Électronique du nœud | ≈ 95 € |
| 2. Chaîne de sécurité C4 / C5 | ≈ 55 € |
| 3. Puissance 230 V | ≈ 115 € |
| 4. Ventilation | ≈ 75 € |
| 5. Sondes et peignes | ≈ 85 € |
| 6. Connectique et câbles | ≈ 110 € |
| 7. Mécanique du toit et du plancher | ≈ 85 € |
| 8. Étalonnage et essais | ≈ 330 € |
| **Total (hors imprimante 3D, hors outillage courant)** | **≈ 950 €** |
| 10. Variante **plancher chauffant d'appoint** (D21, option pour l'essai E14, rechanges compris) | ≈ 245 € |
| **Total avec la variante** | **≈ 1 195 €** |

Le poste 8 (bain thermostaté, thermomètre de référence, masse thermique) est un **investissement d'atelier** réutilisé pour toutes les phases.

---

## 1. Électronique du nœud ruche (compartiment TBTS du toit)

| Repère | Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|---|
| U1 | Carte ESP32 | ESP32-DevKitC-32E (module **WROOM-32E**, 4 Mo, antenne PCB), Espressif ou distributeur reconnu | 1 (+1 rechange) | 12 € |
| U2 | Convertisseur 12 V → 5 V | abaisseur synchrone 5 V / 1 A, entrée 6–36 V, type Pololu D24V10F5 ou Traco TSR 1-2450 | 1 | 9 € |
| U4 | Module µSD | adaptateur µSD SPI **3,3 V direct** (sans régulateur 5 V ni translateur), pull-up 10 kΩ sur CS | 1 | 4 € |
| — | Carte µSD industrielle | 4–8 Go **pSLC/SLC**, plage −25/+85 °C, type Swissbit S-45u ou Kingston Industrial | 1 | 18 € |
| U5 | Horloge RTC | module **DS3231** (TCXO ±2 ppm) avec pile CR2032 — **retirer la diode/résistance de charge** si le module en a une (pile non rechargeable) | 1 | 6 € |
| H1 | Capteur T/HR | **SHT45** sur petit circuit avec **membrane PTFE** (type Sensirion SHT45-AD1F ou module avec filtre), I²C 0x44 | 1 | 14 € |
| D1 | LED d'état | WS2812B, 1 pixel, sur circuit avec condensateur 100 nF | 1 | 1 € |
| S1 | Bouton départ / acquittement | poussoir IP67 Ø 16 mm, contact NO, monté en face avant du boîtier | 1 | 6 € |
| — | Platine | carte à pastilles (ou petit circuit KiCad en Phase 2), borniers à ressort pas 3,5 mm | 1 | 8 € |
| — | Passifs | résistances 1 % 0,25 W (100 Ω ×6, 1 kΩ ×8, 2,2 kΩ ×8, 4,7 kΩ ×6, 10 kΩ ×12, 47 kΩ ×1, 100 kΩ ×6, 1 MΩ ×2), condensateurs X7R (10 nF ×6, 100 nF ×15, 1 µF ×2, 4,7 µF ×2, 10 µF ×4) | lot | 8 € |
| — | Diodes | 1N4148 ×6 (pompe de charge, roue libre), 1N5819 ×1 (anti-inversion 12 V) | lot | 1 € |
| Q5 / Q4 | Commutation 12 V ventilateurs | NPN BC847/MMBT3904 + P-MOSFET **AO3401** (ou IRF9540N en traversant) | 1 + 1 | 1 € |
| Q6–Q7 | Étages PWM ventilateurs | NPN MMBT3904 collecteur ouvert : Q6 toit, Q7 **commun M2 + M3** (D21 : Q8 supprimé, GPIO 14 réaffecté au film) | 2 | 0,3 € |
| D10–D17 | Protections des lignes sondes | réseaux TVS basse capacité **ESD (type PESD3V3 ou TPD4E05)** sur les 4 bus 1-Wire et l'I²C sortant | 4 | 1 € |
| — | Boîtier TBTS | boîtier ABS/PC **IP65** env. 150 × 80 × 45 mm, couvercle vissé (ESP32, C4, RTC, µSD) | 1 | 10 € |
| — | **Boîtier 230 V** (séparé) | boîtier PC **IP65** env. 160 × 110 × 60 mm, couvercle vissé (F1, K1, SSR1, PS1, borniers) — **aucune TBTS dans ce boîtier** sauf les fils 12 V de PS1, de la bobine K1 et de la commande SSR, en câble à double isolation | 1 | 14 € |

## 2. Chaîne de sécurité matérielle (C4 électronique, C5 thermique)

| Repère | Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|---|
| RT1 | **NTC de sécurité C4** | 10 kΩ ±1 % à 25 °C, B25/85 ±0,75 %, **tête époxy ou verre Ø ≤ 3 mm**, fils isolés, type Vishay **NTCLE100E3103FB0** ou TDK B57861S0103F040 ; collée dans un fourreau laiton à la grille de soufflage | 2 (1 + 1 rechange) | 2 € |
| U6 | Comparateur double | **LM393** (ou LM2903, plage −40/+125 °C), alimenté en 12 V | 1 | 0,5 € |
| U7 | Référence de tension | **TL431** (2,495 V) — sert aux deux branches : seuil **ratiométrique**, la précision de la référence n'intervient pas | 1 | 0,3 € |
| R_seuil | Résistance de seuil 45,0 °C | **0,1 %, 25 ppm/K**, valeur = R(45,0 °C) de la table R/T de la NTC achetée (≈ **4,32 kΩ** pour B = 3977–3988) ; ajustée après étalonnage au bain (protocole E3) | 2 | 2 € |
| R_haut | Résistances de pont | 10,0 kΩ **0,1 %, 25 ppm/K** (×2 : côté NTC et côté seuil) | 3 | 1,5 € |
| R_test | Résistance « 46 °C simulés » | **4,12 kΩ 0,1 %** (≈ 46,2 °C) commutée par S2 | 1 | 1,5 € |
| R_fil | Seuil « NTC coupée » | pont 2,2 V sur la référence (≈ −15 °C) : 10 kΩ + 75 kΩ 1 % | 1 | 0,5 € |
| S2 | Bouton « TEST SÉCURITÉ » | poussoir IP67 **inverseur** (1 RT), capuchon jaune | 1 | 8 € |
| S3 | Bouton « RÉARMEMENT C4 » | poussoir IP67 NO, capuchon bleu, **protégé contre l'appui accidentel** (collerette) | 1 | 8 € |
| K1 | **Relais de sécurité série** | bobine **12 V DC**, **2 contacts NO/RT 8 A 250 V AC**, type Finder **40.52.9.012** + socle 95.05 + étrier | 1 | 12 € |
| K2 | Relais d'auto-maintien (TBTS) | bobine 12 V DC, 1 RT 6 A, type Finder **34.51.7.012** (relais fin). **Variante D21 : 2 RT 8 A**, type Finder **40.52.9.012** + socle 95.05 (second contact en série dans le 24 V du film, pouvoir de coupure DC1 ≥ 5 A sous 30 V **[H]** à vérifier sur la fiche) | 1 | 6 € (12 € en 2 RT) |
| Q1 / Q2 / Q3 | Pilotage K1/K2 et SSR | N-MOSFET **BS170** (Q1, bobines K1+K2, 500 mA) ; NPN BC547 (Q2 : ouverture par le MCU) ; NPN BC547 (Q3 : sortie de la pompe de charge vers le SSR) | 1+1+1 | 0,5 € |
| U3 | Opto retour d'état C4 | **PC817** (LED en parallèle sur la bobine K1 via 2,2 kΩ) | 1 | 0,3 € |
| F2 | **Bimétal C5 à réarmement manuel** | thermostat disque KSD301 **à réarmement manuel**, contact NF 10 A 250 V. Seuil définitif fixé au protocole E6 (T élément en régime normal + 10 °C) : acheter **un jeu 70 / 80 / 90 °C [H]** | 3 | 3 € |
| F3 | **Fusible thermique C5 (TCO)** | TCO axial non réarmable, **Tf = 102 °C [H]**, 10 A 250 V, type Aupo A4-F 102 °C / Thermodisc G4A ; serti (jamais soudé) | 5 | 1 € |

## 3. Puissance 230 V (compartiment 230 V du toit + prolongateur)

| Repère | Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|---|
| — | **Prolongateur de banc protégé** | coffret ou bloc **disjoncteur différentiel 30 mA type A** + disjoncteur 10 A courbe C, prise mâle 2P+T, prise de sortie IP44 — *ou* prise murale déjà protégée par DDR 30 mA **testé** | 1 | 45 € |
| — | Arrêt d'urgence de banc | coup-de-poing à accrochage, 2 NF 10 A, en boîtier, en série sur la phase du prolongateur | 1 | 18 € |
| F1 | Fusible d'entrée | porte-fusible 5 × 20 sur rail ou en ligne, fusible **2 A temporisé** (élément 1,1 A + alim) | 1 + 5 | 4 € |
| SSR1 | **Relais statique** | **passage par zéro**, 230 V AC, **10 A** (≥ 8 × 1,1 A), commande 3–32 V DC, type Crydom D2410 / Carlo Gavazzi RM1A23D25 ; **avec dissipateur** (≈ 1,2 W à 1,1 A), monté hors gaine | 1 | 25 € |
| — | Varistance | MOV 275 V AC (S14K275) aux bornes de sortie du SSR si absente du SSR | 1 | 1 € |
| PS1 | Alimentation 12 V | module encapsulé **230 V → 12 V 2,5 A**, type Mean Well **IRM-30-12** (classe II, −30/+70 °C avec déclassement) | 1 | 14 € |
| R1 | **Élément chauffant 250 W** | résistance **blindée à ailettes aluminium** 230 V **250 W**, longueur ≤ 200 mm, bornes isolées, **raccordée à la terre** (classe I), type « résistance de convection pour armoire / tube à ailettes » ; *variante* : tapis silicone 230 V 250 W collé sur radiateur aluminium à ailettes 150 × 100 mm [H] | 1 (+1 variante) | 30 € |
| — | Borniers | bornes à levier 3 × 2,5 mm² (type Wago 221) ; bornier de terre | 6 | 1 € |
| — | Câble interne 230 V | silicone **H05SS-F** 1 mm² (tenue 180 °C) près de l'élément ; **H05V2-K** 1 mm² (90 °C) ailleurs | 3 m | 2 €/m |

> L'architecture (§1.3) vise un élément **classe II**. Les éléments 250 W à ailettes métalliques disponibles sont classe I : en Phase 1 on **raccorde la terre** et on s'appuie sur le DDR 30 mA. La classe II reste l'objectif de série (Phase 6). [H]

## 4. Ventilation

| Repère | Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|---|
| M1 | **Soufflante du toit** | **radiale 12 V, 4 fils (PWM + tachymètre)**, roulement à billes, tenue ≥ 70 °C, **débit à l'air libre ≥ 30 m³/h**, format posé à plat ≤ 120 × 120 × 33 mm, type Delta BFB1012VH (97 × 97 × 33) ou BFB1212 (120 × 120 × 32). Voir **le point critique de débit** dans `module-toit-phase1.md` §2 | 1 (+1) | 22 € |
| M2, M3 | **Soufflantes du plancher** | **radiales 12 V 40 × 40 × 10 mm**, roulement à billes, **4 fils PWM + tachymètre** (à défaut 3 fils + tachymètre : PWM alors sans effet, régler par `pwm_plancher`), tenue ≥ 70 °C, type Sunon UB5U3 / Delta BFB0412 | 2 (+1) | 9 € |
| — | Grilles de protection 40 mm | grille inox ou imprimée, maille ≤ 2,5 mm (inaccessible aux abeilles) | 2 | 2 € |
| — | Grille de soufflage du toit | **tôle inox perforée**, trous Ø 2 mm, ≥ 40 % de vide, 0,5 mm, découpée à ≈ 380 × 100 mm (×2 : soufflage et aspiration) | 2 | 8 € |

## 5. Sondes et peignes

| Repère | Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|---|
| — | **DS18B20U** (boîtier µSOP-8) pour peignes | **Analog Devices/Maxim, acheté chez un distributeur agréé (Mouser, Farnell, Digi-Key)** — nombreuses contrefaçons ailleurs | 5 + 3 rechanges | 5 € |
| — | DS18B20 étanche pour le toit | DS18B20 (authentique, distributeur agréé) en **TO-92**, monté par vos soins dans un fourreau inox Ø 4 × 30 mm, câble silicone | 2 (T_air, T_retour) + 1 | 6 € |
| — | NTC élément (diagnostic) | NTC **100 kΩ, B 3950, verre**, tenue 250 °C, fils isolés fibre de verre | 2 | 2 € |
| — | **Circuit imprimé des peignes** | FR4 **1,0 mm**, 15 mm de large, P1 ≈ 320 mm (3 sondes), P2/P3 ≈ 200 mm (1 sonde), finition ENIG, vernis épargne (fichiers à produire, voir `peignes-sondes.md`) | 5 ex. (min. de commande) | 30 € le lot |
| — | Encapsulation | **époxy bi-composant coulable** de classe électronique (type MG Chemicals 832C) + **vernis de tropicalisation** silicone (type MG 422C) | 1 + 1 | 25 € |
| — | Gaine thermorétractable à colle | 3:1, Ø 6 et Ø 9 mm | 1 m chacune | 5 € |

## 6. Connectique et câbles

| Repère | Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|---|
| J1 | **Embase M8 femelle 4 broches** (P1, centre), codage A, IP67 | montage en façade, fils 0,25 mm², type Binder série 718 ou Phoenix SACC-DSI-M8FS-4CON | 1 | 9 € |
| J2, J3 | **Embases M8 femelles 3 broches** (P2, P3, bords), IP67 | même série, **3 broches** : une fiche 4 broches ne peut pas s'y insérer et inversement → **détrompage mécanique centre / bords** | 2 | 9 € |
| — | **Câbles M8 mâles surmoulés** | **PUR** 0,25 mm², **1,0 m**, connecteur droit IP67, extrémité libre soudée au peigne : 1 × **4 broches** (P1), 2 × **3 broches** (P2, P3) | 3 | 11 € |
| — | Capuchons d'obturation M8 | IP67, avec cordelette | 3 | 2 € |
| — | Bagues de repérage couleur | P1 = **rouge**, P2 = **jaune**, P3 = **vert** (embase + fiche) | 1 lot | 3 € |
| J4 | **Embase M12 8 broches** (plancher) | M12 code A, femelle, IP67, montage façade (sur le toit) | 1 | 12 € |
| — | Câble M12 8 broches mâle | surmoulé PUR 8 × 0,25 mm², **1,0 m** (côté plancher) | 1 | 15 € |
| — | Câble d'alimentation de la ruche | **H07RN-F 3G1,5**, 3 m, fiche 2P+T IP44 | 1 | 15 € |
| — | **Presse-étoupes** | polyamide **IP68**, M20 (câble 230 V, Ø 7–13), M16 (rechange), M12 (passage interne TBTS↔230 V), avec contre-écrous et joints | 2 + 2 + 2 | 2 € |
| — | Câble USB | USB-A ↔ micro-USB, 3 m, avec ferrite (console et flashage) | 1 | 6 € |

## 7. Mécanique du toit et du plancher

| Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|
| **Toit Nicotplast Dadant 10** | stock de l'apiculteur (D10) — prévoir un **2ᵉ toit** pour itérer sans détruire le premier | 2 | stock |
| **Plancher Nicot aéré** | stock de l'apiculteur | 1 | stock |
| **Plaque d'obturation du plancher** | **PVC expansé 3 mm** (type Forex) ou **polycarbonate 2 mm** ; découpée à la cote de la glissière (voir `plancher-phase1.md`) | 1 plaque 500 × 1000 | 15 € |
| Plaque de cloisonnement du toit | PVC expansé **5 mm** (fond du toit : bandes + plaque centrale) | 1 plaque 500 × 1000 | 20 € |
| **Isolant** | **XPS 20 mm** (polystyrène extrudé, tenue 75 °C) pour le dessus et les côtés ; *près de l'élément* : **laine de roche ou feutre céramique 6 mm** + tôle alu 0,5 mm (XPS interdit à moins de 30 mm de l'élément) | 1 panneau + 0,25 m² | 20 € |
| Gaine de chauffe | tôle **aluminium 0,8 mm** pliée (section ≈ 150 × 45 mm, longueur ≈ 230 mm) ou conduit imprimé **ASA** hors zone élément | 1 | 10 € |
| **Joints** | mousse **EPDM cellules fermées** adhésive 10 × 5 mm (périphérie toit/corps, plaque centrale/têtes de cadres) ; **silicone** 10 × 3 mm près de l'élément ; joint mousse 6 × 3 mm (plaque de plancher) | 10 m + 2 m + 3 m | 15 € |
| Visserie | inox A2, M3/M4, inserts laiton à chaud M3 pour pièces imprimées, colliers, entretoises nylon | lot | 10 € |
| Mastic | silicone neutre (étanchéité presse-étoupes / plaque) — **pas de silicone acétique** (corrosion) | 1 | 6 € |

## 8. Étalonnage et essais à vide

| Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|
| **Bain thermostaté** | **thermoplongeur sous-vide** de cuisine 800–1200 W, stabilité ±0,1 °C annoncée [H], dans une **glacière** 15–25 L (bain isolé, couvercle percé) | 1 | 90 € |
| **Thermomètre de référence** | thermomètre numérique à sonde Pt100 / thermistance, résolution 0,01 °C, **certificat d'étalonnage raccordé COFRAC (ou équivalent) couvrant 35–50 °C, incertitude ≤ 0,1 °C** | 1 | 180 € |
| Résistances étalons | 0,1 % : 4,32 kΩ (45,0 °C), 4,12 kΩ (46,2 °C), 4,53 kΩ (43,8 °C) — pour vérifier la chaîne C4 sans bain | 1 jeu | 6 € |
| **Masse thermique de banc** | **4 cadres bâtis vides** (ou cire gaufrée) au centre, entre lesquels vont les peignes + **8 à 10 bouteilles d'eau de 0,5–1 L** en rive, à la place des 6 autres cadres (≈ 8–10 kg d'eau, ≈ 35–45 kJ/K, soit ~2/3 de H4) [H] | 1 | 5 € |
| **Lampe témoin** | lampe **halogène 230 V 40–60 W** sur douille avec câble : charge visible pour tester SSR, K1, C5 **sans** l'élément | 1 | 8 € |
| Wattmètre-compteur | prise wattmètre 230 V (énergie kWh, puissance W) | 1 | 20 € |
| Multimètre | catégorie **CAT III 300 V** minimum, mesure de continuité, résistance, tension AC/DC | 1 | (atelier) |
| Contrôleur d'isolement | **500 V DC** (mégohmmètre) — à défaut, faire réaliser la mesure par un électricien | 1 | (atelier / prêt) |
| Thermomètre infrarouge | −20/+380 °C, émissivité réglable (contrôle de l'élément et des surfaces) | 1 | 25 € |
| Anémomètre à fil chaud | 0–20 m/s (estimation des débits aux fentes) **[optionnel]** | 1 | 60 € |
| **Capteur CO₂ de référence** (D16) | NDIR, plage **≥ 0–5 % vol (50 000 ppm)**, sortie I²C/UART, compensation en température, tenue ≥ 50 °C. **Attention : les capteurs courants type SCD41 plafonnent à ~0,5 % et saturent dans une ruche.** Utilisé seulement pour les **premiers essais avec abeilles** (inutile à vide) | 1 | 80–150 € [H] |

---

## 9. Imprimante 3D (pièces de prototypage)

Aucune imprimante n'est disponible (architecture §7). Elle n'est **pas indispensable** à la Phase 1 (le plancher est fermé par une plaque, le cloisonnement du toit est en PVC découpé) mais elle accélère beaucoup les itérations.

**Pièces visées (petites)** : supports de soufflantes 40 × 40 avec grille intégrée, cadre de fixation de la soufflante du toit, réglettes de cloisonnement à proportions variables (1/4–1/2–1/4 puis 1/5–3/5–1/5…), « peigne passe-câbles » posé sur le bord du corps, porte-sondes de la grille de soufflage (T_air + NTC C4 côte à côte), boîtiers des embases M8/M12, cales et gabarits de perçage, pinces de maintien des peignes. Aucune pièce ne dépasse ≈ 200 × 200 mm : **le plancher entier (≈ 500 × 400) n'est pas visé** (il faudrait 2–4 segments, inutile tant que la plaque suffit).

**Classe de machine** : imprimante FDM **fermée (caisson)**, volume **moyen ≈ 220–256 mm de côté**, plateau chauffant ≥ 100 °C, buse ≥ 260 °C (idéalement tout-métal), extrudeur direct, plateau PEI. Le caisson est nécessaire pour l'ASA (retrait, gauchissement, vapeurs). Prévoir une pièce ventilée ou un filtre (l'ASA émet du styrène).

**Matériaux** :

| Matériau | Usage | Pourquoi |
|---|---|---|
| **ASA** (recommandé) | toutes les pièces dans le toit et la ruche | Tenue en température élevée (ramollissement ≈ 95–100 °C), **résiste aux UV et à l'extérieur**, peu sensible à l'humidité ; marge confortable à 45 °C permanent et près de la gaine (≤ 60 °C). |
| **PETG** | pièces froides : supports de câbles, boîtiers d'embases, gabarits | Facile à imprimer, robuste, ramollissement ≈ 75–80 °C : suffisant à 45 °C, **pas** près de l'élément ; vieillit moins bien aux UV que l'ASA. |
| **PLA — exclu** | — | **Transition vitreuse ≈ 55–60 °C** : à 45 °C pendant des heures, sous charge, il **flue** (déformation lente) ; il devient mou au soleil dans un toit (> 60 °C) ; il se dégrade à l'humidité et à la chaleur. Inacceptable dans une ruche chauffée. |
| Rien d'imprimé | dans la gaine, à moins de **30 mm de l'élément** | Tôle aluminium, feutre céramique, silicone uniquement (C0 : aucun plastique imprimé au contact du point chaud). |

Remplissage ≥ 40 %, 4 périmètres, inserts laiton à chaud pour les vis. Les pièces au contact des abeilles (Phase 2+) : surfaces lisses, sans cavités (propolis), nettoyables.

---

## 10. Variante plancher chauffant d'appoint (D21) — option

> Nécessaire seulement pour l'essai comparatif E14 (`param plancher_chauffant 1`). Repères : `cablage-phase1.md` §1 et §3.7, `plancher-phase1.md` §7. **TBTS 24 V uniquement dans le plancher.**

| Repère | Désignation | Caractéristiques / référence type | Qté | Prix unit. [H] |
|---|---|---|---|---|
| Film A, B | **Films chauffants 24 V DC** | **polyimide** (Kapton) adhésif, ≈ **400 × 145 mm**, **30–40 W chacun** (≈ 14–19 Ω), tenue ≥ 150 °C, sorties câble silicone ; *option* : un film unique ≈ 400 × 300 mm **sur mesure** avec 2 réserves 50 × 50 mm pour les soufflantes (60–90 €) ; *à défaut* : tapis silicone 1,5 mm (vérifier la hauteur, `plancher-phase1.md` §7.2) | 2 (+1) | 25 € |
| PS2 | **Alimentation 24 V dédiée** | module encapsulé **230 V → 24 V ≈ 90 W (3,75 A)**, classe II, type Mean Well **IRM-90-24** ; *à défaut* bloc rail DIN 24 V 100 W (type Mean Well HDR-100-24) dans le boîtier 230 V | 1 | 30 € |
| F4 | Fusible d'entrée PS2 | porte-fusible 5 × 20 + fusible **1,6 A temporisé**, raccordé **en amont de F1, hors K1** | 1 + 3 | 4 € |
| F5 | Fusible de sortie 24 V | porte-fusible 5 × 20 + fusible **5 A rapide** | 1 + 3 | 4 € |
| Q9 | **N-MOSFET logique côté bas** | 30 V, Vgs(th) ≤ 2,5 V, Rds(on) ≤ 20 mΩ à 3,3 V **[H]**, TO-220, type **IRLB8721PbF** (à défaut : MOSFET spécifié à Vgs = 2,5 V) ; + 100 Ω série, 100 kΩ grille-source | 1 (+1) | 1,5 € |
| D20, D21 | Protections 24 V | diode Schottky **SS34** (roue libre sur le film) ; TVS **SMBJ28A** sur le rail 24 V | 1 + 1 | 1 € |
| K2 | Relais 2 RT (remplace le relais 1 RT du §2) | Finder **40.52.9.012** + socle — surcoût | 1 | +6 € |
| BM_A, BM_B | **Bimétaux NF 55 °C** | thermostat plat **KSD9700** (≈ 5 mm d'épaisseur), contact **NF**, 5 A, seuil **55 °C ±5 K [H]** ; acheter aussi 50 °C et 60 °C pour le réglage en E14 | 2 (+4) | 2 € |
| TCO_A, TCO_B | **Fusibles thermiques 72 °C** | TCO axial non réarmable **Tf ≈ 72 °C [H]**, ≥ 5 A, type Aupo A3-F 72 °C ; **serti**, jamais soudé | 2 (+4) | 1 € |
| T_film | **Sonde de surface du film** | DS18B20 **TO-92** authentique (distributeur agréé), couchée sur le film avec pâte thermique + ruban alu ; étalonnée au bain avec les autres (position `film`) | 1 (+1) | 6 € |
| J5 | **Embase M12 code T 4 broches** femelle (24 V DC) | IP67, 12 A / 63 V DC par contact, montage façade (sur le toit) | 1 | 20 € |
| — | Câble M12 code T mâle surmoulé | 4 × 0,75 mm² minimum (1,5 mm² si disponible), PUR, **1,0 m** (côté plancher) | 1 | 25 € |
| — | Capuchon M12-T | obturation de J5 quand le plancher n'a pas de film | 1 | 3 € |
| — | Presse-étoupe M12 supplémentaire | passage du câble J5 dans la plaque du plancher | 1 | 2 € |
| — | Plaque d'obturation **polycarbonate 2 mm** | support du film (le PVC expansé ne tient que ≈ 60 °C [H]) | 1 plaque 500 × 1000 | 15 € |
| — | Fixation thermique | pâte thermique silicone, ruban aluminium adhésif, ruban polyimide (Kapton) 25 mm | lot | 10 € |
| — | Câble silicone | 0,75 mm² rouge/noir (liaisons films ↔ bimétaux ↔ TCO ↔ plaquette de jonction) | 2 × 2 m | 2 €/m |
| — | Boîtier 230 V agrandi | ≈ 200 × 120 × 75 mm IP65 pour loger PS2 en plus de PS1 **[H]** (cote IRM-90 à vérifier) — surcoût | 1 | +8 € |
