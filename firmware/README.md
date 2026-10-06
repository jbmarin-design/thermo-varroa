# Firmware — nœud ruche (Phase 1)

Projet PlatformIO. Logique **portable** dans `lib/` (aucun en-tête Arduino, testée sur PC) ; accès matériel et boucle principale dans `src/noeud/` (ESP32 uniquement).

| Dossier | Contenu |
|---|---|
| `include/brochage.h` | GPIO (copie dans `hardware/cablage-phase1.md` §4 — modifier les deux) |
| `include/parametres_defaut.h` | valeurs par défaut et **bornes figées** (seuils de sécurité non modifiables sans recompiler) |
| `lib/capteurs` | DS18B20/SHT45/NTC, filtre médian, validation, détection de peigne absent, positions |
| `lib/regulation` | tout-ou-rien avec hystérésis sur la sonde couvain la plus froide, limites 43,5 / 44,0 °C, plafond de puissance ; `CommandeFilm` (variante plancher chauffant D21 : même demande, limite 50 °C, interdit sans soufflantes de plancher) |
| `lib/securite` | supervision C2 : défauts verrouillés, SSR collé, ventilateurs, durée max, film du plancher (55 °C / 10 s, sonde du film) |
| `lib/machine_etats` | ATTENTE → AUTOTEST → MONTÉE → PALIER → REFROIDISSEMENT → FIN, + DÉFAUT |
| `lib/journal` | lignes CSV (mesures `M`, événements `E`, métadonnées `#`) |
| `lib/ihm` | bouton (court / long), couleurs de LED |
| `lib/hal/enable_ssr.h` | logique de l'enable dynamique du SSR (C3) |
| `src/noeud/` | `hal_esp32.cpp` (GPIO, 1-Wire, I²C, µSD, NVS, timers), `main.cpp` (boucle, console) |
| `test/` | tests Unity (env `native`) |

## Installer

```bash
pip install platformio            # ou --break-system-packages selon la distribution
# ou : extension PlatformIO IDE de VS Code
```

## Lancer les tests unitaires (PC)

```bash
cd firmware
pio test -e native
```

Trois suites : `test_machine_etats` (44 tests), `test_securite` (43), `test_journal_ihm` (12), soit **99 tests** (dont 16 pour la variante plancher chauffant D21). Un compilateur C++ (gcc ou clang) doit être installé sur le PC.

## Compiler et flasher l'ESP32

```bash
cd firmware
pio run -e esp32                    # compilation
pio run -e esp32 -t upload          # flashage (USB ; maintenir BOOT si la carte ne passe pas seule en téléchargement)
pio device monitor -e esp32         # console 115 200 bauds
```

Carte : ESP32-DevKitC-32E (WROOM-32E). **Pas de module WROVER** (GPIO 16/17 occupés par la PSRAM).
⚠️ Flashage avec le 230 V branché : uniquement via un **isolateur USB** ou un ordinateur sur batterie (`hardware/cablage-phase1.md`, avertissement 8).

## Première mise en service

1. `rtc 2026-10-04T14:30:00` (heure locale) — le départ est refusé tant que l'horloge est invalide.
2. Au premier démarrage, les paramètres NVS sont absents : le firmware charge les défauts et **refuse le départ** ; les valider en réécrivant un paramètre, ex. `param consigne 42.3`.
   **Mise à jour depuis 0.1.0** : le schéma des paramètres passe en v3 (ajout de `plancher_chauffant`) ; les paramètres enregistrés sont rejetés au premier démarrage (même procédure : les réécrire et vérifier `param`). La **table d'étalonnage est conservée** (version distincte).
3. Étalonner et déclarer chaque sonde (`hardware/peignes-sondes.md` §5) : `etal <ROM> <offset> <position>`.
4. Armer la chaîne C4 (bouton S3), puis `depart` (ou appui court sur S1).

## Console série (locale uniquement — aucune commande à distance)

| Commande | Effet |
|---|---|
| `aide`, `etat`, `sondes` | aide ; état, défauts, avertissements ; détail des embases et sondes (ROM, valeur brute, offset, position) |
| `etal <ROM> <offset> [pos]` / `etalref <ROM> <T_ref>` / `pos <ROM> <pos>` / `etal_effacer` | étalonnage (ATTENTE seulement ; offset refusé au-delà de ±1,0 °C). Positions : `P1_haut P1_centre P1_bas P2 P3 air retour film` (`film` = sonde de surface du film du plancher, sur le bus du toit via J4) |
| `param` / `param <nom> <valeur>` | lecture / écriture (ATTENTE seulement, bornées) : `consigne`, `hyst_bas`, `hyst_haut`, `palier_min`, `timeout_montee_min`, `pwm_toit`, `pwm_plancher` (0 = soufflantes de plancher désactivées pour l'essai E12), `rpm_toit`, `rpm_plancher`, `puissance_max`, `plancher_chauffant` (0/1, défaut 0 : film 24 V du plancher, variante D21, essai E14) |
| `depart`, `arret`, `acquit` | équivalents du bouton S1 (court en ATTENTE, long en cycle, court en FIN/DÉFAUT) |
| `test_trip` | le MCU ouvre la chaîne C4 (réarmement manuel S3) |
| `test_gel` | fige la boucle : vérifie l'enable dynamique (SSR ouvert ≤ 3,5 s) puis le watchdog (reset ≈ 5 s) |
| `rtc AAAA-MM-JJTHH:MM:SS`, `silence`, `bavard` | horloge ; coupe / rétablit l'écho série du journal |

## Journal

Un fichier par cycle sur la µSD (`/tv_AAAAMMJJ_HHMMSS.csv`), recopié sur la série. Lignes `#` : version, paramètres figés (dont `plancher_chauffant`), limites (dont `# film: max_reg=50.0 hyst=2.0 defaut=55.0/10s bimetal=55 tco=72`), table d'étalonnage. Lignes `M` : mesures toutes les 10 s en cycle (60 s sinon), séparateur `;`, décimales avec un point, champ vide = mesure invalide. Lignes `E` : transitions, défauts, avertissements, résumé de cycle (`RESUME`, avec `tmax_film`).

Colonnes de la variante plancher chauffant (D21), **ajoutées en fin de ligne** (les colonnes existantes ne bougent pas) :

| Colonne | Contenu |
|---|---|
| `t_film` | T surface du film (°C, filtrée) ; vide si la sonde est absente ou invalide |
| `film` | commande du film appliquée (0/1) — GPIO 14, MOSFET Q9 |
| `raisons_film` | pourquoi le film est coupé (bits) : 1 = variante désactivée, 2 = pas de demande (consigne atteinte ou limite couvain/air), 4 = limite 50 °C, 8 = soufflantes de plancher arrêtées/lentes, 16 = sonde du film invalide ; 0 = film commandé |

Nouveaux codes : défauts `film_surtemp` (0x80000) et `sonde_film` (0x100000) ; auto-test `AT_SONDE_FILM` (0x2000) et `AT_FILM_SANS_SOUFFLANTES` (0x4000).

## Ce qui a été vérifié / ce qui ne l'a pas été (au 2026-10-04)

| Point | État |
|---|---|
| Tests unitaires de `lib/` (99 tests au 2026-10-06, dont 16 pour la variante plancher chauffant, avec AddressSanitizer et UBSan, `-Werror`) | **vérifié**, mais avec g++ 13 + Unity compilés **manuellement**, pas via `pio test` : PlatformIO n'a pas pu être installé dans l'environnement de développement (accès à pypi.org et au registre PlatformIO refusés par le proxy) |
| `pio test -e native` | **non exécuté** (même raison) — à lancer en premier sur le PC de l'atelier |
| `pio run -e esp32` (compilation ESP32) | **non vérifié** (toolchain non installable). `src/noeud/*.cpp` a seulement passé une **vérification syntaxique** avec des en-têtes Arduino simulés (cœur Arduino-ESP32 2.x). Points à surveiller à la première compilation : signatures `ledcAttach/ledcWrite`, `esp_task_wdt_*` et `rgbLedWrite` si la plateforme installe le cœur 3.x |
| Fonctionnement sur carte réelle (1-Wire, SHT45, DS3231, µSD, PWM 25 kHz, tachymètres, enable dynamique) | **non vérifié** — protocole `tests/protocole-phase1.md` E2 à E4 |
| Valeurs [H] (`RPM_NOMINAL_*`, `PWM_*`, `PUISSANCE_MAX_PCT`, hystérésis, délais SSR collé) | à recaler au banc (E2, E5, E8) |

Variante plancher chauffant (D21) : brochage modifié — **PWM commun** des soufflantes de plancher sur GPIO 27 (M2 + M3 reliés sur la plaquette du plancher), **GPIO 14 = commande du film** (ex-PWM M3). Avec un montage encore câblé selon l'ancien brochage (Q8 sur GPIO 14), M3 tourne à 100 % tant que la variante est désactivée (GPIO 14 BAS) ; **ne pas activer `plancher_chauffant` avant recâblage** (`hardware/cablage-phase1.md` §3.4 et §3.7).

Non implémenté en Phase 1 (prévu en Phase 2) : PID / cascade et rampe de consigne, FRAM et reprise après coupure, watchdog externe, cohérence entre sondes bloquante (avertissement seulement), sonde figée, pente.
