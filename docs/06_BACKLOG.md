# 06 — Backlog de FM80S

> v0.1, 2026-10-10. Court à dessein : seules P0-01 et P0-02 sont détaillées. Les autres stories tiennent en une ligne ; on les précise quand on les commence, après l'avis d'Anthony sur la précédente.

**Règles communes :**
- On fait une story à la fois, en mode plan d'abord.
- Une story est finie quand :
  - `make lint test web` est au vert ;
  - le commit cite l'ID de la story (ex. `P0-01: batterie au clavier`) ;
  - **Anthony l'a essayée dans le navigateur (puis sur l'appareil) et a donné son avis.** On ne commence pas la suivante avant.
- Chaque nouveau moteur arrive avec son golden et son écran.

## Jalon P0 — Ça sonne

### P0-01 — La batterie au clavier, dans le navigateur

**Ce qu'on fait :**
- On reprend la plateforme de Trawler (`05_ARCHITECTURE.md` §3), avec le renommage et `LICENSING.md`.
- On écrit l'interface de moteur (`engine.h`) et le moteur Drums v1 : les 16 sons de la spec §2.2, et le bruit LFSR.
- Les touches blanches jouent les sons ; les potards règlent le dernier son frappé ; MASTER est le volume.
- On dessine l'écran des 16 pads (spec §5), et les LED suivent les sons.
- La page renommée affiche la jauge CPU estimé.
- On met en place la CI et GitHub Pages pour `fm1-fm80s`.

**Tu essaies :**
- `make serve`, puis http://localhost:8080 dans Safari ;
- tu joues un rythme sur `W X C V B N ,` (sons 1–7) et `A Z E R T Y U I O` (sons 8–16) en AZERTY, ou au panneau ;
- tu tournes les potards avec ⌥ + 1…8 ou à la souris ;
- ensuite, la même chose sur la page publiée.

**Fait quand :**
- les 16 sons sont jouables et réglables ;
- le hat fermé coupe le hat ouvert ;
- les golden `drums` (un WAV par son, écoutés) et le scénario `drums.trs` passent, avec la parité wasm/natif ;
- `make size` est sous les budgets ;
- la CI est verte et la page est publiée.

### P0-02 — Le premier son sur le FM-1

**Ce qu'on fait :**
- `plat_fm1.c` minimal sur la couche matérielle de Felucca : l'audio, les touches, les potards, l'écran et les LED. Le stockage attendra.
- La batterie de P0-01 jouée aux touches de l'appareil.
- Le CPU mesuré (`fm1_ticks()` autour de `app_audio`) affiché à l'écran.
- On recale le modèle de coût sur cette mesure, et on fixe le nombre de voix par moteur.

**Tu essaies :**
- tu flashes l'appareil (on te guide pas à pas) et tu joues ;
- on note le CPU avec un son, puis avec les 16 sons frappés d'un coup.

**Quand :** dès réception du FM-1 (semaine du 2026-10-12). Si l'appareil n'est pas encore là après P0-01, on fait d'abord P1-01 (ADR-022).

## Jalon P1 — Ça boucle (la batterie seule)

- **P1-01 — Le transport :** PLAY / STOP, le tempo (GLO), le métronome, la longueur choisie d'avance (1, 2, 4 ou 8 mesures), l'horloge à 96 ticks par noire (spec §3).
- **P1-02 — Le séquenceur de batterie :** SEQ ; les 16 touches blanches sont les pas et les LED montrent le motif ; SEL + touche blanche choisit le son ; de 16 à 128 pas, par pages.
- **P1-03 — Enregistrer en direct :** REC, overdub, quantification (aucune, 1/16 ou 1/8), annuler la dernière prise, effacer, mélanger avec les pas. → *une vraie boucle de batterie, jouée et programmée.*

## Jalon P2 — Plusieurs pistes

- **P2-01 — Le moteur VA (basse)**, avec son golden et son écran vivant.
- **P2-02 — 8 pistes :** SEL + touche noire choisit la piste ; un moteur par piste ; mute ; la longueur fixée par la première prise.
- **P2-03 — Les gammes et le mode facile.**
- **P2-04 — L'arpégiateur :** les modes classiques, repeat, gate, vitesse.

## Jalon P3 — Le son

- **P3-01 — Les effets :** filtre et saturation par piste ; delay synchronisé et réverbe (dont la gated) en envoi ; saturation façon bande et limiteur sur le master.
- **P3-02 — Le punch-in :** FX + les touches noires.
- **P3-03 — Le moteur Supersaw.**
- **P3-04 — Le moteur FM à 4 opérateurs.**

## Jalon P4 — Le morceau

- **P4-01 — Les 16 scènes**, avec le changement à la fin de la boucle.
- **P4-02 — La song :** une liste de 64 entrées, qu'on écrit ou qu'on enregistre en direct.
- **P4-03 — Les sons et la sauvegarde :** presets par moteur, projets.

## Plus tard (non ordonné)

L'écran vivant propre à Drums ; SEL + potard (les 4 paramètres suivants) ; les accents ; d'autres kits ; les moteurs suivants du PRD §3.
