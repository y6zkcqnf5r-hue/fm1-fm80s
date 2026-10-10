# CLAUDE.md — FM80S, un synthé-looper pour le M-VAVE FM-1

## Ce qu'on construit

FM80S est un **fork de SLOOP 2.5** ([isod89/sloop-fm1](https://github.com/isod89/sloop-fm1), GPL-3.0, sur Felucca), un looper-groovebox qui marche déjà sur le FM-1. On lui ajoute, dans l'ordre :
1. **un FM-1 virtuel dans le navigateur** : le vrai firmware en wasm ;
2. **8 pistes au lieu de 4** : 7 synthés et la batterie ;
3. **une identité synthwave forte** : le nom, une image de démarrage, un thème, des sons, des écrans vivants ;
4. puis le reste du PRD, après qu'Anthony a joué SLOOP en main.

Décidé le 2026-10-10 (`docs/09_COMPARAISON.md`, ADR-036). La plateforme reprise de Trawler (P0-01) est gardée sous le tag `plateforme-trawler`. Anthony reçoit son FM-1 la semaine du 2026-10-12.

> **Transition :** tant que F0-01 n'est pas faite, le dépôt contient encore l'ancienne plateforme et SLOOP est cloné en lecture seule dans `vendor-ref/sloop-fm1`. F0-01 fait repartir main de l'historique de SLOOP (ADR-037).

Documents à lire avant toute tâche, dans cet ordre :

1. `docs/03_PRD.md` — vision, périmètre, ce que SLOOP apporte et ce que FM80S ajoute (§2 bis)
2. `docs/04_SPEC.md` — ce que FM80S change dans SLOOP
3. `docs/05_ARCHITECTURE.md` — l'arbre de SLOOP, l'amont, le build, l'émulateur
4. `docs/06_BACKLOG.md` — l'ordre des stories (on prend la prochaine non faite)
5. `docs/08_DECISIONS.md` — décisions et questions ouvertes
6. `docs/10_FAISABILITE_8_PISTES.md` — la mémoire, les projets, l'interface à 8 pistes
7. Les docs de SLOOP : `BUILDING.md`, `GUIDE.md`, `SLOOP.md`, et les commentaires en tête de `tests/run_tests.sh` et `tests/regress.c`
8. `../FM1/docs/01_FM1_HARDWARE.md` — contraintes de la puce, à relire avant tout code DSP

## Règles non négociables

1. **On suit l'arbre de SLOOP, sans le réécrire** (ADR-043). On change le moins de lignes possible dans ses fichiers ; ce qui est à nous va dans des fichiers à nous quand c'est possible ; nos changements dans ses fichiers portent un commentaire `/* FM80S: ... */`. But : pouvoir fusionner les nouvelles versions de SLOOP (remote `sloop`).
2. **Budgets du FM-1** (`firmware/app.ld`) : RAM ≤ 96 Kio ; pool ≤ 336 Kio, déjà presque plein (`tools/build.py` refuse le build sous 8 Kio libres) ; code ≤ ~568 Kio, serré. Toute mémoire en plus se chiffre avant d'être écrite.
3. **Audio : callback de 256 frames à 44,1 kHz** (5,8 ms), rien de bloquant dedans : ni flash, ni écran, ni attente.
4. **Pas de `malloc` ni de libc hors `firmware/src/libc.c`.** Mémoire statique ; gros tampons dans le pool.
5. **Le code qu'on écrit est en entiers** (point fixe), comme le DSP de SLOOP. Les `float` du code repris par SLOOP (`fm6_core.c`, `phys_dsp.c`) restent tels quels. Pas de division 64 bits.
6. **Les golden de SLOOP ne changent que sur un changement de son voulu**, écouté par Anthony : `GOLDEN_UPDATE=1`, et le diff de `tests/golden.txt` relu dans le commit. Même chose pour le budget CPU (`BUDGET_UPDATE=1`).
7. **Wasm et natif rendent le même son** dès F0-03 (rendus de `tests/golden.txt`, `-ffp-contract=off`).
8. **GPL-3.0-only**, en-tête SPDX dans chaque fichier ; le code repris garde son en-tête et ses crédits, et se note dans `LICENSING.md`.
9. **Noms :** le produit s'appelle FM80S. Ni « SLOOP », ni « Felucca », ni « M-VAVE » comme nom de produit ; le crédit « d'après SLOOP et Felucca » reste (ADR-044). Ni marque, ni graphismes, ni noms de Teenage Engineering ou d'Arturia (« façon Juno » dans la doc, jamais dans l'interface).

## Commandes

```sh
./build.sh                # le firmware (Docker Desktop sur Mac) : build/felucca.fwsc
tests/run_tests.sh        # les tests sur l'ordinateur (après ./build.sh)
GOLDEN_UPDATE=1 sh tests/run_tests.sh   # accepter un changement de son voulu
```

Les commandes de l'émulateur arrivent avec F0-02. Avant F0-01, l'ancien `Makefile` (`make lint test web`) ne concerne que la plateforme abandonnée.

## Façon de travailler

- **Une story à la fois, et chaque story se termine par quelque chose qu'Anthony essaie** : sur le FM-1 (un fichier à installer depuis Chrome, avec le pas à pas), et dans le navigateur dès F0-02. On attend son avis avant la suivante, même s'il a dit « décide toi-même » : c'est la leçon de Trawler.
- Mode plan d'abord, avec les points à trancher et une option recommandée pour chacun ; puis le code, les tests au vert, un commit qui cite l'ID de la story (ex. `F0-01: image de démarrage`).
- Toute opération qui réécrit l'historique (push forcé de main en F0-01) se fait après l'accord explicite d'Anthony.
- Langue : documentation et commits en français ; identifiants et commentaires de code en anglais.
