# 04 — Spec de FM80S : ce qui change dans SLOOP

> v0.2, 2026-10-10. FM80S est un fork de SLOOP 2.5 (ADR-036) : **le comportement de référence est celui de SLOOP**, décrit dans son `GUIDE.md` et son `SLOOP.md`. Cette spec ne dit que ce que FM80S change. Ce qui est marqué *provisoire* se tranche dans sa story, à l'essai. Vision : `03_PRD.md`. Décisions : `08_DECISIONS.md`.
>
> La spec v0.1 (interface de moteur, moteur Drums v1, boucle par événements, budget mémoire de la plateforme Trawler) est gardée sous le tag `plateforme-trawler` (ADR-045).

## 1. L'identité (F0-01)

| Élément | Dans SLOOP | Dans FM80S |
| --- | --- | --- |
| Nom à l'écran | SLOOP | **FM80S** (démarrage, ABOUT, secours `FM80S USB RESCUE`, fichier `fm80s-VER.fwsc`) ; version `FM80S 0.1`. Le visualiseur « SLOOP » s'appelle « SAIL » |
| Crédit | « based on felucca » sous le logo | « by Floydus » et « inspired by SLOOP & Felucca » sous l'image ; dans ABOUT, BY FLOYDUS, INSPIRED BY SLOOP & FELUCCA, ISOD89 / LEO KUROSHITA, HÜGELTON INSTRUMENTS, la GPL et notre source (ADR-044, ADR-047) |
| Image de démarrage | `splash.c` + `tools/gen_logo.py` : 240×188, 16 couleurs, RLE, ~900 ms | **Faite (F0-01) :** `tools/gen_fm80s_splash.py` + `fm80s_splash.c`, même format (RLE 6649 o) : ciel de nuit, soleil couchant à bandes, grille en perspective, « FM80S » en cyan chromé, « v0.1 » en haut à droite. Validée par Anthony sur capture. Aperçu : `assets/logo/fm80s-splash.png` |
| Thème | 5 palettes d'écran au choix (menu COLOR) | *Provisoire :* une palette « FM80S » en plus, proche du thème de P0-01 (nuit violette, magenta, cyan) ; les palettes de SLOOP restent au choix |
| Icônes | `assets/icons.png` (Hügelton, permission GPL §7) | Inchangées jusqu'à Q-12 |

Rien d'autre ne change en F0-01 : les golden de SLOOP restent identiques (vérifié : 182 rendus, 0 changé).

## 2. Le FM-1 virtuel (F0-02, F0-03)

- Le même code que le firmware : `firmware/src/` compilé en wasm, sans en changer une ligne, avec des bouchons `fm1_*` (entrées, écran, LED, flash, minuterie), comme le font déjà `tests/hostsim.c` et `tests/ui_pages_test.c`.
- Il joue dans un AudioWorklet à 44,1 kHz, par blocs de 256 frames, comme l'appareil.
- L'écran 240×240 dessiné sur un canvas, les LED des touches et des boutons sur le panneau.
- Le clavier d'ordinateur AZERTY et le panneau de P0-01 : touches blanches sur la rangée du bas et du milieu, noires sur les chiffres, ⌥ + 1–8 pour les potards, clic droit pour verrouiller un bouton (calques à tenir).
- F0-03 : parité wasm/natif (les rendus de `tests/golden.txt`), projets gardés dans le navigateur, page publiée.

## 3. Les 8 pistes (F1)

### 3.1 Le modèle (F1-01)

| | SLOOP | FM80S |
| --- | --- | --- |
| Pistes | 4 : 3 synthés + batterie | **8 : 7 synthés + batterie** (`NPART 7`, `NTRK 8`, `TRK_DRUM 7`) |
| Voix | 8 partagées par les synthés, 6 pour la batterie | Inchangé (ADR-038) |
| Moteurs lourds (GRAIN, FM6, PHYS) | Sur les 3 synthés | **Pistes 1 à 3 seulement** (ADR-039). Sur les pistes 4 à 7, ils n'apparaissent pas dans le choix du moteur ni dans PRESETS |
| Slicer | Un tampon par piste | **Un tampon partagé** : l'activer sur une piste le retire de l'autre (ADR-040) |
| Sons par défaut d'un nouveau projet | 808 BOOM, RHODES, LOFI FLUTE, kit 808 | *Provisoire :* une basse, une nappe, un lead, puis des pistes vides ; le kit synthwave sur la batterie |

Détail des fichiers touchés et des bugs à corriger : `10_FAISABILITE_8_PISTES.md` §7.

### 3.2 L'interface à 8 (F1-02, provisoire)

- **Écran TRACKS :** 8 bandes de 30 px, ou 2 rangées de 4 ; on dessine les deux et Anthony choisit sur une capture.
- **8 couleurs de piste**, synthwave : magenta, cyan, orange, violet, jaune, rose, bleu, vert ; la batterie garde la dernière. Remplacent les 4 couleurs `TE_*`, indexées par piste au lieu de `& 3`.
- **Choisir la piste :** SEL + touche noire (PRD §4) ; le potard ALGORITHM de SLOOP marche toujours.
- **Couche GLO (mix) :** touches 1–8 mute ; solo en tenant SEL + touche 1–8 ; les potards 1–4 règlent les niveaux des pistes 1–4, SEL + potards ceux des pistes 5–8. Les fills (touches 9, 10) et le tap tempo (16) gardent leur place.

### 3.3 Les projets à 8 pistes (F1-03)

- Format **`FUN6`** : le nombre de pistes écrit dans l'en-tête ; conversion depuis `FUN5` (les pistes 4–7 vides, la batterie de SLOOP passe en piste 8).
- Nouvelle carte de la flash, et une solution pour les 4 sections (Q-11), décidée en F1-01.
- L'éditeur web et `web/EDITOR_PROTOCOL.md` suivent : nombre de pistes lu sur l'appareil, 8 couleurs, solo sur deux octets SysEx.

## 4. Le reste du PRD

Les contrôles, le punch-in, les scènes, le mode facile et les moteurs suivent SLOOP **jusqu'à ce qu'Anthony l'ait joué** (Q-01, Q-02, Q-07, Q-08). On écrit leur spec dans la story qui les change.
