# Licence de FM80S

FM80S est un logiciel libre sous la GNU General Public License, version 3 uniquement (`GPL-3.0-only`, texte complet dans `LICENSE`). Si vous distribuez FM80S, ou un firmware qui en dérive, vous devez fournir à vos destinataires le source complet correspondant, sous la même licence.

Chaque fichier source commence par `SPDX-License-Identifier: GPL-3.0-only`. Le code repris d'un autre projet garde son en-tête et sa licence d'origine, et il est listé ci-dessous.

## D'où vient le code

La plateforme, le DSP de base, l'écran, les tests et les outils viennent de **Trawler** ([y6zkcqnf5r-hue/fm1-trawler](https://github.com/y6zkcqnf5r-hue/fm1-trawler), commit `153f4f4`, archivé), le tracker du même auteur arrêté le 2026-10-10. Ils ont été copiés puis renommés (`trawler` → `fm80s`), sans lien entre les deux dépôts (ADR-012). Les lignes ci-dessous reprennent l'origine de chaque morceau telle que Trawler la notait.

| Quoi | Origine | Licence |
| --- | --- | --- |
| Méthode de la table de hauteurs (une seule octave stockée, décalée pour les autres) : `tools/gen_tables.py`, `pitch_inc()` dans `firmware/src/dsp/fixed.h` | [Felucca](https://github.com/hugelton/Felucca) `tools/gen_tables.py`, Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments ; réécrit pour Trawler | GPL-3.0-only |
| Forme de l'interface `firmware/src/app/plat.h` | [X0X](https://github.com/charlesvestal/fm1-x0x) et [FoMni](https://github.com/charlesvestal/fm1-fomni) de Charles Vestal ; réécrite pour Trawler | GPL-3.0-only |
| Simulateur hôte (`host/`) : horloge `host_run_ms`, langage de scénarios `.trs`, PNG en blocs deflate « stored » sans zlib | [FoMni](https://github.com/charlesvestal/fm1-fomni) `host/omni_host.c` et [X0X](https://github.com/charlesvestal/fm1-x0x) `host/x0x_host.c` de Charles Vestal ; réécrit pour Trawler | GPL-3.0-only |
| Panneau `web/emu/panel.js` : géométrie du FM-1 (viewBox 1000×640), SVG sous des zones de clic HTML, potards au glisser et à la molette, verrou des boutons | [X0X](https://github.com/charlesvestal/fm1-x0x) et [FoMni](https://github.com/charlesvestal/fm1-fomni) `web/emu/index.html` de Charles Vestal ; réécrit pour Trawler (légendes gravées, légende du clavier par disposition, appui long) | GPL-3.0-only |
| Build web (`web/emu/fm80s_web.c`, `worklet.js`, `index.html`) : wasm sans glue Emscripten et faux WASI, exports `web_*`, anneau audio rempli par l'horloge de l'appareil, messages `load` / `frame`, conversion de l'écran, clavier relâché sur `blur` | [X0X](https://github.com/charlesvestal/fm1-x0x) et [FoMni](https://github.com/charlesvestal/fm1-fomni) `web/emu/` de Charles Vestal ; réécrit pour Trawler | GPL-3.0-only |
| Test de parité `tests/host/emu_test.mjs` : le module chargé dans node avec les bouchons du worklet, comparaison échantillon par échantillon avec le natif | [ChoralRootFM1](https://github.com/Quixotic7/ChoralRootFM1) `tools/emu/web/test_web_emu.mjs` (Quixotic7) ; réécrit pour Trawler | GPL-3.0-only |
| CI `.github/workflows/ci.yml` : actions épinglées par commit, Pages par `upload-pages-artifact` et `deploy-pages` | [ChoralRootFM1](https://github.com/Quixotic7/ChoralRootFM1) `.github/workflows/pages.yml` (Quixotic7) ; réécrit pour Trawler | GPL-3.0-only |
| Emscripten en Docker sous l'utilisateur courant, cache dans le dossier de build (`Makefile`, `DOCKER=1`) | [Hortator](https://github.com/deadactive/hortator) `tools/build_sim.sh` (DEADACTIVE) | GPL-3.0-only |
| Générateur de police `tools/gen_font.py` (lecture du BDF, découpe), `firmware/src/ui/gfx.c`, `firmware/src/dsp/env.c`, `firmware/src/dsp/voice.c`, `firmware/src/app/cmdq.c`, `tools/size_report.py`, `tools/lint.sh` | Trawler | GPL-3.0-only |
| Filtre `firmware/src/dsp/svf.c` | Écrit d'après les équations publiées du « Linear Trapezoidal Integrated SVF » d'Andrew Simper (Cytomic, 2013) ; aucun code repris | GPL-3.0-only |
| Oscillateurs `firmware/src/dsp/osc.h` (polyBLEP) | Écrit d'après Välimäki et Huovilainen, « Antialiasing oscillators in subtractive synthesis » (IEEE SPM, 2007) ; aucun code repris | GPL-3.0-only |
| Tout le reste | FM80S | GPL-3.0-only |

## Matériel tiers

| Quoi | Origine | Licence | Fichiers |
| --- | --- | --- | --- |
| Police Terminus 12 px (6×12) et 16 px (8×16) | Copyright (C) 2020 Dimitar Toshkov Zhekov, copiées depuis [X0X](https://github.com/charlesvestal/fm1-x0x) `assets/fonts/` | SIL OFL 1.1 | `assets/fonts/ter-u12n.bdf`, `assets/fonts/ter-u16n.bdf`, `assets/fonts/Terminus-LICENSE.txt` ; les glyphes générés dans `build/gen/font.h` restent sous OFL. Les faces 6×10 (Terminus 12, lignes 1–10) et 8×12 (Terminus 16, lignes 2–13) sont des versions modifiées au sens de l'OFL, générées par `tools/gen_font.py` (ADR-027) ; la licence ne réserve aucun nom de police |

## Marques

« M-VAVE » et « FM-1 » sont des marques de leurs propriétaires. FM80S est un firmware indépendant : il n'est ni affilié à M-VAVE, ni approuvé, ni soutenu par la société. Les instruments cités dans la documentation pour décrire un caractère sonore (Juno, DX7, OP-1…) sont des marques de leurs propriétaires ; FM80S n'en reprend ni les noms, ni les graphismes.

## Radio

FM80S n'active jamais la radio Bluetooth ou Wi-Fi de l'appareil.
