# CLAUDE.md — FM80S, un synthé-looper pour le M-VAVE FM-1

## Ce qu'on construit

Un firmware pour le synthé de poche M-VAVE FM-1 : plusieurs moteurs de synthèse (Drums, VA, Supersaw, FM d'abord), un looper de notes à 8 pistes, un séquenceur pas à pas, des scènes enchaînées en song, un arpégiateur et des effets punch-in. Vision complète : `docs/03_PRD.md`, à lire avant toute tâche.

- **D'abord dans le navigateur :** le même code C que celui de l'appareil, compilé en WebAssembly, joué dans un AudioWorklet, sur un FM-1 virtuel.
- **Ensuite sur l'appareil :** un seul fichier de plateforme (`plat_fm1.c`) au-dessus de la couche matérielle de Felucca. Anthony reçoit son FM-1 la semaine du 2026-10-12 : le premier son sur l'appareil passe avant l'écriture des moteurs suivants.

La plateforme et le DSP viennent de Trawler (`../FM1`, dépôt `y6zkcqnf5r-hue/fm1-trawler`, archivé), le tracker arrêté le 2026-10-10.

Documents à lire avant toute tâche, dans cet ordre :

1. `docs/03_PRD.md` — vision, périmètre, critères de succès
2. `docs/04_SPEC.md` — contrôles, interface de moteur, moteur Drums, esquisse de la boucle, budget mémoire
3. `docs/05_ARCHITECTURE.md` — dossiers, ce qu'on reprend de Trawler, build, tests, CPU
4. `docs/06_BACKLOG.md` — l'ordre des stories (on prend la prochaine non faite)
5. `docs/08_DECISIONS.md` — décisions et questions ouvertes
6. `../FM1/docs/01_FM1_HARDWARE.md` — contraintes de la puce, à relire avant tout code DSP

## Règles non négociables (reprises de Trawler)

1. **Entiers seulement dans `firmware/src/`.** Pas de `float` ni de `double` : point fixe (Q15 niveaux, Q12 gains, phase 32 bits). Les constantes se calculent sur l'ordinateur (`tools/gen_tables.py`).
2. **Pas de division 64 bits.** Multiplier en `int64_t`, oui ; `/` ou `%` sur 64 bits, jamais.
3. **Pas de libc, sauf ce qu'on fournit.** Pas de `malloc`, de `printf` ni de `math.h` dans `firmware/src/`. Mémoire statique.
4. **Budgets du FM-1 :** RAM ≤ 96 Kio ; pool ≤ 336 Kio (`APP_POOL`) ; code ≤ ~560 Kio.
5. **Audio : callback de 256 frames à 44,1 kHz**, rien de bloquant dedans ; enveloppes, LFO et coefficients par blocs de 16 ou 32 frames.
6. **Le code de l'app ne parle au matériel que par `firmware/src/app/plat.h`.** Aucun `#ifdef WEB` dans le code commun.
7. **Wasm et natif rendent le même son, échantillon par échantillon** (test de parité).
8. **GPL-3.0-only**, en-tête SPDX dans chaque fichier ; le code repris garde son en-tête et se note dans `LICENSING.md`.
9. **Ni marque, ni graphismes, ni noms de Teenage Engineering, d'Arturia ou de M-VAVE dans le produit.** « Façon Juno » pour décrire un son dans la doc, jamais dans l'interface.

## Façon de travailler

- **Une story à la fois, et chaque story se termine par quelque chose qu'Anthony essaie dans le navigateur (puis sur l'appareil).** On attend son avis avant la suivante, même s'il a dit « décide toi-même » : c'est la leçon de Trawler.
- Mode plan d'abord, avec les points à trancher et une option recommandée pour chacun ; puis le code, `make test` au vert, un commit qui cite l'ID de la story.
- Avant tout nouveau moteur DSP, un test qui rend un WAV de référence et vérifie son hash (golden).
- Langue : documentation et commits en français ; identifiants et commentaires de code en anglais.
