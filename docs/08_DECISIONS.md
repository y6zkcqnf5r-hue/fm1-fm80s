# 08 — Décisions et questions ouvertes

## Décisions prises

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

## Reprises de Trawler, toujours valables

Elles s'appliquent telles quelles. Leur détail est dans `../FM1/docs/08_DECISIONS.md`.

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
| Q-01 | Scènes et song : 16 scènes, changement à la fin de la boucle, liste de 64 entrées (PRD §3) | Anthony, à l'essai | En P4 |
| Q-02 | Mode facile : les blanches jouent les degrés de la gamme, ou le clavier du piano ramené sur la gamme ? Gamme par projet ou par scène ? | Anthony, sur l'appareil | En P2-03 |
| Q-03 | La mémoire : 8 pistes × 16 scènes × 128 pas, effets compris | Spec | Estimée (ADR-027), à revoir en P3 |
| Q-04 | Le pas des potards : ±2 par cran, ±8 en tournant vite ? | Anthony, à l'essai | Codé en P0-01 tel que proposé (`UI_KNOB_*`), à confirmer à l'essai |
| Q-05 | Au séquenceur, SEL + touche blanche pour choisir le son à programmer ? | Anthony, à l'essai | En P1-02 |
| Q-06 | Le projet sauvegardé dépassera 48 Kio : taille et nombre de projets dans la zone de données (~292 Kio) | Spec | En P4-03 |
