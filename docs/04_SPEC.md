# 04 — Spec minimale de FM80S

> v0.1, 2026-10-10. Elle couvre ce qu'il faut pour P0-01 (la batterie au clavier) et pose les grandes lignes de la suite, **sans les figer** : ce qui est marqué *provisoire* se tranche dans sa story, à l'essai. Vision : `03_PRD.md`. Décisions : `08_DECISIONS.md`.

## 1. Les contrôles

### 1.1 Intention générale (PRD §4, à essayer sur l'appareil)

| Contrôle | Rôle visé | Arrive en |
| --- | --- | --- |
| 16 touches blanches | Jouer ; en batterie, un son par touche ; au séquenceur, les 16 pas | P0-01, P1-02 |
| 11 touches noires | Les dièses ; en mode punch-in, les effets ; SEL + noire choisit la piste | P2-02, P3-02 |
| KNOB1–4 | Les 4 paramètres de l'écran ; SEL + potard, les 4 suivants | P0-01 |
| MASTER | Volume général | P0-01 |
| PLAY / REC | Lancer et arrêter / enregistrer, overdub | P1-01, P1-03 |
| SEQ | Le séquenceur pas à pas | P1-02 |
| GLO | Réglages globaux : tempo, métronome, gamme | P1-01 |
| ENV, LFO, FX, ARP, HOME | Les écrans : enveloppes, LFO, effets et punch-in, arpégiateur, moteur | P2, P3 |
| SELECT / ALGORITHM / PRESETS | Choisir le moteur, la piste, le son | P2-02, P4-03 |
| SAVE | Sauvegarder | P4-03 |

### 1.2 L'écran Drums en P0-01 (le seul détaillé ici)

| Geste | Effet |
| --- | --- |
| Touche blanche n (1 à 16, de F3 à G5) | Joue le son n (§2.2) **et le choisit** pour les potards |
| KNOB1 / KNOB2 / KNOB3 / KNOB4 | Hauteur / longueur / couleur / volume du dernier son frappé, en direct sur le son qui joue |
| MASTER | Volume général |
| Touches noires, autres boutons | Rien en P0-01 |

- **Potards :** la valeur va de 0 à 255. Chaque cran ajoute ±2, et ±8 quand on tourne vite (crans espacés de moins de 40 ms). *[À régler à l'essai : constantes `UI_KNOB_*` de `ui/ui.h`.]* Au démarrage, ils règlent le kick long.
- **LED :** le FM-1 n'a que trois états par LED (éteinte, faible lueur, allumée). Une touche blanche est allumée tant qu'on la tient ou que son son est au-dessus du quart de son niveau, en faible lueur tant qu'il résonne encore, éteinte quand il est fini (ADR-029). Le pad à l'écran, lui, suit le niveau par paliers de 6 dB.
- **Coup unique :** relâcher la touche ne coupe pas le son, qui va au bout de sa décroissance (ADR-028).
- **Pas de vélocité :** le FM-1 n'en a pas. Chaque coup a la même force ; les accents viendront plus tard.

## 2. Les moteurs

### 2.1 L'interface commune

- **Un moteur par piste.** Le moteur reçoit les notes, rend le son de sa piste et déclare son coût en CPU. Sa forme en C est dans `05_ARCHITECTURE.md` §4.
- **Le son d'une piste :** jusqu'à 128 octets de paramètres, chacun de 0 à 255. Un synthé utilise 8 paramètres : KNOB1–4, puis SEL + KNOB1–4. La batterie utilise 8 paramètres par son, pour ses 16 sons.
- **Ce que l'écran voit :** l'audio publie 16 niveaux (0 à 255), un par voix ou par son. L'écran et les LED s'animent avec ces niveaux, sans calcul de plus.
- **Pas d'écran dans le moteur :** l'écran d'un moteur est dans `ui/`, à part, parce que le moteur tourne du côté de l'audio.

### 2.2 Le moteur Drums v1

**Le kit.** Une piste de batterie porte les 16 sons (ADR-018). Synthèse pure, sans échantillon. Les sons, dans l'ordre des touches blanches :

| Touche | Son | Synthèse (première idée) |
| --- | --- | --- |
| 1 (F3) | Kick long | Sinus, enveloppe de hauteur lente, clic d'attaque, longue décroissance (le « boum » 808) |
| 2 (G3) | Kick court | Idem, plus tendu et plus court |
| 3 (A3) | Snare | Deux sinus accordés et du bruit filtré en passe-bande |
| 4 (B3) | Snare 2 | Plus sèche, plus de bruit (pour la réverbe gated, P3-01) |
| 5 (C4) | Clap | Trois ou quatre rafales de bruit rapprochées, puis une queue, en passe-bande |
| 6 (D4) | Rim | Deux sinus aigus très courts et un clic |
| 7 (E4) | Hat fermé | Banque « métallique » de carrés désaccordés, passe-bande et passe-haut, très court ; **coupe le hat ouvert** |
| 8 (F4) | Hat ouvert | Même banque, plus longue |
| 9 (G4) | Tom grave | Sinus, enveloppe de hauteur marquée (toms électroniques années 80) |
| 10 (A4) | Tom médium | Idem, plus haut |
| 11 (B4) | Tom aigu | Idem, plus haut |
| 12 (C5) | Cowbell | Deux carrés (rapport d'environ 1,5), passe-bande |
| 13 (D5) | Crash | Banque métallique et bruit, longue décroissance |
| 14 (E5) | Ride | Banque métallique, plus tonale, décroissance moyenne |
| 15 (F5) | Shaker | Bruit en passe-haut, attaque douce, court |
| 16 (G5) | Zap | Sinus ou triangle, chute de hauteur très rapide |

**Les 4 paramètres de chaque son :**
- **TUNE**, la hauteur : ±1 octave autour du réglage d'usine ;
- **DECAY**, la longueur ;
- **TONE**, la couleur : la coupure du filtre ou la part de bruit, selon le son, de plus sourd à plus brillant et claquant ;
- **LEVEL**, le volume (0x80 = 0 dB).

Les octets 4 à 7 de chaque son sont réservés : panoramique, saturation, etc., sous SEL + potard, plus tard. Chaque son a ses valeurs d'usine, dans une table qu'on retouche à l'oreille.

**Les voix et les coupures (ADR-020) :**
- une voix par son : les 16 sons peuvent sonner ensemble ;
- rejouer un son coupe sa note précédente, avec un fondu de 2 ms, comme la voix de Trawler ;
- le hat fermé coupe le hat ouvert.

**Le coût (déclaré, pas mesuré) :** de 5 à 14 MAC par frame et par son actif, selon le son. Les 16 sons frappés d'un coup déclarent 135 MAC par frame, soit 22 % de la référence (ADR-031). À recaler en P0-02 avec la mesure sur l'appareil. Chaque métal a sa propre banque de 6 carrés, pour que TUNE l'accorde seul (ADR-033).

**Le moteur tel qu'écrit (P0-01) :** `firmware/src/engine/drums.c`. Chaque son suit une recette (un modèle parmi huit, ses hauteurs, ses plages de DECAY et de TONE, son filtre, son gain), et les quatre potards se relisent tous les 16 frames. Les réglages d'usine sont une première passe, à reprendre à l'oreille.

## 3. La boucle (esquisse, *provisoire* : à confirmer en P1)

- **Une seule représentation :** une piste d'une scène est une liste d'**événements** (le tick, la touche ou la note, la vélocité, la durée).
  - Le jeu en direct en ajoute (REC).
  - Le séquenceur pas à pas est une **vue** de ces mêmes événements, calés sur la grille : allumer un pas crée un événement à ce tick, l'éteindre l'efface.
  - On peut donc mélanger ce qu'on joue et ce qu'on programme (PRD §3).
- **Les événements viennent d'un pool commun** à toutes les scènes et à toutes les pistes, pas de tableaux fixes 8 × 16 × 128 : une piste vide ne coûte rien.
- **L'horloge :** 96 ticks par noire, soit 5 ms à 120 BPM, assez fin pour garder le groove sans quantification. Un tick dure 55125 / (2·bpm) échantillons, avec le quotient et le reste exacts de Trawler (ADR-023 de Trawler) : aucune dérive. Une boucle de 8 mesures fait 3072 ticks, et tient sur 16 bits.
- **La longueur :** de 1 à 8 mesures, soit 16 à 128 pas de double croche. Elle est choisie d'avance, ou fixée par la première prise et arrondie à la mesure (PRD §9.3).
- **Le moteur reçoit les événements** par la même entrée que les touches (`note_on`, `note_off`).

## 4. Le budget mémoire (estimation, *provisoire* : à revoir en P3)

| Quoi | Taille | Où |
| --- | --- | --- |
| Événements : 8192 × 6 o (tick 16 bits, note, vélocité, durée 16 bits). Assez pour ~512 notes par scène sur 16 scènes | 48 Kio | pool |
| Delay : 1,5 s mono 16 bits à 44,1 kHz | 130 Kio | pool |
| Réverbe | ~32 Kio | pool |
| **Total pool** | **~210 Kio sur 336** | |
| Voix et état des moteurs : 8 pistes, au plus ~2 Kio chacune | ~16 Kio | RAM |
| Sons des pistes : 8 × 128 o | 1 Kio | RAM |
| Index des scènes, transport, interface | < 4 Kio | RAM |

Le projet sauvegardé (événements, sons, scènes, song) dépassera les 48 Kio que Trawler s'était fixés. La zone de données de la flash fait ~292 Kio (0x97000–0xE0000, voir `../FM1/docs/05_ARCHITECTURE.md` §8). La taille d'un projet et le nombre de projets se chiffrent en P4-03.

## 5. L'écran de P0-01

Écran de 240 × 240, dessiné avec `ui/gfx` (Terminus). On ne redessine que ce qui change.

- **En-tête :** `FM80S`, le nom du moteur (`DRUMS`) et le nom du dernier son frappé (`KICK L`, `SNARE`…).
- **Les pads :** 2 rangées de 8, dans l'ordre du clavier, les touches 1 à 8 en bas et 9 à 16 en haut.
  - Chaque pad a la couleur de sa famille : kicks, caisses et clap, métaux, toms, percussions.
  - Il s'allume quand on frappe le son, et s'éteint au rythme de sa décroissance (les niveaux publiés par l'audio, §2.1).
  - Le dernier son frappé a un contour blanc.
- **Le pied :** 4 jauges, aux couleurs de KNOB1–4, pour TUNE, DECAY, TONE et LEVEL du dernier son frappé.
- **Le thème :** maison, couleur synthwave (nuit violette, magenta, cyan), défini dans P0-01. Il ne reprend aucune couleur ni aucun graphisme d'un autre produit (règle 9).
- **L'écran « vivant » propre à Drums** (pads qui rebondissent, forme du son…) viendra dans une story à part, une fois le looper en place.
