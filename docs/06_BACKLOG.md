# 06 — Backlog de FM80S

> v0.2, 2026-10-10 : FM80S repart d'un fork de SLOOP 2.5 à 8 pistes (ADR-036 à ADR-045). Seule F0-01 est détaillée ; les autres stories tiennent en quelques lignes et se précisent quand on les commence, après l'avis d'Anthony sur la précédente.
>
> Le backlog v0.1 (plateforme Trawler) s'arrête à **P0-01, faite** (la batterie au clavier dans le navigateur) ; son code est sous le tag `plateforme-trawler`.

**Règles communes :**
- On fait une story à la fois, en mode plan d'abord.
- Une story est finie quand :
  - `./build.sh` et `tests/run_tests.sh` sont au vert (plus, dès F0-02, le build et les tests de l'émulateur) ;
  - le commit cite l'ID de la story (ex. `F0-01: image de démarrage`) ;
  - **Anthony l'a essayée (sur le FM-1, puis aussi dans le navigateur dès F0-02) et a donné son avis.** On ne commence pas la suivante avant.
- On garde les golden de SLOOP identiques, sauf changement de son voulu et écouté.

## Jalon F0 — FM80S tourne

### F0-01 — FM80S sur le FM-1

> **État (2026-10-10) :** le dépôt, le build sur le Mac, le nom, l'image de démarrage (validée), ABOUT (validée), le README, la licence et les mesures sont faits ; tests verts, golden inchangés. **Reste, à l'arrivée du FM-1 :** l'installation, l'essai et ton avis.

**Ce qu'on fait :**
- Le dépôt (ADR-037) : tag `plateforme-trawler` ; main repart de l'historique de SLOOP 2.5, plus nos docs et notre `CLAUDE.md` ; remote `sloop`. **Le push forcé de main t'est demandé avant.**
- Le build sur ton Mac : Docker Desktop, la chaîne JieLi, les 3 fichiers du SDK AC79 (on te guide pas à pas). Première mesure de la RAM, du pool et du code, notée dans `10_FAISABILITE_8_PISTES.md` (Q-10).
- Le nom FM80S à l'écran et sur le fichier installé, le crédit « by Floydus » / « inspired by SLOOP & Felucca », interface en anglais (spec §1, ADR-044, ADR-047).
- L'image de démarrage synthwave : un script la génère au format de SLOOP (240×188, 16 couleurs) ; **tu la valides sur un PNG avant qu'on construise.**
- `README.md` et `LICENSING.md` réécrits pour FM80S.

**Tu essaies :**
- tu installes FM80S depuis Chrome (pas à pas ; pour revenir en arrière : la même page avec le firmware d'origine) ;
- tu vois l'image de démarrage et le nom ;
- tu suis « Un beat en soixante secondes » de `DEMARRAGE-RAPIDE-FR.md` : batterie en prise libre, overdub, une basse, la gamme avec SEL, un punch-in avec FX ; puis le kit *synthwave* et un lead supersaw.

**Ton avis, à noter dans `08_DECISIONS.md` :** les calques à tenir te vont-ils (Q-07) ? Le punch-in sur les blanches (Q-08) ? Les 4 sections (Q-01) ? La gamme sur les blanches (Q-02) ? Que manque-t-il le plus ?

**Fait quand :** le firmware se construit sur ton Mac, les tests de SLOOP passent sans changer un golden, FM80S tourne sur ton FM-1, et tu as donné ton avis.

### F0-02 — Le FM-1 virtuel : le son, les touches, l'écran

Le vrai firmware en wasm dans Safari (architecture §5) : les touches, les boutons et les potards au clavier AZERTY et au panneau de P0-01, l'écran, le son. **Tu essaies** la même boucle que sur l'appareil, dans le navigateur, et tu compares.

### F0-03 — Le FM-1 virtuel fidèle et publié

La parité wasm/natif sur les rendus de `tests/golden.txt`, les projets gardés dans le navigateur, les LED, la CI (build du firmware, tests, émulateur) et la page publiée. **Tu essaies** la page publiée.

## Jalon F1 — 8 pistes (`10_FAISABILITE_8_PISTES.md`)

- **F1-01 — 8 pistes sous le capot :** `NPART 7`, `NTRK 8`, `TRK_DRUM 7` ; moteurs lourds sur les pistes 1–3 (ADR-039, Q-09) ; slicer partagé (ADR-040) ; les vrais bugs à 8 (faisabilité §7) ; le vol de voix à 7 parties ; mesures de la RAM, du pool et du CPU, sur le FM-1 ; le choix pour les sections (Q-11), et une solution provisoire pour qu'elles marchent. L'écran reste celui de SLOOP : on choisit les pistes 5–8 au potard ALGORITHM. **Tu essaies** : 8 pistes qui jouent ensemble, dans l'émulateur et sur le FM-1, et on lit le CPU.
- **F1-02 — 8 pistes à l'écran et sous les doigts :** écran TRACKS à 8, 8 couleurs, choix de piste à SEL + touche noire, mute / solo / niveaux à 8 dans GLO (spec §3.2). **Tu choisis** la disposition de l'écran sur deux captures, puis tu joues.
- **F1-03 — Les projets à 8 pistes :** format `FUN6`, conversion des projets de SLOOP, carte de la flash, sections ; l'éditeur web et son protocole. **Tu essaies** : sauvegarder, éteindre, rallumer, recharger ; ouvrir un projet de SLOOP.

## Jalon F2 — L'identité synthwave

- **F2-01 — Le thème FM80S :** palette d'écran, couleurs, icônes (Q-12).
- **F2-02 — Des sons synthwave :** une banque de presets et un kit, à partir des moteurs de SLOOP.
- **F2-03 — Les écrans vivants :** un écran animé par moteur, qui montre ce que font les potards.
- **F2-04 — La réverbe gated**, signature de la caisse claire synthwave.

## Plus tard, à trier après avoir joué SLOOP

16 scènes au lieu de 4 sections (Q-01) ; le punch-in sur les touches noires (Q-08) ; le mode facile du PRD (Q-02) ; un moteur VA ou Supersaw à nous si ceux de SLOOP ne suffisent pas ; l'avenir de l'éditeur web ; suivre les nouvelles versions de SLOOP.
