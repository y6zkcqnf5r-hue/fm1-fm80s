# FM80S — un synthé-looper ludique pour le M-VAVE FM-1

Un firmware libre (GPL-3.0) qui fait du FM-1, le petit synthé de poche de M-VAVE, un **synthé-looper** : on joue une boucle au clavier, elle tourne, on en joue une autre avec un autre instrument par-dessus, et ainsi de suite ; ou on la programme pas à pas. Les boucles s'enchaînent en une song. Une dizaine de moteurs de synthèse, chacun avec son écran vivant, une orientation synthwave, un arpégiateur et des effets à déclencher en direct.

On le développe d'abord dans le navigateur, avec **le même code C** que celui qui tournera sur l'appareil.

## Où en est-on

- **La vision : écrite**, à relire dans [`docs/03_PRD.md`](docs/03_PRD.md).
- **La spec, l'architecture et le backlog : écrits, en version minimale** : [`docs/04_SPEC.md`](docs/04_SPEC.md), [`docs/05_ARCHITECTURE.md`](docs/05_ARCHITECTURE.md), [`docs/06_BACKLOG.md`](docs/06_BACKLOG.md), [`docs/08_DECISIONS.md`](docs/08_DECISIONS.md).
- **P0-01, la batterie au clavier dans le navigateur : faite, à essayer.** 16 sons de batterie synthétique sur les touches blanches, les 4 potards sur le dernier son frappé, l'écran des pads. `make serve`, puis http://localhost:8080, ou la page publiée : https://y6zkcqnf5r-hue.github.io/fm1-fm80s/.
- **Prochaine étape : P0-02**, le premier son sur le FM-1.
- Le code reprend la plateforme et le DSP de [Trawler](https://github.com/y6zkcqnf5r-hue/fm1-trawler), un tracker pour le FM-1 arrêté (archivé) parce que l'ergonomie du FM-1 ne convient pas à un tracker.

## Licence et marques

GPL-3.0-only, texte complet dans [`LICENSE`](LICENSE). « M-VAVE » et « FM-1 » sont des marques de leurs propriétaires. FM80S est un firmware indépendant : il n'est ni affilié à M-VAVE, ni approuvé, ni soutenu par la société. Les instruments cités pour décrire un caractère sonore (Juno, DX7, OP-1…) sont des marques de leurs propriétaires ; FM80S n'en reprend ni les noms, ni les graphismes.
