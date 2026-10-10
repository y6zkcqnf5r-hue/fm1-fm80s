# PRD v0.1 — FM80S, un synthé-looper ludique pour le FM-1

> Brouillon relu une première fois avec Anthony (2026-10-10). Il remplace le tracker Trawler, arrêté le même jour : l'ergonomie du FM-1 ne convient pas à un tracker. **Nom : FM80S.** Nouveau dépôt ; Trawler est archivé.

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

## 3. Le périmètre du MVP

| Bloc | Contenu |
| --- | --- |
| **Moteurs** | Une interface commune, puis des moteurs ajoutés un par un. Les 4 priorités, dans l'ordre : **1. Drums** (batterie synthétique façon boîtes à rythmes 80s, un son par touche : kick, snare, hats, clap, toms, percus) ; **2. VA**, le moteur de basse (soustractif mono, filtre résonant, glide, façon Minimoog / Jupiter ; sert aussi aux leads et aux arpèges) ; **3. Supersaw** (nappes et leads, chorus intégré, façon Juno / JP-8000) ; **4. FM** (4 opérateurs : piano électrique, cloches, basse slap, façon DX7). Ensuite, au choix : wavetable, distorsion de phase (façon CZ), cordes (façon Solina), corde pincée (Karplus-Strong), orgue additif, cuivres façon CS-80… |
| **Identité visuelle** | Chaque moteur a son écran animé qui montre ce que font les potards (forme d'onde qui bouge, opérateurs FM, pads de batterie…). Peu de texte : l'écran est petit. |
| **Potards** | KNOB1–4, de 4 couleurs, règlent les 4 paramètres principaux de l'écran affiché ; SEL + potard, les 4 suivants. |
| **Looper** | 8 pistes de notes, un moteur et un son par piste. Longueur fixée par la première prise ou choisie d'avance. Overdub, annuler la dernière prise, effacer, mute. Quantification au choix (aucune, 1/16, 1/8). Métronome. Tempo réglable. |
| **Séquenceur** | Programmer une piste pas à pas, sur les 16 touches blanches (un pas par touche, les LED montrent le motif). D'abord la batterie seule, pour éprouver le concept, puis extensible aux autres pistes. Longueur au choix de l'utilisateur, pour toutes les pistes : 16 pas par défaut, jusqu'à 128. Ce qu'on joue en live et ce qu'on programme vont dans la même boucle : on peut mélanger. |
| **Scènes et song** | **16 scènes** par projet ; une scène = une boucle complète (les motifs des 8 pistes, sa longueur). En live, sur l'écran des scènes, les touches blanches choisissent la scène suivante, qui part **à la fin de la boucle en cours** (toujours en rythme). En mode song, une liste de 64 entrées au plus (scène, 1 à 16 répétitions) se joue toute seule ; on l'écrit, ou on l'enregistre en jouant les scènes en live. *(Proposition, à valider à l'essai.)* |
| **Effets** | Par piste : filtre, saturation. En envoi : chorus, delay synchronisé, réverbe (dont une réverbe « gated », signature de la caisse claire synthwave). Sur le master : saturation façon bande, limiteur. |
| **Punch-in** | Une partie dédiée de l'écran FX : une fois le mode punch-in enclenché, **les 11 touches noires** déclenchent chacune un effet momentané sur tout le mix, tant qu'on la tient (répétition, arrêt de bande, balayage passe-bas ou passe-haut, bitcrush, gate rythmique, envoi delay ou réverbe à fond…), et les potards règlent l'effet en cours. Les touches blanches continuent de jouer. |
| **Arpégiateur** | **Obligatoire** (bouton ARP) : les modes classiques (montant, descendant, aller-retour, aléatoire, ordre de jeu), un **mode repeat** qui répète la note tenue (surtout pour la basse), et le réglage du **gate** et de la **vitesse** (synchronisée au tempo). |
| **Jeu** | Polyphonie selon le moteur (de 4 à 8 voix, à mesurer sur l'appareil), glide, mono / poly. |
| **Presets** | Une banque de sons par moteur, orientée synthwave, et sauvegarde des sons et des projets (boucles comprises). |
| **FM-1 virtuel** | La même chose dans le navigateur, au clavier de l'ordinateur et au panneau. |

**Hors MVP :** enregistrer du son (pas d'entrée audio sur le FM-1, et ~5 s de mémoire), granulaire, échantillons longs, export WAV de la boucle (à voir), MIDI out.

## 4. Les contrôles du FM-1 (première idée, à essayer sur l'appareil)

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

## 5. Ce qu'on reprend de Trawler

À reprendre : `plat.h` et la plateforme (simulateur hôte, FM-1 virtuel, worklet, panneau, parité wasm/natif, CI, rapport de taille), le point fixe et les tables générées, les oscillateurs anticrénelés, le filtre SVF, les enveloppes, la voix, l'horloge exacte, la file de commandes, l'écran et les polices.

À laisser : le modèle de données du tracker, ses vues et sa saisie.

## 6. Contraintes (inchangées)

Entiers seulement, pas de division 64 bits, pas de libc, mémoire statique ; RAM ≤ 96 Kio, pool ≤ 336 Kio, code ≤ ~560 Kio ; même son en wasm et en natif ; GPL-3.0 ; ni marque, ni graphismes, ni noms de Teenage Engineering ou d'Arturia (« façon Juno » pour décrire un caractère, jamais dans le produit).

## 7. Critères de succès

1. **Ludique :** dans le navigateur, quelqu'un qui découvre empile 3 boucles en moins de 10 minutes, sans manuel.
2. **Musical :** Anthony fait une boucle synthwave de 4 pistes (batterie, basse, nappe, lead) qu'il a envie de réécouter.
3. **Une song :** Anthony enchaîne au moins 3 scènes en un morceau de 2 minutes.
4. **Sur l'appareil :** 4 pistes + effets sans craquement sur un vrai FM-1.

## 8. Risques et parades

| Risque | Parade |
| --- | --- |
| La vitesse du processeur est inconnue | **Faire tourner un premier son sur le FM-1 dès son arrivée** (la semaine prochaine), avant d'écrire beaucoup de moteurs ; mesurer, puis fixer le nombre de voix par moteur |
| Une dizaine de moteurs, c'est long | Une interface de moteur commune et petite ; un moteur = une story ; on joue à chaque étape |
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

## 10. Questions encore ouvertes

1. Scènes et song : la proposition du §3 (16 scènes, changement à la fin de la boucle, liste de 64 entrées) est à essayer.
2. La mémoire : 8 pistes × 16 scènes × jusqu'à 128 pas de notes et de gestes, à chiffrer dans la spec contre les budgets (pool de 336 Kio, effets compris).
