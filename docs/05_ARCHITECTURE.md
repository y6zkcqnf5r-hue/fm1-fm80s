# 05 — Architecture minimale de FM80S

> v0.1, 2026-10-10. La plateforme vient de Trawler (`../FM1`, dépôt archivé `y6zkcqnf5r-hue/fm1-trawler`, commit `153f4f4`), dont l'architecture détaillée reste la référence : `../FM1/docs/05_ARCHITECTURE.md`. Ce document ne dit que ce qui change, et ce qu'on reprend.

## 1. Les trois cibles

| Cible | Build | Rôle |
| --- | --- | --- |
| Hôte natif | `make host` → `build/host/fm80s` | Simulateur scripté : scénarios `.trs` → WAV + PNG, tests |
| Navigateur | `make web` → `build/emu/` (`fm80s.wasm`, `worklet.js`, `index.html`, `panel.js`, `keymap.js`) | Le FM-1 virtuel : le même code C en wasm dans un AudioWorklet |
| FM-1 | P0-02 : `plat_fm1.c` sur la couche matérielle de Felucca | L'appareil |

Le même code (`firmware/src/`) tourne sur les trois. Seuls `host/`, `web/emu/` et `plat_fm1.c` savent sur quelle cible ils sont.

## 2. Les dossiers

```
firmware/src/app/     main.c, plat.h (la frontière), config.h (limites et budgets), cmdq.*
firmware/src/dsp/     les briques : fixed.h, osc.h, env.*, svf.*, noise.h
firmware/src/engine/  les moteurs, un fichier chacun : engine.h, drums.c (P0-01), va.c (P2-01)…
firmware/src/core/    l'horloge, les pistes, le looper, les scènes (à partir de P1-01)
firmware/src/ui/      gfx.*, la police, un écran par moteur et par fonction
host/                 plat.h sur l'hôte, lanceur de scénarios, WAV, PNG
web/emu/              wrapper wasm, worklet, page, panneau SVG, clavier
tests/                unit/, scenarios/*.trs, golden/, host/ (parité, golden), lint/
tools/                gen_tables.py, gen_font.py, size_report.py, lint.sh
assets/fonts/         Terminus (OFL)
```

## 3. Ce qu'on reprend de Trawler (ADR-012)

On copie les fichiers, on renomme `trawler` en `fm80s`, et on note leur origine dans `LICENSING.md`. Il n'y a pas de lien entre les deux dépôts.

| Repris | Notes |
| --- | --- |
| `firmware/src/app/plat.h`, `config.h` | Les budgets tels quels ; les limites `TRK_*` du tracker disparaissent |
| `firmware/src/dsp/fixed.h`, `osc.h`, `env.*`, `svf.*` | Tels quels, avec leurs tests |
| `firmware/src/tracker/cmdq.*` | Passe dans `app/` |
| `sclock` de `firmware/src/tracker/seq.c` | Devient `core/clock.*` en P1-01, à 96 ticks par noire (spec §3) |
| `firmware/src/ui/gfx.*`, `tools/gen_font.py`, `assets/fonts/` | Le thème change (spec §5) |
| `host/` (sauf `mockup.c`), `web/emu/` | Renommés ; les sondes `expect` de l'app changent (`ui_probe`) |
| `tests/unit/test.h`, `render.h`, `test_fixed.c`, `test_gfx.c`, la partie osc / env / svf de `test_wav.c` ; `tests/host/*`, `tests/lint/` | |
| `tools/gen_tables.py`, `size_report.py`, `lint.sh`, `Makefile`, `.github/workflows/ci.yml`, `.claude/launch.json` | |

**Ce qu'on laisse :** `tracker/model`, `seq`, `instr`, `fx`, les vues et la saisie de `ui/`, et `dsp/voice.*`, une voix faite pour le tracker.

## 4. L'interface de moteur (ADR-014)

```c
/* firmware/src/engine/engine.h, la forme visée (elle peut bouger dans P0-01) */
#define ENG_PARAMS 128                  /* octets de son par piste */
#define ENG_METERS 16

typedef struct {
    volatile uint8_t meter[ENG_METERS]; /* écrit par l'audio : le niveau de chaque voix, 0..255 */
    /* ... l'état propre du moteur suit (union par piste, taille max fixée à la compilation) */
} eng_head_t;

typedef struct {
    const char *name;                   /* "DRUMS" */
    uint16_t state_size;                /* vérifiée à la compilation contre l'union */
    const uint8_t *defaults;            /* ENG_PARAMS octets : le son d'usine */
    void (*init)(void *st);
    void (*note_on)(void *st, const uint8_t *p, int note, int vel);   /* batterie : note = son 0..15 */
    void (*note_off)(void *st, const uint8_t *p, int note);
    void (*render)(void *st, const uint8_t *p, int32_t *lr, int n);   /* ajoute n frames L R, Q15 */
    int (*cost)(const void *st);        /* MAC par frame déclarés, voix actives seulement */
} engine_t;
```

- `p` désigne les octets du son de la piste. Ils sont écrits par l'interface seule et relus par le moteur à chaque sous-bloc de 16 frames : les potards agissent en direct.
- Les enveloppes et les coefficients se calculent par sous-blocs de 16 frames (règle 5).

## 5. La frontière entre l'interface et l'audio (ADR-015)

C'est celle de Trawler :

- `app_frame()` (~60 Hz) lit les contrôles, pousse les notes dans `cmdq`, écrit les octets de son et dessine ;
- `app_audio()` vide `cmdq`, appelle `render` pour chaque piste, applique MASTER avec une rampe, et publie les niveaux (`meter`).

Il y a un seul écrivain par donnée partagée, et chaque écriture se fait en un seul coup sur 8 ou 32 bits.

## 6. Le build et les commandes

Ce sont les commandes de Trawler, avec les mêmes flags (`-ffp-contract=off -fwrapv`) et Emscripten **6.0.11** figé (en Docker avec `DOCKER=1`, ou en emsdk local à la même version) :

```sh
make host        # simulateur natif
make web         # FM-1 virtuel dans build/emu
make serve       # http://localhost:8080
make test        # lint + unitaires + golden + taille + scénarios + parité wasm/natif
make golden      # accepte les nouveaux WAV et PNG, après écoute
make size        # RAM, pool, code face aux budgets
make lint        # pas de float/double/malloc/printf dans firmware/src, en-têtes SPDX
```

Make 3.81 de macOS compare les dates à la seconde près : après une modification automatique, supprimer le binaire avant de relancer.

## 7. Les tests (ADR-023)

| Niveau | Ce qu'on vérifie |
| --- | --- |
| Unitaires | Point fixe, osc / env / svf (repris), bruit LFSR déterministe, chaque son de batterie |
| Rendus de référence | `tests/unit/test_drums.c` écrit un WAV par son dans `build/renders/drums/` ; leurs hashes sont dans `tests/golden/renders/drums.sha256`, acceptés seulement après écoute |
| Scénarios | `tests/scenarios/drums.trs` : frapper les touches, tourner les potards, `wav`, `shot` ; golden du WAV et du PNG |
| Parité | `tests/host/emu_test.mjs` : le wasm rejoue chaque scénario et rend les mêmes octets que le natif |
| Taille | `make size` : objets ELF 32 bits par clang (ADR-020 de Trawler), budgets de `app/config.h` |

## 8. Le budget CPU

**Avant P0-02**, on applique le modèle de coût de Trawler (`../FM1/docs/05_ARCHITECTURE.md` §7) :
- chaque moteur déclare un coût en MAC par frame et par voix active ;
- la page affiche la jauge « CPU estimé », rouge au-dessus de 50 %.

**En P0-02**, sur l'appareil :
- on mesure `fm1_ticks()` autour de `app_audio`, et on l'affiche à l'écran ;
- on recale la référence du modèle sur cette mesure ;
- on fixe le nombre de voix par moteur.

## 9. La CI et GitHub Pages (ADR-024)

- Le même workflow que Trawler : `make lint test web DOCKER=1` à chaque push et à chaque PR, avec les actions épinglées par commit.
- Un push sur `main` publie `build/emu` sur https://y6zkcqnf5r-hue.github.io/fm1-fm80s/.
- Un réglage manuel est à faire une fois dans le dépôt GitHub : *Settings → Pages → Source : GitHub Actions*.
