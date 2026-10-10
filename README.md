# FM80S — un synthé-looper ludique pour le M-VAVE FM-1

Un firmware libre (GPL-3.0) qui fait du FM-1, le petit synthé de poche de M-VAVE, un **synthé-looper** : on joue une boucle au clavier, elle tourne, on en joue une autre avec un autre instrument par-dessus, et ainsi de suite ; ou on la programme pas à pas. Les boucles s'enchaînent en une song. Une dizaine de moteurs de synthèse, chacun avec son écran vivant, une orientation synthwave, un arpégiateur et des effets à déclencher en direct.

**Nouveau départ (2026-10-10) : FM80S devient un fork de [SLOOP](https://github.com/isod89/sloop-fm1)** (isod89, GPL-3.0, d'après [Felucca](https://github.com/hugelton/Felucca)), avec **8 pistes au lieu de 4**, un FM-1 virtuel dans le navigateur et une identité synthwave. Pourquoi : [`docs/09_COMPARAISON.md`](docs/09_COMPARAISON.md).

## Où en est-on

- **La vision, la spec, l'architecture et le backlog : réécrits pour le fork** : [`docs/03_PRD.md`](docs/03_PRD.md), [`docs/04_SPEC.md`](docs/04_SPEC.md), [`docs/05_ARCHITECTURE.md`](docs/05_ARCHITECTURE.md), [`docs/06_BACKLOG.md`](docs/06_BACKLOG.md), [`docs/08_DECISIONS.md`](docs/08_DECISIONS.md), et l'étude des 8 pistes : [`docs/10_FAISABILITE_8_PISTES.md`](docs/10_FAISABILITE_8_PISTES.md).
- **Prochaine étape : F0-01**, FM80S (le firmware de SLOOP, renommé, avec son image de démarrage) sur le FM-1.
- Le code actuel de ce dépôt est l'ancienne plateforme, reprise de [Trawler](https://github.com/y6zkcqnf5r-hue/fm1-trawler) (P0-01, la batterie au clavier dans le navigateur) ; il sera gardé sous le tag `plateforme-trawler` quand main repartira de SLOOP.

## Licence et marques

GPL-3.0-only, texte complet dans [`LICENSE`](LICENSE). « M-VAVE » et « FM-1 » sont des marques de leurs propriétaires. FM80S est un firmware indépendant : il n'est ni affilié à M-VAVE, ni approuvé, ni soutenu par la société. Les instruments cités pour décrire un caractère sonore (Juno, DX7, OP-1…) sont des marques de leurs propriétaires ; FM80S n'en reprend ni les noms, ni les graphismes.
