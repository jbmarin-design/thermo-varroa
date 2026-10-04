# Thermothérapie Varroa — Système instrumenté

Traitement thermique anti-varroa pour ruches Nicot (plastique, fond grillagé) — Les Ruchers de Gascogne.

## Cahier des charges

| Paramètre | Valeur |
|---|---|
| Palier couvain | 40–41 °C à cœur, 2–3 h, ventilation douce |
| Protection reine | Micro-cage à 38 °C pendant 24 h |
| Cible | Varroas phorétiques + varroas en cellules operculées |
| Organisation | 100 ruches, traitées par lots de 20, sur site (rucher isolé) |
| Mode | « Pose et repars » : automate local souverain, télésurveillance en lecture seule |
| Énergie | Séquencement par sous-lots de 4–5 ruches ; groupe 5–6 kVA ou batterie + onduleur + solaire |
| Électronique | ESP32, sondes température / hygrométrie (type SHT), relais, logging local |

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
