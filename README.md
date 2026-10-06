# Thermothérapie Varroa — Système instrumenté

Traitement thermique anti-varroa pour ruches Nicot (plastique, fond grillagé) — Les Ruchers de Gascogne.

## Cahier des charges

| Paramètre | Valeur |
|---|---|
| Palier couvain | 42,0 °C au point de couvain le plus froid pendant 2 h, rampe ~20 min (paramétrable 41,0–43,5 °C) |
| Architecture | Module de toit (électronique + chauffe + soufflante) + plancher fermé imprimé 3D, boucle d'air forcée |
| Sondes | Peignes connectables indépendants du toit, 5 points couvain en prototype ; sondes air, élément, sécurité et SHT45 intégrées au toit |
| Protection reine | Couveuse séparée à 38 °C (traitement + 24 h), cages de reine Nicot standard, Peltier réversible |
| Trou de vol | Fermé, porte Nicot à petites aérations |
| Sécurité | Régulation sur la sonde la plus froide, coupures sur la plus chaude ; coupure matérielle indépendante à 45,0 °C |
| Cible | Varroas phorétiques + varroas en cellules operculées |
| Organisation | 100 ruches, traitées par lots de 20, sur site (rucher isolé) |
| Mode | « Pose et repars » : automate local souverain, télésurveillance en lecture seule |
| Énergie | Séquencement par sous-lots de 4–5 ruches ; groupe 5–6 kVA (+ tampon batterie pour l'électronique et la couveuse) |
| Électronique | ESP32 par ruche, bus RS-485 vers un contrôleur de lot, logging local |

## État d'avancement

| Phase | Contenu | État |
|---|---|---|
| 0 — Architecture | [`docs/architecture.md`](docs/architecture.md) : décisions validées, boucle d'air, sondes, sécurités, bilan de puissance | ✅ Validée |
| 1 — Prototype mono-ruche | Dossier matériel, firmware, protocole de test à vide (voir ci-dessous) | 🟡 Conception terminée — **en attente : cotes réelles + achat du matériel + montage + essais** |
| 2 — Régulation & sécurités | PID, validation de chaque défaut à vide | ⏳ À faire (après les essais Phase 1) |
| 3 — Couveuse à reines | Boîtier 38 °C, cages Nicot, Peltier | ⏳ |
| 4 — Télésurveillance | Passerelle de lot, serveur, tableau de bord | ⏳ |
| 5 — Lot de 20 | Contrôleur de lot, séquencement, coffret | ⏳ |
| 6 — Industrialisation | Isolation, nomenclature figée | ⏳ |
| 7 — Validation terrain | Essais avec témoins, comptages varroa | ⏳ |

### Phase 1 — où trouver quoi

| Fichier | Contenu |
|---|---|
| [`hardware/bom-phase1.md`](hardware/bom-phase1.md) | Nomenclature : tout le matériel à acheter (+ conseil imprimante 3D) |
| [`hardware/module-toit-phase1.md`](hardware/module-toit-phase1.md) | Aménagement du toit Nicot, cloisonnement, **cotes à relever** |
| [`hardware/plancher-phase1.md`](hardware/plancher-phase1.md) | Fermeture du plancher Nicot, soufflantes dans 16 mm, **cotes à relever** |
| [`hardware/peignes-sondes.md`](hardware/peignes-sondes.md) | Peignes de sondes, connecteurs M8, étalonnage à 42 °C |
| [`hardware/cablage-phase1.md`](hardware/cablage-phase1.md) | Câblage 230 V / 12 V, chaîne de sécurité, brochage ESP32 |
| [`firmware/`](firmware/) | Code ESP32 + 99 tests unitaires (voir [`firmware/README.md`](firmware/README.md)) |
| [`tests/protocole-phase1.md`](tests/protocole-phase1.md) | Protocole de test à vide, essais E0 à E13 |
| [`docs/journal-tests.md`](docs/journal-tests.md) | Résultats des essais (à remplir) |

### Prochaines actions (apiculteur)

1. Relever les cotes du toit, du corps et du plancher listées dans les fichiers hardware, avec photos.
2. Commander le matériel de `bom-phase1.md`.
3. Monter le prototype, puis dérouler `tests/protocole-phase1.md` — **sans abeilles**.

### Points de vigilance connus

- **Débit d'air** : il faut ~80–120 m³/h en recirculation (et non 5–15 m³/h, estimation initiale corrigée). Premier point à mesurer au banc (essais E7–E8).
- **Firmware** : tests unitaires verts sur PC ; **compilation ESP32 jamais vérifiée** (PlatformIO indisponible dans l'environnement de développement). Voir `firmware/README.md`.

## Arborescence

```
firmware/   Code embarqué (ESP32) + tests unitaires
hardware/   Nomenclature, plans d'aménagement, câblage
docs/       Architecture, journal de tests
server/     Réception et stockage des données (Phase 4)
data/       Jeux de données de traitement (non versionnés en CSV)
tests/      Protocoles de test physiques (banc)
```

## Méthode

Tout le travail validé est sur la branche **`main`**. Voir [`MARCHE_A_SUIVRE.md`](MARCHE_A_SUIVRE.md) pour le découpage en phases et les prompts. En cas d'écart, **`docs/architecture.md` fait foi** : il intègre les décisions prises après la rédaction des prompts.

> ⚠️ Aucun essai avec abeilles avant validation à vide de la Phase 2 (sécurités).
