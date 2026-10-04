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

## Arborescence

```
firmware/   Code embarqué (ESP32)
hardware/   Schémas, nomenclature, câblage, plans mécaniques
docs/       Architecture, procédures, journal de tests
server/     Réception et stockage des données de télésurveillance
data/       Jeux de données de traitement (non versionnés en CSV)
tests/      Tests unitaires et protocoles de test à vide
```

## Méthode

Voir [`MARCHE_A_SUIVRE.md`](MARCHE_A_SUIVRE.md) : une branche par phase, un prompt IA par livrable, validation par test avant merge dans `main`.

> ⚠️ Aucun essai avec abeilles avant validation à vide de la Phase 2 (sécurités).
