# PRD v0.2 — FM80S, un synthé-looper ludique pour le FM-1

> v0.1 relue avec Anthony le 2026-10-10 ; elle remplaçait le tracker Trawler, arrêté le même jour (l'ergonomie du FM-1 ne convient pas à un tracker). **v0.2, le même jour : FM80S devient un fork de SLOOP** (`09_COMPARAISON.md`, ADR-036), avec deux différences majeures, **8 pistes** et **une identité synthwave forte**, puis le reste de ce PRD. Ce qui change est au §2 bis ; les fonctions que SLOOP fait déjà autrement sont marquées *« à confronter à SLOOP »*.

## 1. La vision

> **FM80S est un synthé de poche qu'on joue au clavier et qu'on empile en boucles : on joue une boucle, elle tourne, on en joue une deuxième avec un autre instrument par-dessus, et ainsi de suite ; ou on la programme pas à pas, au séquenceur (la batterie, au minimum). Les boucles s'enchaînent en une song complète. Une dizaine de moteurs de synthèse, chacun avec son écran vivant et son caractère, pour toute sorte de musique avec une orientation synthwave, et des effets à déclencher en direct (punch-in). L'esprit ludique et immédiat de l'OP-1 Field, sans en copier ni la bande ni le look.**

Ce que ce n'est pas : un OP-XY (station de séquençage, très complète mais peu ludique, Anthony en a déjà un), un tracker, un sampler.

## 2. La boucle de jeu (ce qu'on fait en 5 minutes)

1. Choisir la piste 1, choisir un moteur et un son (preset).
2. REC : jouer au clavier. La longueur de la boucle est fixée par la première prise (arrondie à la mesure) ou choisie d'avance (1, 2, 4 ou 8 mesures) : les deux sont possibles. Ou bien, au séquenceur, poser les pas un à un (batterie d'abord).
3. La boucle tourne. Passer à la piste 2, un autre moteur (une basse FM, une batterie…), REC : on joue par-dessus. Chaque passage s'ajoute (overdub).
4. Pendant que tout tourne : sculpter le son avec les 4 potards, déclencher des **effets punch-in** (répétition, arrêt de bande, balayage de filtre…), couper ou rétablir des pistes.
5. Annuler la dernière prise, effacer une piste, recommencer.
6. Faire une autre boucle (le refrain, le pont…) et enchaîner les boucles en une song.

## 2 bis. Partir de SLOOP

[SLOOP](https://github.com/isod89/sloop-fm1) (isod89, GPL-3.0, sur Felucca) est un looper-groovebox qui marche déjà sur le FM-1. FM80S en est un fork.

**Ce que SLOOP apporte déjà :** le looper (prise libre ou longueur choisie, overdub, annuler / rétablir, effacer), le séquenceur de 64 pas (verrous de paramètres, micro-décalage, fills), une batterie de 16 sons sur les blanches et 37 kits (dont un synthwave), 12 moteurs et 153 sons, l'arpégiateur et la répétition de note, 16 gammes et les accords sur une touche, 16 effets punch-in, des envois chorus / delay / réverbe, 4 sections A–D enchaînées en song, l'audio USB, le MIDI, l'éditeur web et l'installateur.

**Ce que FM80S ajoute, dans l'ordre :**
1. **Un FM-1 virtuel dans le navigateur** : le vrai firmware en wasm, pour essayer chaque story (SLOOP n'en a pas).
2. **8 pistes au lieu de 4** : 7 synthés et la batterie (`10_FAISABILITE_8_PISTES.md`).
3. **Une identité synthwave forte** : le nom FM80S, une image de démarrage, un thème et des couleurs, des sons et des kits synthwave, des écrans vivants par moteur.
4. **Le reste de ce PRD**, après avoir joué SLOOP en main : là où SLOOP fait autrement, on garde sa façon si elle plaît à Anthony, ou on la change (`08_DECISIONS.md`, Q-01, Q-02, Q-07, Q-08).

## 3. Le périmètre du MVP

| Bloc | Contenu |
| --- | --- |
| **Moteurs** | *À confronter à SLOOP, qui a déjà 12 moteurs (analogique, FM 4 et 6 opérateurs, distorsion de phase, granulaire, modèles physiques…) : on n'écrit un moteur que s'il manque.* Une interface commune, puis des moteurs ajoutés un par un. Les 4 priorités, dans l'ordre : **1. Drums** (batterie synthétique façon boîtes à rythmes 80s, un son par touche : kick, snare, hats, clap, toms, percus) ; **2. VA**, le moteur de basse (soustractif mono, filtre résonant, glide, façon Minimoog / Jupiter ; sert aussi aux leads et aux arpèges) ; **3. Supersaw** (nappes et leads, chorus intégré, façon Juno / JP-8000) ; **4. FM** (4 opérateurs : piano électrique, cloches, basse slap, façon DX7). Ensuite, au choix : wavetable, distorsion de phase (façon CZ), cordes (façon Solina), corde pincée (Karplus-Strong), orgue additif, cuivres façon CS-80… |
| **Identité visuelle** | Chaque moteur a son écran animé qui montre ce que font les potards (forme d'onde qui bouge, opérateurs FM, pads de batterie…). Peu de texte : l'écran est petit. |
| **Potards** | KNOB1–4, de 4 couleurs, règlent les 4 paramètres principaux de l'écran affiché ; SEL + potard, les 4 suivants. |
| **Looper** | 8 pistes de notes (7 synthés et la batterie ; les moteurs lourds sur les pistes 1 à 3, ADR-039), un moteur et un son par piste. Longueur fixée par la première prise ou choisie d'avance. Overdub, annuler la dernière prise, effacer, mute. Quantification au choix (aucune, 1/16, 1/8). Métronome. Tempo réglable. |
| **Séquenceur** | Programmer une piste pas à pas, sur les 16 touches blanches (un pas par touche, les LED montrent le motif). D'abord la batterie seule, pour éprouver le concept, puis extensible aux autres pistes. Longueur au choix de l'utilisateur, pour toutes les pistes : 16 pas par défaut, jusqu'à 128 *(SLOOP : 64 pas ; 128 se chiffre après F1, la mémoire est pleine)*. Ce qu'on joue en live et ce qu'on programme vont dans la même boucle : on peut mélanger. |
| **Scènes et song** | *À confronter à SLOOP (4 sections A–D, une chaîne, une song enregistrée).* **16 scènes** par projet ; une scène = une boucle complète (les motifs des 8 pistes, sa longueur). En live, sur l'écran des scènes, les touches blanches choisissent la scène suivante, qui part **à la fin de la boucle en cours** (toujours en rythme). En mode song, une liste de 64 entrées au plus (scène, 1 à 16 répétitions) se joue toute seule ; on l'écrit, ou on l'enregistre en jouant les scènes en live. *(Proposition, à valider à l'essai.)* |
| **Effets** | Par piste : filtre, saturation. En envoi : chorus, delay synchronisé, réverbe (dont une réverbe « gated », signature de la caisse claire synthwave). Sur le master : saturation façon bande, limiteur. |
| **Punch-in** | *À confronter à SLOOP (16 effets, FX + une touche blanche).* Une partie dédiée de l'écran FX : une fois le mode punch-in enclenché, **les 11 touches noires** déclenchent chacune un effet momentané sur tout le mix, tant qu'on la tient (répétition, arrêt de bande, balayage passe-bas ou passe-haut, bitcrush, gate rythmique, envoi delay ou réverbe à fond…), et les potards règlent l'effet en cours. Les touches blanches continuent de jouer. |
| **Arpégiateur** | **Obligatoire** (bouton ARP) : les modes classiques (montant, descendant, aller-retour, aléatoire, ordre de jeu), un **mode repeat** qui répète la note tenue (surtout pour la basse), et le réglage du **gate** et de la **vitesse** (synchronisée au tempo). |
| **Gammes** | **Obligatoire.** Une note de base (C à B) et une gamme, réglées pour le projet : chromatique, majeure, mineure naturelle, **mineure harmonique**, mineure mélodique, les modes (dorien, phrygien, lydien, mixolydien, locrien), pentatoniques majeure et mineure, blues, et d'autres au besoin. L'arpégiateur et le séquenceur la suivent. **Mode facile** *(à confronter à « la gamme sur les blanches » de SLOOP)* : on ne joue que des notes de la gamme. Proposition : les 16 touches blanches jouent les degrés successifs de la gamme (plus de deux octaves, aucune fausse note possible), les touches noires restent libres (punch-in…). Autre façon, à comparer à l'essai : garder le clavier du piano et ramener une touche hors gamme sur la note de la gamme la plus proche. |
| **Jeu** | Polyphonie selon le moteur (de 4 à 8 voix, à mesurer sur l'appareil), glide, mono / poly. |
| **Presets** | Une banque de sons par moteur, orientée synthwave, et sauvegarde des sons et des projets (boucles comprises). |
| **FM-1 virtuel** | La même chose dans le navigateur, au clavier de l'ordinateur et au panneau. **Avant toute fonction nouvelle** (ADR-041). |
| **Identité** | Le nom FM80S, une image de démarrage synthwave, un thème et 8 couleurs de piste ; le crédit « d'après SLOOP et Felucca » (ADR-044). |

**Hors MVP :** enregistrer du son (pas d'entrée audio sur le FM-1, et ~5 s de mémoire), échantillons longs, export WAV de la boucle (à voir). *Le granulaire, les échantillons courts et le MIDI out viennent de SLOOP : on les garde.*

## 4. Les contrôles du FM-1 (première idée, à essayer sur l'appareil)

> *Jusqu'à l'essai en main, on garde les contrôles de SLOOP* (son `GUIDE.md`). Ce tableau est l'intention de départ ; on le confronte à SLOOP après F0-01. Le choix de piste à SEL + touche noire devient nécessaire à 8 pistes (F1-02).

| Contrôle | Rôle |
| --- | --- |
| 27 touches | Jouer ; en batterie, un son par touche |
| OCT− / OCT+ | Octave |
| KNOB1–4 | Les 4 paramètres de l'écran ; SEL + potard, les 4 suivants |
| SELECT / ALGORITHM / PRESETS | Choisir le moteur, la piste, le son (à préciser) |
| REC / PLAY | Enregistrer et overdub / lancer et arrêter la boucle |
| ENV, LFO, FX, ARP, SEQ, GLO, HOME | Les écrans : enveloppes, LFO, effets, arpégiateur, looper, réglages globaux, moteur |
| SAVE | Sauvegarder |
| FX (mode punch-in) + touches noires | Les 11 effets punch-in |
| Séquenceur + 16 touches blanches | Les 16 pas du motif |
| SEL + touche noire | Les noires sont gravées OP1–OP6, PIT, GLO, MONO, POLY : choisir la piste (OP1–OP6…), glide, mono / poly |

## 5. Ce qu'on reprend

- **De SLOOP : tout le firmware**, avec son historique (ADR-037), ses tests (golden, budget CPU, simulateurs sur l'ordinateur), son build et son installateur.
- **De P0-01 et de Trawler** (tag `plateforme-trawler`, ADR-045) : pour l'émulateur, le worklet, le panneau, le clavier AZERTY et l'anti-cache `?v=` ; les idées de l'écran des pads et du thème.

## 6. Contraintes

Celles de la puce et de SLOOP (ADR-043, `CLAUDE.md`) : RAM ≤ 96 Kio, pool ≤ 336 Kio (déjà presque plein), code ≤ ~568 Kio, rien de bloquant dans le callback audio, pas de `malloc` ; GPL-3.0 ; ni marque, ni graphismes, ni noms de Teenage Engineering, d'Arturia ou de M-VAVE (« façon Juno » pour décrire un caractère, jamais dans le produit), et ni « SLOOP » ni « Felucca » comme nom de produit.

## 7. Critères de succès

1. **Ludique :** dans le navigateur, quelqu'un qui découvre empile 3 boucles en moins de 10 minutes, sans manuel.
2. **Musical :** Anthony fait une boucle synthwave de 4 pistes (batterie, basse, nappe, lead) qu'il a envie de réécouter.
3. **Une song :** Anthony enchaîne au moins 3 scènes en un morceau de 2 minutes.
4. **Sur l'appareil :** 8 pistes + effets sans craquement sur un vrai FM-1.

## 8. Risques et parades

| Risque | Parade |
| --- | --- |
| La vitesse du processeur est inconnue | SLOOP tourne déjà sur le FM-1 avec 4 pistes. On mesure le coût des 8 pistes sur l'appareil (F1-01) avant de dessiner leur interface |
| La mémoire est presque pleine | Moteurs lourds sur 3 pistes, slicer partagé (ADR-039, ADR-040) ; mesurer au premier build ; les sections à 8 pistes sont le point dur (`10_FAISABILITE_8_PISTES.md` §4) |
| SLOOP avance de son côté | Remote `sloop`, fusions ponctuelles ; nos changements groupés et commentés pour que les fusions restent faciles |
| Les calques de SLOOP ne sont pas assez « ludiques » | Anthony le juge en main dès F0-01 (Q-07), avant qu'on construise par-dessus |
| Trop proche d'un OP-1 | Le looper de notes et l'orientation synthwave font l'identité ; visuels et noms maison |
| Refaire l'erreur du tracker | Chaque story se termine par quelque chose qu'Anthony essaie dans le navigateur (puis sur l'appareil), et il donne son avis avant la suivante |

## 9. Décisions prises (2026-10-10)

1. **Nom : FM80S.** « FM » est un terme générique (la synthèse FM) ; on n'utilise ni « M-VAVE » ni « FM-1 » comme nom de produit.
2. **Nouveau dépôt**, et Trawler archivé.
3. **Boucle :** longueur fixée par la première prise *ou* choisie d'avance ; **et un séquenceur pas à pas**, au minimum pour la batterie.
4. **Punch-in :** une partie de l'écran FX ; en mode punch-in, les touches noires les jouent, les potards les règlent.
5. **Moteurs, dans l'ordre :** Drums, VA (basse), Supersaw, FM.
6. **Une song complète :** des scènes enchaînées.
7. **Arpégiateur obligatoire**, avec un mode repeat (surtout pour la basse), les modes classiques, le gate et la vitesse.
8. **En mode punch-in, les touches noires ne jouent plus de dièses :** accepté.
9. **Séquenceur :** batterie seule d'abord, extensible ensuite ; 16 pas par défaut, jusqu'à 128, pour tous les instruments.
10. **Dépôt :** `y6zkcqnf5r-hue/fm1-fm80s`.
11. **Gammes obligatoires :** note de base + gamme (majeures, mineures dont l'harmonique, modes…), et un **mode facile** qui ne joue que des notes de la gamme.
12. **Fork de SLOOP** (v0.2), dans le même dépôt ; **8 pistes** = 7 synthés + batterie ; moteurs lourds sur 3 pistes ; **émulateur web avant les fonctions** ; la 1re story installe FM80S sur le FM-1 (ADR-036 à ADR-045).

## 10. Questions encore ouvertes

Elles sont tenues à jour dans `08_DECISIONS.md` : Q-01 (scènes ou sections), Q-02 (mode facile), Q-07 (les calques de SLOOP), Q-08 (punch-in), Q-09 à Q-11 (moteurs légers, RAM, sections à 8 pistes), Q-12 (icônes).
