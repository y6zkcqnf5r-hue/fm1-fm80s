# 05 — Architecture de FM80S (fork de SLOOP)

> v0.2, 2026-10-10. FM80S est un fork de SLOOP 2.5 (ADR-036) : l'arbre, le build et les tests sont ceux de SLOOP, décrits dans son `BUILDING.md` et en tête de ses fichiers. Ce document dit comment on s'y place et ce qu'on y ajoute. L'architecture v0.1 (plateforme de Trawler, `plat.h`, interface de moteur) est gardée sous le tag `plateforme-trawler` (ADR-045).
>
> Depuis F0-01, main part de l'historique de SLOOP 2.5 (remote `sloop`) ; l'ancienne plateforme est sous le tag `plateforme-trawler`.

## 1. Les cibles

| Cible | Build | Rôle |
| --- | --- | --- |
| FM-1 | `./build.sh` → `build/fm80s.fwsc` (le même paquet que `felucca.fwsc`, que lisent les tests de SLOOP ; identité `FM-1_900`) | Le firmware installé depuis Chrome ou Edge (installateur web de SLOOP) ou `tools/fm1_install.py` |
| Ordinateur | `tests/run_tests.sh` → `build/host/` | Les tests de SLOOP : rendus WAV (`tests/hostsim.c`), écrans en PPM (`tests/ui_pages_test.c`), golden, budget CPU |
| Navigateur | F0-02 : `web/emu/` | Le FM-1 virtuel : le même firmware en wasm, dans un AudioWorklet (ADR-041) |

## 2. L'arbre de SLOOP, en bref

```
firmware/src/felucca.c   la seule unité de compilation : elle inclut tous les .c ci-dessous
firmware/src/core.h      NVOICE, NPART, NTRK, TRK_DRUM, les structures des pistes et de la song
firmware/src/eng_*.c     les 12 moteurs ; engines.c (arènes partagées de GRAIN, FM6, PHYS)
firmware/src/seq.c       le séquenceur et le looper ; voice.c (budget de 8 voix) ; drums.c ; fx.c
firmware/src/ui*.c       les écrans, les calques (ui_layers.c), la saisie (ui_input.c), splash.c
firmware/src/project.c   le format des projets (FUN5) ; storage.c (la flash) ; editor.c (SysEx de l'éditeur)
firmware/hal/            la couche matérielle de Felucca (fm1_*.h) ; firmware/app.ld (les zones mémoire)
tools/                   build.py, gen_tables.py, gen_logo.py, gen_font.py, fm1pkg_make.py…
tests/                   run_tests.sh, regress.c, golden.txt, cpu_baseline.txt, hostsim.c, ui_pages_test.c…
web/                     l'éditeur (editor.html), l'installateur, EDITOR_PROTOCOL.md
docs/                    la page publiée de SLOOP (installateur, éditeur) — nos docs y sont ajoutées
```

Nos documents (`03_PRD.md` à `10_FAISABILITE_8_PISTES.md`) sont dans `docs/` à côté de la page de SLOOP (aucun nom en commun). Notre page Pages est publiée par un workflow, pas depuis `docs/` : nos `.md` n'y sont pas servis.

## 3. Le dépôt et l'amont (ADR-037)

- **F0-01 :** tag `plateforme-trawler` sur l'ancienne main ; main repart de l'historique de SLOOP (`fa9ce57`, version 2.5), puis un commit ajoute nos docs et notre `CLAUDE.md` ; push forcé, validé par Anthony avant.
- **L'amont :** `git remote add sloop https://github.com/isod89/sloop-fm1.git`. Quand SLOOP publie une version, on la fusionne dans une branche, on lance les tests, et Anthony essaie avant de fusionner dans main.
- **Pour que ces fusions restent faciles :**
  - on change le moins de lignes possible dans les fichiers de SLOOP ;
  - ce qui est à nous va dans des fichiers à nous quand c'est possible (`fm80s_*.c`, `tools/gen_fm80s_*.py`, `web/emu/`) ;
  - on marque nos changements dans les fichiers de SLOOP d'un commentaire `/* FM80S: ... */`.
- `LICENSING.md` de SLOOP est gardé et complété (nos fichiers, l'image de démarrage, le code repris de P0-01 pour l'émulateur).

## 4. Le build et les tests

Les prérequis de SLOOP sur Mac (`BUILDING.md`) :
- Docker Desktop : la chaîne JieLi (clang 4.0.1 pour pi32v2) est Linux x86-64 seulement ; chaque outil tourne dans un conteneur `linux/amd64` (Rosetta sur Apple silicon) ;
- la chaîne JieLi : `tools/get_toolchain.sh` ;
- 3 fichiers du SDK AC79 (Apache-2.0) : un clone de `fw-AC79_AIoT_SDK` dans `~/fw-AC79_AIoT_SDK` ;
- Python 3 avec Pillow ; Node.js pour les tests web.

```sh
./build.sh                # le firmware : build/fm80s.fwsc (app + loader)
tests/run_tests.sh        # les tests sur l'ordinateur (demande le build d'abord)
GOLDEN_UPDATE=1 sh tests/run_tests.sh   # après un changement de son voulu, puis relire le diff de golden.txt
```

- `tools/build.py` refuse le build si le pool a moins de 8 Kio libres : c'est notre garde-fou mémoire.
- Le budget CPU se compare à `tests/cpu_baseline.txt` (+25 %) et `tests/target_budget.txt` (+10 %).
- **F0-01, sur le Mac d'Anthony :** le build marche ; SLOOP 2.5 se reconstruit à l'identique de son paquet publié (seule la date de build change). Les 3 fichiers du SDK se téléchargent seuls depuis gitee (empreintes vérifiées par `build.py`), sans cloner le SDK ; chaîne JieLi `jieli-linux-toolchains-20260730.1` (SHA-256 `a1545cdd…47bd`). Le budget CPU a été recalé sur SLOOP 2.5 tel quel (les comptes d'instructions dépendent de la machine, et `target_budget.txt` n'avait pas suivi SLOOP 2.4 et 2.5). Mesures : `10_FAISABILITE_8_PISTES.md` §3.1.

## 5. Le FM-1 virtuel (F0-02, F0-03, ADR-041)

```
web/emu/fm80s_emu.c   inclut les sources du firmware comme tests/ui_pages_test.c, avec des bouchons fm1_* :
                      entrées (touches, boutons, potards), écran (la bande de 240×124 recopiée dans un
                      tampon 240×240), LED, flash en mémoire, minuterie avancée par l'audio
web/emu/worklet.js    l'AudioWorklet : blocs de 256 frames à 44,1 kHz (repris de P0-01)
web/emu/index.html, panel.js, keymap.js   la page, le panneau SVG, le clavier AZERTY (repris de P0-01)
```

- Compilé en wasm **sans la colle d'Emscripten**, comme en P0-01, avec Emscripten **6.0.11** figé (Docker ou emsdk local).
- **L'horloge avance avec l'audio** : demander des frames fait tourner le firmware, comme dans P0-01 ; pas de dérive entre le son et l'écran.
- La page demande ses fichiers avec `?v=<somme du build>` (leçon de P0-01 : Safari gardait l'ancien worklet).
- **F0-03, la parité :** le wasm rend les mêmes octets que le natif pour les rendus de `tests/golden.txt`, avec `-ffp-contract=off` (SLOOP a des `float` dans `fm6_core.c` et `phys_dsp.c`).
- Le code repris de P0-01 et de Trawler garde son en-tête et ses crédits (Charles Vestal pour la forme du worklet et du panneau).

## 6. La CI et la page publiée (F0-03)

- SLOOP n'a pas de CI. On en ajoute une : `./build.sh` et `tests/run_tests.sh` sur Linux x86-64 (la chaîne JieLi y tourne sans Docker), puis le build de l'émulateur.
- La page publiée sur https://y6zkcqnf5r-hue.github.io/fm1-fm80s/ montre le FM-1 virtuel et l'installateur de FM80S.

## 7. Le CPU

- On ne réécrit pas de modèle de coût : SLOOP mesure en relatif sur l'ordinateur (`cpu_baseline.txt`) et sur l'appareil (GLO → SYSTEM, potard 4). Il coupe une voix quand deux demi-tampons de suite dépassent 85 %.
- **F1-01 mesure les 8 pistes sur le FM-1**, avec le même affichage, avant de dessiner leur interface.
