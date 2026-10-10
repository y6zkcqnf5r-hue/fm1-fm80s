# 09 — FM80S face à Groove OS, SLOOP et sloopDX

> 2026-10-10, après P0-01. Sources : le site de Groove OS (version 1.2.1), les README de SLOOP (2.5) et de sloopDX (3.4), le catalogue [aroum/fm1-custom-fw](https://github.com/aroum/fm1-custom-fw). Rien n'a encore été essayé à la main : les impressions de jeu viendront de tes essais (§4). But : décider si la direction de FM80S change, **avant P0-02**.

## 1. En une phrase

**Ce que vise le PRD de FM80S existe déjà en grande partie :**
- SLOOP (libre, GPL) est un looper-groovebox joué en direct, avec une batterie de 16 sons sur les touches blanches, des effets punch-in, les gammes, un « mode facile » et des sections enchaînées ;
- Groove OS (payant) a 8 pistes de 1 à 8 mesures, des scènes et un supersaw.

Ce qui reste propre à FM80S est **la façon de jouer** (l'immédiateté façon OP-1, un écran vivant par moteur), pas la liste des fonctions.

## 2. Les fonctions, face au PRD

| PRD de FM80S | Groove OS 1.2.1 | SLOOP 2.5 | sloopDX 3.4 |
| --- | --- | --- | --- |
| Licence, prix | Fermé, 29 $ | **GPL-3.0, gratuit** | GPL-3.0, gratuit (fork de SLOOP) |
| Base | Le firmware V15 de M-VAVE | Felucca | SLOOP |
| Essai dans le navigateur | **Oui**, émulateur complet | Non (éditeur web, et l'appareil) | Non |
| Sur un vrai FM-1 | Oui, version « complète » | Oui, bêta très suivie (369 ★) | Oui, bêta |
| **8 pistes** | **8**, chacune 1–8 mesures | 4 (3 synthés + batterie) | 4 |
| Looper : jouer, ça tourne, overdub | Oui, avec décompte et clic | **Oui, sans clic** ; prise libre : la boucle fixe le tempo | Oui (celui de SLOOP) |
| Longueur par la 1re prise ou d'avance | 1–8 mesures d'avance | **Les deux** (libre, ou 1, 2, 4 mesures) | Les deux |
| Annuler, effacer | 16 annulations par piste | Annuler / rétablir, effacer en tenant REC | Idem |
| Séquenceur pas à pas, 16 → 128 pas | 64 pas, verrous de paramètre | 64 pas, verrous, micro-décalage, fills | **128 pas** |
| Batterie : 16 sons sur les blanches | Non : 8 pads, les blanches sont les pas | **Oui**, 37 kits dont synthwave | **Oui**, FM, réglable (TUNE, DECAY…) |
| 16 scènes, song | 11 scènes sur les noires | 4 sections A–D, chaîne, song enregistrée | Idem |
| Effets punch-in | 8 | **16**, FX + une touche blanche | Oui |
| Arpégiateur obligatoire, mode repeat | 5 modes, 1–4 octaves | Arp et note repeat (ARP tenu) | Idem |
| Gammes et mode facile | 12 gammes, toujours dans la tonalité | **16 gammes, « la gamme sur les blanches »**, accords sur une touche | Idem |
| Moteurs : Drums, VA, Supersaw, FM | FM 6 op. + VA avec SUPER (7 voix) | 12 moteurs : analogique, FM 4 et 6 op., CZ, granulaire, modèles physiques… ; un lead supersaw | DX7 seul (Dexed) |
| Orientation synthwave | Un pack de sons parmi 11 | Kit synthwave, sons « 80s poly », string machine | Non |
| Écran vivant par moteur | Courbes d'automation, vue scène | « Écran à la teenage engineering », visualiseur 12 styles | Dessin des opérateurs |
| Extras | MIDI, clock, DX7 SysEx | Audio USB, MIDI IN jack, clock, samples, éditeur web | Édition DX7 sur l'appareil, OMNI |

Pour mémoire, une **bêta synthwave** basée sur Felucca circule aussi (la vidéo YouTube) : des sons façon Juno, Jupiter et M1, 8 pistes d'enregistrement, de grosses polices. Je n'ai pas trouvé son dépôt.

## 3. Ce qui resterait propre à FM80S

1. **8 pistes de notes, libres.** Seul Groove OS les a, et il est fermé et payant. SLOOP en a 4, et sloopDX utilise déjà ~85 Kio de RAM sur 96 : passer à 8 pistes dans une base SLOOP serait dur.
2. **L'immédiateté façon OP-1.** SLOOP passe beaucoup par des calques à tenir (FX, SEQ, SEL, GLO, SAVE + touche). C'est puissant, mais c'est à juger en main : est-ce « ludique » au sens de ton PRD, ou déjà un peu « OP-XY » ?
3. **Une identité visuelle forte par moteur**, et un son synthwave assumé.
4. **16 scènes** au lieu de 4 sections.
5. **Ton propre instrument**, que tu façonnes comme tu veux. C'est une vraie raison, mais elle coûte des mois. À ce jour, FM80S a une batterie et pas de boucle.

## 4. Les essais à faire (≈ 30 min)

### 4.1 Groove OS, maintenant, dans le navigateur

- https://www.groove-os.com/emu. Le site ne cite que Chrome et Edge pour l'installation : si Safari bloque, prends Chrome.
- Clavier de l'émulateur :
  - la rangée du milieu (A → ;) joue les touches blanches, celle du dessus les noires ;
  - 1 à 8 tournent K1–K4 ;
  - Espace : PLAY ; R : REC ;
  - C V B N M , : ARP, SEQ, FX, EDIT, GLO, LFO.
- **À faire :**
  1. Ouvre une chanson de démo (FIRST GROOVE), écoute, coupe des pistes et lance une scène.
  2. Repars d'EMPTY : fais une boucle de batterie, puis une basse par-dessus.
- **À noter :** combien d'appuis pour la 1re boucle ? Le son te donne-t-il envie ? Te sens-tu perdu ?

### 4.2 SLOOP, sur le FM-1 dès son arrivée (l'installation s'annule)

- Installation dans **Chrome ou Edge** : https://isod89.github.io/sloop-fm1/, câble USB de données, INSTALL. Pour revenir au firmware d'origine : la même page, avec le fichier V15 de M-VAVE.
- Guide en français : `DEMARRAGE-RAPIDE-FR.md` du dépôt isod89/sloop-fm1.
- **À faire :**
  1. Suis « Un beat en soixante secondes » : batterie en prise libre, overdub, une basse, la gamme avec SEL, un punch-in avec FX.
  2. Choisis le kit *synthwave* et un lead supersaw.
- **À noter :** les calques à tenir te vont-ils sous les doigts ? Manque-t-il quelque chose d'important (8 pistes, scènes, sons) ?

sloopDX n'est utile que si tu veux une batterie FM et un DX7. On peut le sauter.

## 5. Les options

| Option | Ce que c'est | Pour | Contre |
| --- | --- | --- | --- |
| **A. Continuer FM80S tel quel** | Le PRD et le backlog, sur notre plateforme | Ton instrument, en entier ; le code est propre et testé | Des mois avant d'atteindre SLOOP ; la plateforme n'a encore jamais tourné sur un FM-1 |
| **B. Recentrer FM80S** | On garde notre plateforme, mais un périmètre plus étroit et distinct : par exemple 8 pistes de notes, 3 moteurs synthwave, des écrans vivants, et rien d'autre | Moins long ; une identité nette | Reste plus long que C pour un premier son sur l'appareil |
| **C. FM80S devient un fork de SLOOP** (comme sloopDX) | On part de SLOOP (GPL) et on y ajoute l'identité FM80S : moteurs et sons synthwave, écrans vivants, ton ergonomie | Tout marche déjà sur l'appareil (CPU, flash, installateur) ; on travaille sur ce qui te distingue dès la 1re story | On hérite de ses choix (4 pistes, calques, RAM presque pleine) ; on abandonne la plateforme de Trawler et de P0-01 |
| **D. Jouer plutôt que construire** | Utiliser SLOOP (ou Groove OS, 29 $), et garder le développement pour plus tard | La musique tout de suite | Plus de projet |

## 6. Recommandation

**Ne décide pas sur le papier : fais les deux essais du §4, puis on tranche avec ces trois questions.**

1. **Le jeu en main de SLOOP te plaît-il ?** Si oui, **C** est le meilleur rapport effort / résultat. On garde de P0-01 les idées (le kit, l'écran des pads) et on les porte dans SLOOP.
2. **Les 8 pistes ou les 16 scènes sont-elles indispensables pour toi ?** Si oui, et que Groove OS te convient, D vaut la peine d'être regardée. Sinon, **B**, puisque SLOOP ne les aura pas.
3. **Construire ton instrument fait-il partie du plaisir, indépendamment du résultat ?** Si oui, **B** plutôt que A : un périmètre plus court et à toi.

En attendant ta réponse, je ne commence pas P0-02. Si tu choisis A ou B, P0-02 reste la prochaine story. Si tu choisis C, on réécrit le backlog autour de SLOOP.
