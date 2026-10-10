# 08 — Décisions et questions ouvertes

## Le fork de SLOOP (2026-10-10, après `09_COMPARAISON.md`)

| ID | Date | Décision | Pourquoi |
| --- | --- | --- | --- |
| ADR-036 | 2026-10-10 | **FM80S devient un fork de SLOOP 2.5** (isod89/sloop-fm1, GPL-3.0, sur Felucca), option C de `09_COMPARAISON.md`. La plateforme reprise de Trawler est abandonnée | SLOOP marche déjà sur l'appareil (CPU, flash, installateur, 12 moteurs, looper, séquenceur) : on travaille dès la 1re story sur ce qui distingue FM80S. Anthony a décidé d'après le README, sans avoir encore joué SLOOP |
| ADR-037 | 2026-10-10 | **On reste dans `fm1-fm80s`.** L'ancienne main est gardée sous le tag `plateforme-trawler` ; main repart de l'historique de SLOOP, plus nos docs ; SLOOP est le remote `sloop`, d'où l'on fusionne ses corrections de temps en temps. Remplacer main demande un push forcé, validé par Anthony avant (F0-01) | Le nom, la page et l'historique de FM80S restent ; l'historique de SLOOP garde le lien avec l'amont pour suivre ses versions |
| ADR-038 | 2026-10-10 | **8 pistes = 7 synthés + la batterie** (`NPART 7`, `NTRK 8`, `TRK_DRUM 7`). Le budget de voix ne change pas : 8 voix partagées par les synthés, 6 pour la batterie | La différence majeure avec SLOOP (PRD §3). Les masques de SLOOP sont sur 8 bits : 8 est le maximum sans refonte. Plus de voix coûterait du CPU qu'on ne connaît pas encore (`10_FAISABILITE_8_PISTES.md` §5) |
| ADR-039 | 2026-10-10 | **Les moteurs lourds (GRAIN, FM6, PHYS) ne vont que sur les pistes 1 à 3** ; les pistes 4 à 7 prennent les moteurs légers. Liste exacte en F1-01 | Leurs arènes coûteraient +34 Ko dans un pool qui n'a que ~10 Ko de libre (`10_FAISABILITE_8_PISTES.md` §3.3). Le delay, la réverbe et le punch-in restent intacts |
| ADR-040 | 2026-10-10 | **Un seul tampon de slicer, partagé** : une seule piste à la fois a le slicer | −24 Ko dans le pool par rapport à aujourd'hui. Avec ADR-039, le pool garde ~16 Ko de marge en plus |
| ADR-041 | 2026-10-10 | **Un émulateur web avant toute fonction nouvelle** (F0-02, F0-03) : le vrai firmware en wasm, construit comme `tests/ui_pages_test.c` de SLOOP (les sources du firmware incluses, des bouchons `fm1_*`), avec le worklet, le panneau et le clavier de P0-01 | Anthony essaie et juge dans le navigateur (la leçon de Trawler) ; SLOOP n'a pas d'émulateur, mais son code tourne déjà sur l'ordinateur pour ses tests |
| ADR-042 | 2026-10-10 | **F0-01 passe avant l'émulateur** : le fork compilé, renommé, avec son image de démarrage, installé sur le FM-1 | Elle ne change aucune fonction, et le FM-1 arrive la semaine du 2026-10-12 : Anthony joue SLOOP en main avant qu'on change quoi que ce soit |
| ADR-043 | 2026-10-10 | **Les règles de code sont celles de SLOOP et de la puce** (`CLAUDE.md`) : pas de `malloc` ni de libc hors `libc.c`, gros tampons dans le pool, rien de bloquant dans le callback audio, golden et budget CPU de SLOOP. Le code qu'on écrit reste en entiers ; les `float` de `fm6_core.c` et `phys_dsp.c` (code repris par SLOOP) restent tels quels. Plus de `plat.h` | On suit l'arbre qu'on reprend, sans le réécrire. La parité wasm/natif revient avec l'émulateur (F0-03), mesurée sur les golden de SLOOP |
| ADR-044 | 2026-10-10 | **Le produit s'appelle FM80S**, avec le crédit de SLOOP (isod89) et de Felucca (Leo Kuroshita, Hügelton Instruments) à l'écran ABOUT et dans le README (*libellé revu en F0-01, en anglais : ADR-047*). Ni « SLOOP », ni « Felucca », ni « M-VAVE » comme nom de produit | « Felucca » et « Hügelton Instruments » sont des noms de Hügelton (`LICENSING.md` de SLOOP), « SLOOP » celui d'isod89 : un fork prend son propre nom |
| ADR-045 | 2026-10-10 | **Le code de P0-01 reste sous le tag `plateforme-trawler`.** On en reprend les idées et, pour l'émulateur, le worklet, le panneau, le clavier AZERTY et l'anti-cache `?v=` | P0-01 a servi : il a montré la plateforme web et l'écran des pads. SLOOP a déjà sa batterie |
| ADR-046 | 2026-10-10 | **F0-01, le build et le paquet :** `build.py` écrit `build/fm80s.fwsc` à côté de `felucca.fwsc` (que lisent les tests de SLOOP) ; le site local donne `fm80s-VER.fwsc`. **L'identité `FM-1_900` ne change pas.** Le SDK AC79 se réduit à ses 3 fichiers, téléchargés depuis gitee (empreintes vérifiées par `build.py`). `splash.c` et `gen_logo.py` de SLOOP restent dans l'arbre, mais `felucca.c` inclut `fm80s_splash.c`. Le budget CPU est recalé sur SLOOP 2.5 tel quel, sur le Mac d'Anthony | L'identité est vérifiée par l'installateur, le loader et `make_site.py` : la changer casserait l'installation. Garder les fichiers de SLOOP rend les fusions faciles (`ui_pages_test.c` inclut `splash.c`). Le budget : les comptes d'instructions dépendent de la machine, et `target_budget.txt` n'avait pas suivi SLOOP 2.4 et 2.5 (SLOOP 2.5 échouait déjà à son contrôle) |
| ADR-047 | 2026-10-10 | **L'interface est en anglais** (écrans du FM-1, démarrage, ABOUT, installateur, émulateur), comme celle de SLOOP ; la doc et les commits restent en français. Le crédit à l'écran : « by Floydus » et « inspired by SLOOP & Felucca » sous l'image de démarrage, « v0.1 » en haut à droite ; dans ABOUT, BY FLOYDUS, INSPIRED BY SLOOP & FELUCCA, ISOD89 / LEO KUROSHITA, HÜGELTON INSTRUMENTS, GPL-3.0, NO WARRANTY et l'adresse de notre source | Choix d'Anthony, sur la première capture (le français « d'après SLOOP et Felucca » est refusé). La GPL (§5 d) demande de garder les mentions légales dans l'interface : les crédits de SLOOP et de Felucca restent dans ABOUT |

**Ce que ces décisions remplacent :**
- **Remplacées** par le fork (ADR-036, ADR-043) : ADR-010, ADR-012 à ADR-016, ADR-021 à ADR-027, ADR-030 à ADR-033, ADR-035, et le tableau « Reprises de Trawler » ci-dessous. Elles décrivent le code gardé sous le tag `plateforme-trawler`.
- **À confronter à SLOOP en main** (après F0-01) : ADR-004 (punch-in sur les noires ; SLOOP : FX + touche blanche), ADR-005 (nos 4 moteurs ; SLOOP en a 12), ADR-006 (song ; SLOOP : 4 sections A–D), ADR-009 (mode facile), ADR-019 (potards sur le dernier son frappé), ADR-020, ADR-028.
- **Toujours valables** : ADR-001 à ADR-003, ADR-007, ADR-008, ADR-011, ADR-017, ADR-018, ADR-029 ; ADR-034 (thème) est repris en F0-01 et F2.

## Décisions de la plateforme Trawler (P0-01)

| ID | Date | Décision | Pourquoi |
| --- | --- | --- | --- |
| ADR-001 | 2026-10-10 | **Nom : FM80S** | « FM » est un terme générique (la synthèse FM). Ni « M-VAVE » ni « FM-1 » comme nom de produit (PRD §9.1) |
| ADR-002 | 2026-10-10 | **Nouveau dépôt** `y6zkcqnf5r-hue/fm1-fm80s`, Trawler archivé | Le tracker s'arrête, la plateforme se reprend (PRD §9.2, §9.10) |
| ADR-003 | 2026-10-10 | **La boucle :** longueur fixée par la première prise *ou* choisie d'avance ; **et un séquenceur pas à pas**, au moins pour la batterie | PRD §9.3 |
| ADR-004 | 2026-10-10 | **Punch-in :** une partie de l'écran FX ; en mode punch-in, les touches noires jouent les effets, les potards les règlent, et les noires ne jouent plus de dièses | PRD §9.4, §9.8 |
| ADR-005 | 2026-10-10 | **Moteurs, dans l'ordre :** Drums, VA (basse), Supersaw, FM | PRD §9.5 |
| ADR-006 | 2026-10-10 | **Une song complète :** des scènes enchaînées | PRD §9.6 |
| ADR-007 | 2026-10-10 | **Arpégiateur obligatoire** : modes classiques, repeat, gate, vitesse | PRD §9.7 |
| ADR-008 | 2026-10-10 | **Séquenceur :** batterie seule d'abord, extensible ; 16 pas par défaut, jusqu'à 128, pour tous les instruments | PRD §9.9 |
| ADR-009 | 2026-10-10 | **Gammes obligatoires** (note de base + gamme) et **mode facile** | PRD §9.11 |
| ADR-010 | 2026-10-10 | **Règles de la puce reprises de Trawler** : entiers seulement, pas de division 64 bits, pas de libc, mémoire statique, budgets du FM-1, parité wasm/natif, GPL-3.0-only | `CLAUDE.md` ; elles valent dès le navigateur |
| ADR-011 | 2026-10-10 | **Chaque story se termine par un essai d'Anthony**, et son avis, avant la suivante | La leçon de Trawler (PRD §8) |
| ADR-012 | 2026-10-10 | **On reprend Trawler par copie** du commit `153f4f4` : plateforme, DSP de base, écran, tests, outils, CI (liste : `05_ARCHITECTURE.md` §3). Renommés, origine dans `LICENSING.md`. On laisse le modèle du tracker, ses vues et sa voix | Trawler est archivé : un lien entre les dépôts n'apporterait rien. Tout ce qui est repris est déjà testé (golden, parité) |
| ADR-013 | 2026-10-10 | **Dossiers :** `app/`, `dsp/` (briques), `engine/` (un fichier par moteur), `core/` (horloge, pistes, looper), `ui/` | Un moteur = un fichier = une story ; le looper ne dépend d'aucun moteur |
| ADR-014 | 2026-10-10 | **Interface de moteur commune et petite** : `engine_t` (nom, taille d'état, son d'usine, `init`, `note_on`, `note_off`, `render`, `cost`) ; 128 octets de son par piste, de 0 à 255 ; l'état de chaque piste dans une union statique ; 16 niveaux publiés pour l'écran. **L'écran d'un moteur est dans `ui/`** | Ajouter un moteur ne touche ni au looper ni à la plateforme. Le moteur tourne côté audio : il ne dessine pas |
| ADR-015 | 2026-10-10 | **Frontière interface / audio de Trawler** : les notes par `cmdq`, les octets de son écrits par l'interface seule et relus à chaque sous-bloc | Éprouvée dans Trawler (ADR-021 et ADR-030 de Trawler) : les potards agissent en direct, sans verrou |
| ADR-016 | 2026-10-10 | **Drums v1 en synthèse pure**, une voix par son, sur les briques de Trawler (osc, svf, env) plus un bruit LFSR entier ; coût déclaré par voix | La batterie synthétique est dans le PRD ; pas de place pour des échantillons (flash ~560 Ko). Le LFSR est déterministe : la parité tient |
| ADR-017 | 2026-10-10 | **16 sons sur les 16 touches blanches** (kit de la spec §2.2) ; les noires ne jouent rien en P0-01 | Les blanches sont les pads naturels et les 16 pas du séquenceur : un pas = une touche = un son. Les noires restent pour le punch-in, le choix de piste et les accents. Au séquenceur, SEL + touche blanche choisira le son (à confirmer en P1-02) |
| ADR-018 | 2026-10-10 | **Une piste de batterie porte tout le kit** (16 sons, un moteur Drums) | On enregistre toute la batterie sur une piste ; il reste 7 pistes pour les synthés |
| ADR-019 | 2026-10-10 | **Les potards règlent le dernier son frappé** : TUNE, DECAY, TONE, LEVEL | On tape, on sculpte, sans menu : le geste le plus direct |
| ADR-020 | 2026-10-10 | **Coupures :** une voix par son ; rejouer un son coupe sa note précédente (fondu de 2 ms) ; le hat fermé coupe le hat ouvert | Le comportement d'une boîte à rythmes, et un coût CPU borné |
| ADR-021 | 2026-10-10 | **Écran de P0-01 :** 2 rangées de 8 pads dans l'ordre du clavier, animés par les niveaux de l'audio, et 4 jauges de potards. L'écran vivant de Drums vient plus tard | On voit tout de suite quel son on a joué, et sur quelle touche |
| ADR-022 | 2026-10-10 | **P0-02, le premier son sur le FM-1, dès réception de l'appareil** ; s'il n'est pas là après P0-01, P1-01 d'abord | La vitesse du processeur est le risque n° 1 (PRD §8) |
| ADR-023 | 2026-10-10 | **Tests :** un golden WAV par son de batterie (accepté après écoute), un scénario `drums.trs`, la parité wasm/natif ; format `.trs` inchangé | La règle « un nouveau moteur = un golden » de `CLAUDE.md` |
| ADR-024 | 2026-10-10 | **CI et Pages comme Trawler** : `make lint test web DOCKER=1` à chaque push et PR ; `main` publie le FM-1 virtuel sur https://y6zkcqnf5r-hue.github.io/fm1-fm80s/ | Anthony essaie aussi sur la page publiée |
| ADR-025 | 2026-10-10 | *Provisoire, à confirmer en P1.* **La boucle = des événements** (tick, note, vélocité, durée) tirés d'un pool commun ; le séquenceur pas à pas en est une vue sur la grille | Une seule représentation pour le jeu en direct et la programmation : on peut mélanger (PRD §3). Une piste vide ne coûte rien |
| ADR-026 | 2026-10-10 | *Provisoire, à confirmer en P1-01.* **Horloge à 96 ticks par noire**, un tick = 55125 / (2·bpm) échantillons, quotient et reste exacts (ADR-023 de Trawler) | 24 ticks par noire (Trawler) font 21 ms à 120 BPM : trop grossier pour enregistrer sans quantification. 96 en font 5 |
| ADR-027 | 2026-10-10 | *Provisoire, à revoir en P3.* **Budget mémoire** : événements 48 Kio, delay 130 Kio, réverbe ~32 Kio, soit ~210 Kio sur les 336 Kio du pool (spec §4) | Répond en partie à la question ouverte n° 3 du PRD |
| ADR-028 | 2026-10-10 | **Batterie en coups uniques** : relâcher la touche ne coupe pas le son | Le comportement d'une boîte à rythmes (P0-01, point 1) |
| ADR-029 | 2026-10-10 | **LED des touches à 3 paliers** : allumée tant qu'on tient la touche ou que le son est au-dessus du quart de son niveau, faible lueur tant qu'il résonne, éteinte ensuite. `plat.h` inchangé | La couche matérielle de Felucca n'a que 3 états par LED : « suivre le niveau » en continu est impossible (P0-01, point 2) |
| ADR-030 | 2026-10-10 | **Une seule piste en P0-01** (`ENG_TRACKS 1`) ; les pistes vivent dans `core/track.*` : moteur, octets de son, état dans une union | Pas de code à l'avance ; on passe à 8 en P2-02 (P0-01, point 10) |
| ADR-031 | 2026-10-10 | *Provisoire, à recaler en P0-02.* **CPU estimé** : `app_cost()` rend le coût déclaré ; sur l'hôte, 600 MAC par frame = 100 % (`CPU_REF_MAC`) ; la page montre le maximum de la dernière seconde | Une batterie est faite de pics. 600 est une hypothèse prudente tant que la vitesse de la puce est inconnue (P0-01, point 11) |
| ADR-032 | 2026-10-10 | **Un seul bruit pour le kit**, un xorshift 32 bits qui tourne en continu | Chaque coup est un peu différent, comme sur une machine analogique, et le rendu reste déterministe (parité) (P0-01, point 12) |
| ADR-033 | 2026-10-10 | **Une banque métallique par son** (hats, crash, ride) : 6 carrés naïfs aux fréquences classiques, puis un passe-bande et un passe-haut, et un gain par son après le filtre | TUNE accorde chaque métal seul ; les carrés naïfs ne coûtent presque rien. Un banc de carrés filtré vers 9 kHz sort ~30 dB sous un kick : le gain le remonte (P0-01, point 13) |
| ADR-034 | 2026-10-10 | *Provisoire.* **Thème et couleurs des potards** : fond nuit violette, 5 familles de pads (kicks magenta, caisses orange, métaux jaune, toms cyan, percussions violet) ; KNOB1–4 cyan, magenta, jaune, vert, à l'écran et sur le panneau | Les couleurs des vrais capuchons sont inconnues : on les recale à réception de l'appareil (P0-01, points 5 et 6) |
| ADR-035 | 2026-10-10 | **La voix de Trawler (`dsp/voice.*`) part en P0-01**, après avoir servi de preuve : dans le commit de reprise, ses 6 rendus de référence gardaient exactement les hashes de Trawler | La copie est fidèle (mêmes tables, mêmes flags) ; la voix, faite pour le tracker, ne sert plus |

## Reprises de Trawler (plateforme abandonnée)

Elles valaient pour la plateforme de P0-01. Avec le fork de SLOOP, **seules deux restent valables** : Emscripten 6.0.11 figé (pour l'émulateur, F0-02) et le nom **SEL** (SLOOP l'appelle aussi ainsi). Le clavier d'ordinateur (ADR-017) et le panneau (ADR-018) reviennent avec l'émulateur. Les autres sont remplacées par ADR-043. Leur détail est dans `../FM1/docs/08_DECISIONS.md`.

| Trawler | Décision |
| --- | --- |
| ADR-008, ADR-011 | Emscripten **6.0.11** figé, en Docker ou en emsdk local à la même version |
| ADR-009 | Entiers seulement, même si la puce a une FPU simple précision |
| ADR-012 | `app_audio` rend du Q15 dans un `int32`, saturé à 16 bits en sortie |
| ADR-013 | La LED rouge de PLAY (`plat_play_red`), allumée pendant REC |
| ADR-015 | Le 2ᵉ bouton s'appelle **SEL** partout |
| ADR-016 | `make test` construit toujours le wasm et vérifie la parité ; `PARITY=0` la saute en le disant |
| ADR-017 | Clavier d'ordinateur : les chiffres jouent les touches noires ; ⌥ + 1–8 tourne KNOB1–4 |
| ADR-018 | Panneau : clic droit, Ctrl-clic ou appui long de 0,5 s verrouille un bouton |
| ADR-020 | `make size` mesure des objets ELF 32 bits compilés par clang |
| ADR-023 | Horloge en échantillons, quotient et reste : aucune dérive (adaptée à 96 ticks par noire, ADR-026) |
| ADR-026 | Le SVF trapézoïdal, les oscillateurs polyBLEP, l'enveloppe Q24 par sous-blocs de 16 |
| ADR-027 | Les polices Terminus 6×12, 6×10, 8×12 (le thème change) |

## Questions ouvertes

| ID | Question | Qui tranche | Statut |
| --- | --- | --- | --- |
| Q-01 | Scènes et song : 16 scènes (PRD §3) ou les 4 sections A–D de SLOOP ? | Anthony, après avoir joué SLOOP | Rouverte par le fork (ADR-036) ; plus tard |
| Q-02 | Mode facile : les blanches jouent les degrés de la gamme, ou le clavier du piano ramené sur la gamme ? SLOOP a déjà « la gamme sur les blanches » : suffit-elle ? | Anthony, sur l'appareil | Après F0-01 |
| Q-03 | La mémoire : 8 pistes, effets compris | Spec | Étudiée : `10_FAISABILITE_8_PISTES.md`, ADR-039, ADR-040 ; à mesurer en F0-01 et F1-01 |
| Q-04 | Le pas des potards | Anthony, à l'essai | Celui de SLOOP, jusqu'à l'essai en main |
| Q-05 | Au séquenceur, SEL + touche blanche pour choisir le son à programmer ? | Anthony, à l'essai | Remplacée par la façon de SLOOP (page de la grille de batterie), à juger après F0-01 |
| Q-06 | La taille du projet sauvegardé | Spec | Remplacée par Q-11 et F1-03 (format `FUN6`) |
| Q-07 | Les calques à tenir de SLOOP (FX, SEQ, SEL, GLO, SAVE + touche) sont-ils assez immédiats, au sens du PRD ? | Anthony | Après F0-01 |
| Q-08 | Punch-in : garder FX + touche blanche de SLOOP (16 effets) ou passer aux touches noires (PRD, ADR-004) ? | Anthony | Après F0-01 |
| Q-09 | Quels moteurs comptent comme « légers » pour les pistes 4 à 7 (ADR-039) ? | Spec, mesures à l'appui | En F1-01 |
| Q-10 | Le RAM tient-il les ~17 Ko des 8 pistes, ou faut-il déplacer des tables dans le pool ? | Mesure | **Mesuré en F0-01 : non**, 13,2 Kio libres pour ~17 Ko. On déplace `proj_tmp` et des tables par partie dans le pool en F1-01 (`10_FAISABILITE_8_PISTES.md` §3) |
| Q-11 | Les 4 sections à 8 pistes : en flash, en format creux, ou avec moins de verrous ? (`10_FAISABILITE_8_PISTES.md` §4) | Spec, mesures à l'appui | En F1-01, finie en F1-03 |
| Q-12 | Garder `assets/icons.png` de SLOOP (« tous droits réservés », permission GPL §7 de Hügelton) ou construire avec `FELUCCA_ICONS=0` et dessiner nos icônes ? | Anthony | En F0-01 : on garde le réglage de SLOOP, on tranche en F2 |
