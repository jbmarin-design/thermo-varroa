# Marche à suivre & prompts IA

> ⚠️ **Mise à jour** : les contraintes écrites dans les prompts ci-dessous datent du cadrage initial. Plusieurs ont changé depuis (chauffe par **module de toit** et non par plancher, consigne **42 °C / 2 h** au point le plus froid au lieu de 40–41 °C / 2–3 h, **couveuse à reines séparée**, **peignes de sondes connectables**, coupure matérielle **45 °C**, débit d'air ~80–120 m³/h). **`docs/architecture.md` fait foi** : joignez-le à chaque prompt.
>
> Phases 0 et 1 : réalisées (voir l'état d'avancement dans le `README.md`).

## Méthode de travail

1. Une branche Git par phase : `git checkout -b phase-N-nom`
2. Coller le prompt de la phase dans Claude (en écrit), en joignant le README, l'architecture validée et les résultats de la phase précédente
3. Committer les livrables, **tester**, consigner les résultats dans `docs/journal-tests.md`
4. Validé → merge dans `main`. Non validé → renvoyer les résultats de test à Claude pour correction

> ⚠️ Règle d'or : aucun test avec abeilles avant que la Phase 2 (sécurités) soit validée à vide.

---

## Prompt 0 — Cadrage & architecture

```
Tu es ingénieur système embarqué + apiculture de précision. Je conçois un système de thermothérapie varroa instrumenté pour ruches Nicot (plastique, fond grillagé).

Contraintes :
- Palier couvain 40–41 °C à cœur, 2–3 h, ventilation douce pour homogénéité
- Protection reine séparée : micro-cage 38 °C, 24 h
- 100 ruches, traitement en LOTS DE 20, SUR SITE (rucher isolé)
- "Pose et repars" : automate local souverain (sécurités/paliers/arrêt), télésurveillance en lecture seule
- Logging local + export serveur
- Énergie site : séquencement sous-lots ; groupe électrogène OU batterie+onduleur+solaire

Livrable de CE prompt UNIQUEMENT (ne code rien) :
1. Architecture matérielle en blocs
2. Architecture logicielle (modules + machine à états)
3. Choix techno justifiés (MCU, capteurs, actionneurs, comms)
4. Arborescence du dépôt Git
Rédige le tout dans docs/architecture.md. On valide l'architecture avant de coder.
```

## Prompt 1 — Phase 1 : prototype mono-ruche

```
Architecture validée (jointe). PHASE 1 : prototype sur UNE ruche Nicot.

Livrables :
1. Nomenclature (hardware/bom-phase1.md) : ESP32, 3 sondes température (couvain haut / bas / air), 1 sonde hygrométrie type SHT, élément chauffant dimensionné, relais statique, ventilateur, alimentation
2. Schéma de câblage (hardware/cablage-phase1.md)
3. Firmware PlatformIO (firmware/) : lecture capteurs, commande chauffe tout-ou-rien avec hystérésis, logging série + carte SD
4. Protocole de test À VIDE (tests/protocole-phase1.md) : montée en température, homogénéité, inertie
Pas encore de régulation fine ni de sécurités avancées.
```

## Prompt 2 — Phase 2 : régulation & sécurités

```
Résultats Phase 1 joints. PHASE 2 : régulation et sécurités.

Livrables :
1. Régulation PID (ou PI) avec anti-windup, rampe de montée contrôlée vers 40–41 °C
2. Machine à états : ATTENTE → MONTÉE → PALIER (durée paramétrable) → REFROIDISSEMENT → FIN, + état DÉFAUT
3. Sécurités indépendantes : coupure matérielle (thermostat bimétal) au-delà de 43 °C, watchdog, détection sonde HS / incohérente, arrêt si palier non atteint
4. Fichier de paramètres (consignes, durées, seuils)
5. Tests unitaires de la machine à états (tests/)
6. Protocole de validation à vide incluant la simulation de chaque défaut
```

## Prompt 3 — Phase 3 : module couveuse-reine

```
Phases 1–2 validées. PHASE 3 : protection de la reine.

Livrables :
1. Conception d'une micro-cage couveuse régulée à 38 °C pendant 24 h, autonome du plateau couvain
2. Nomenclature et câblage (hardware/)
3. Extension firmware : second canal de régulation, séquence couvain + reine coordonnée
4. Procédure terrain : capture, mise en cage, réintroduction de la reine
```

## Prompt 4 — Phase 4 : télésurveillance

```
PHASE 4 : télésurveillance en LECTURE SEULE (l'automate reste souverain en local).

Livrables :
1. Format de trame / données (docs/format-donnees.md)
2. Envoi périodique depuis l'ESP32 + tampon local si perte de liaison
3. Code serveur minimal (réception + stockage) dans server/
4. Tableau de bord temps réel (courbes par ruche, hygrométrie, état reine) + alertes hors-plage
5. Stratégie comms selon couverture : 4G / LoRa / satellite
Documente le format de données pour exploitation scientifique.
```

## Prompt 5 — Phase 5 : montée en échelle (lots de 20)

```
PHASE 5 : passage à 20 ruches simultanées sur site.

Livrables :
1. Architecture contrôleur central + 20 plateaux (1 sonde + 1 relais par ruche, asservissement individuel)
2. Séquencement par sous-lots de 4–5 ruches pour lisser la puissance de pointe
3. Bilan de puissance et dimensionnement énergie : groupe 5–6 kVA vs batterie lithium + onduleur + solaire (comparatif coût / autonomie)
4. Mode "pose et repars" complet : démarrage, paliers, arrêt sécurité automatiques
5. Nomenclature lot de 20 et procédure de mise en place sur rucher
```

## Prompt 6 — Phase 6 : enveloppe thermique & industrialisation

```
PHASE 6 : efficacité énergétique et reproductibilité.

Livrables :
1. Capot isolé intégrant le boîtier de contrôle (remplace le toit), joint périphérique
2. Portière d'entrée ajustée avec aérations fines (évacuation humidité)
3. Isolation du corps (panneau mousse) et gain de puissance estimé
4. Nomenclature figée + documentation de fabrication reproductible
```

## Prompt 7 — Phase 7 : validation terrain

```
PHASE 7 : validation scientifique sur le terrain.

Livrables :
1. Protocole d'essai : ruches traitées vs témoins, comptage varroa avant/après (lange graissé, lavage alcool ou sucre glace)
2. Indicateurs : efficacité, mortalité couvain, état de la reine, consommation énergétique
3. Gabarit de rapport d'essai et scripts d'analyse des données (data/)
4. Calendrier de traitement recommandé (période avec couvain vs hors couvain)
```

---

## Checklist de validation par phase

- [ ] Livrables présents dans le dépôt
- [ ] Tests exécutés et consignés dans `docs/journal-tests.md`
- [ ] Sécurités vérifiées (à partir de la Phase 2)
- [ ] Merge dans `main`
