# 10 — Faisabilité : SLOOP à 8 pistes

> 2026-10-10. Étude faite en lisant le code de SLOOP 2.5 (commit `fa9ce57` de isod89/sloop-fm1, cloné dans `vendor-ref/sloop-fm1`), sans rien compiler : les tailles sont calculées à la main d'après les structures, sauf `sizeof(project_t)` qu'un test de SLOOP vérifie. **Les chiffres sont à confirmer au premier build (F0-01) et en F1-01.**

## 1. Verdict

**Faisable, mais c'est une refonte moyenne à difficile, pas un changement de constante.**

- La plupart du code (séquenceur, audio, éditeur, MIDI) boucle déjà sur `NTRK` et `NPART` : il suit tout seul.
- Trois choses bloquent un simple `NTRK 8` :
  1. **la mémoire** : le pool est presque plein ;
  2. **les projets** : le format et la zone qui garde les 4 sections sont déjà pleins ;
  3. **l'interface** : tous les écrans sont dessinés pour 4 pistes, 4 couleurs, 4 potards.

## 2. Ce qu'est une piste dans SLOOP

`firmware/src/core.h:9-13` :

| Constante | Valeur | Sens |
| --- | --- | --- |
| `NVOICE` | 8 | 8 voix par partie, et aussi le **total** partagé par toutes les parties de synthé |
| `NPART` | 3 | Les pistes 1–3 sont des synthés |
| `NTRK` | 4 | Les synthés, plus la batterie |
| `TRK_DRUM` | 3 | La batterie est la dernière piste |

Les masques `song.rec` et `song.solo` (`core.h:280-283`) sont sur 8 bits : **8 pistes est leur maximum**. À 8 pistes : `NPART 7`, `NTRK 8`, `TRK_DRUM 7`.

## 3. La mémoire

### 3.1 Les zones (`firmware/app.ld`)

| Zone | Taille | Aujourd'hui |
| --- | --- | --- |
| RAM (`.data`, `.bss`) | 96 Kio | **Marge inconnue**, à mesurer au premier build (sloopDX, plus gros, est à ~85 Kio) |
| Pool (gros tampons) | 336 Kio | ~334 Ko utilisés, **~10 Ko libres** ; `tools/build.py:252` refuse le build sous 8 Kio libres |
| `.noinit` (survit aux redémarrages) | 15 696 o | Les 4 sections A–D : **pleine** |
| Code | ~568 Kio | Serré : le moteur SLICE n'y tient déjà plus |

Le pool aujourd'hui : delay 128 Kio, bande d'écran 58 Kio, mémoire du punch-in 64 Kio, slicer 32 Kio (`sl_buf[NTRK][4096]`), arènes des moteurs 3 × ~8,6 Ko, `song_keep` et `autosave_buf` 2 × 3840 o, visualiseur ~5,5 Ko.

### 3.2 Ce que coûtent 8 pistes, naïvement

| Élément | Zone | En plus |
| --- | --- | --- |
| Arènes des moteurs lourds (`eng_arena[NPART]`) | pool | +34 Ko |
| Slicer (`sl_buf`) | pool | +32 Ko |
| `song_keep` + `autosave_buf` (7600 o chacun) | pool | +7,5 Ko |
| `trk[]` (~1940 o par piste, dont 8 voix et 64 pas) | RAM | +7,8 Ko |
| `proj_tmp` | RAM | +3,8 Ko |
| Drawbar et FM6 par partie | RAM | ~+5 Ko |
| Les 4 sections (`proj_slot[4]`) | `.noinit` | +15 Ko : **déborde** (30 400 o pour 15 696) |

Soit ~+74 Ko dans le pool, qui n'en a que 10.

### 3.3 La solution retenue (ADR-039, ADR-040)

- **Les moteurs lourds (GRAIN, FM6, PHYS) ne vont que sur les pistes 1 à 3**, qui gardent leur arène (`engines.c:37-45`). Les pistes 4 à 7 prennent les moteurs légers : analogique, FM 4 opérateurs, distorsion de phase, lo-fi, échantillons, formant, trio, orgue, bruit. La liste exacte se vérifie en F1-01 (l'orgue a un état par partie). → +0 Ko au lieu de +34.
- **Un seul tampon de slicer, partagé** : une seule piste à la fois peut avoir le slicer. → −24 Ko par rapport à aujourd'hui.
- Reste `song_keep` et `autosave_buf` : +7,5 Ko.

**Bilan pour le pool : ~16 Ko de marge en plus qu'aujourd'hui.** Le delay, la réverbe et le punch-in restent intacts.

Pour la RAM, ~+17 Ko : si la marge n'y est pas, on déplace `proj_tmp` ou les tables par partie dans le pool, qui aura de la place.

## 4. Les projets et les sections

- `project_t` fait **3840 o**, exactement un objet flash (`project.c`, `storage.c:17`). Le test `tests/project_test.c:129` le dit : « un paramètre de plus demande une nouvelle disposition ».
- À 8 pistes : **~7600 o**, plus qu'un secteur de 4 Ko → **nouveau format `FUN6`**, une conversion depuis `FUN5` au chargement, et une **nouvelle carte de la flash** (`storage.c:19-24`).
- Le nombre de pistes n'est pas écrit dans le fichier : il se déduit de la taille. On l'écrira dans `FUN6`.
- **Le point le plus dur : les 4 sections A–D vivent en `.noinit`**, qui ne peut pas grandir. Pistes possibles, à trancher en F1-01 mesures à l'appui (Q-11) :
  - garder les sections en flash et ne charger que la suivante, hors du callback audio ;
  - un format creux : on n'écrit que les pas des pistes utilisées ;
  - moins de verrous de paramètres par piste (24 aujourd'hui).
- Les structures des anciens formats (`project.c:53-92`) sont dimensionnées avec `NTRK` : il faut les figer à 4.

## 5. Les voix et le CPU

- Les synthés partagent **8 voix au total** (`voice.c:6-13`) ; la batterie a ses 6 voix à elle (`drums.c:9`). On garde ce budget : 8 pistes ne veulent pas dire plus de voix en même temps.
- **Un vrai risque :** `voice.c:168` dit que le cas « plus de voix que le budget » « ne peut pas arriver avec 3 parties ». Avec 7 parties qui protègent chacune leur note la plus grave, il peut arriver. À corriger en F1-01.
- Le coût en plus vient surtout du **mix par piste** (saturation, slicer, filtre, envois, `fx.c:360-400`), des LFO et des crochets de moteur : à mesurer.
- SLOOP mesure le CPU sur l'ordinateur, en relatif (`tests/cpu_baseline.txt` : repos + batterie = 1000, 3 synthés pleins + batterie = 2833), et sur l'appareil (GLO → SYSTEM, potard 4). Il coupe une voix quand deux demi-tampons de suite dépassent 85 % (`audio.c:46-81`). **On mesure sur le FM-1 en F1-01**, avant de dessiner l'interface à 8.

## 6. L'interface

Tout est pensé pour 4 :

- l'écran TRACKS : 4 bandes de 60 px (`ui_draw.c:455-560`) ; la vue d'enregistrement : 4 rangées (`ui_studio.c:200-232`, `581-612`) ;
- 4 couleurs de piste `TE_COL` / `TE_MID` / `TE_DIM` (`ui_studio.c:19-22`), indexées par `& 3` dans une dizaine d'endroits ;
- la couche GLO (mix) : touches 1–4 mute, 5–8 solo, 9 et 10 fills, 16 tap tempo ; potards 1–4 = niveaux des pistes 1–4 (`ui_layers.c:15-16`, `410-422`, `542-545`, `860-885`) ;
- le choix de piste : le potard ALGORITHM (`ui_input.c:936`).

C'est l'objet de F1-02 : écran à 8, 8 couleurs, choix de piste (SEL + touche noire, PRD §4), mute / solo / niveaux à 8.

## 7. Les endroits à changer, par catégorie

| Catégorie | Où | Travail |
| --- | --- | --- |
| Suit la constante | `seq.c`, `editor.c:898-1095`, `main.c:106-116`, `arranger_scene.c`, `voice.c`, `fx.c`, `project.c`, `ui_input.c`… | Rien, sauf vérifier le coût des boucles de `voice.c` |
| Mémoire | `core.h:203,290`, `engines.c:37-38`, `slicer.c:49,63`, `eng_fm6.c`, `eng_drawbar.c`, `eng_phys.c` | §3.3 |
| **Vrais bugs à 8** | `ui_studio.c:519` (choix de piste 0 ou 3), `voice.c:383` (`& 3u` : les pistes 5–8 se montrent en 1–4), `ui.c:251` (3 sons par défaut), `ui_draw.c:856` (« /4 » écrit en dur), `ui_vis.c:220-577` (piste 4 = batterie), `editor.c:912` (solo sur un octet SysEx de 7 bits), `voice.c:168` (vol de voix) | F1-01 |
| Interface | `ui_draw.c`, `ui_studio.c`, `ui_layers.c`, `ui_menu.c`, `ui.c:29` | F1-02 |
| Format des projets | `project.c`, `storage.c`, `tests/project_test.c` | F1-03 |
| Éditeur web | `web/editor.html` (lit le nombre de pistes de l'appareil, mais `FM6_NPART = 3`, 4 couleurs, la batterie supposée dernière piste, le simulateur `?mock=1` à 4), `web/EDITOR_PROTOCOL.md` | F1-03 |
| Tests | `tests/project_test.c`, `seq2_test.c`, `ui_pages_test.c`, `regress.c` (« le budget de 8 sur 3 parties »), `tests/golden.txt`, `web/test_web.mjs` (`ntrk === 4`) | Avec chaque story |

Ce qui n'est **pas** un nombre de pistes, à ne pas toucher : `proj_slot[4]`, `ARR_SCENES 4` et les lettres A–D (les 4 sections) ; les nombreuses boucles `k < 4u` (les 4 potards).

## 8. Dans l'ordre

1. **F0-01** : premier build, la vraie marge de RAM et de pool.
2. **F1-01** : 8 pistes sous le capot, les bugs, le vol de voix, la mesure du CPU sur le FM-1, et le choix pour les sections.
3. **F1-02** : l'interface à 8.
4. **F1-03** : le format `FUN6`, la flash, l'éditeur web.
