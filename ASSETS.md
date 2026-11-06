# Grafiki projektu

Jeździk ma nowy sprite od tyłu: [public/assets/rover-rear.png](public/assets/rover-rear.png). Pochodzenie, narzędzie i pełny prompt: [ASSETS-JEZDZIK.md](ASSETS-JEZDZIK.md).

Nowy natywny poligon ma sześć dodatkowych grafik przeciwników, w tym dwóch bossów. Pliki i pełne prompty: [native/ASSETS-ARSENAL.md](native/ASSETS-ARSENAL.md). Opis poniżej dokumentuje grafiki pierwotnego prototypu; natywna wersja również korzysta z `alien-beast.png` i `turret-metal.png`, a teren, efekty i modele tworzy w Direct3D 11.

Wygenerowano wbudowanym narzędziem `image_gen` (umiejętność `imagegen`), bez CLI i bez klucza API. Pliki skopiowane do projektu; gra nie odwołuje się do katalogu generatora. Model działka jest autorską geometrią 3D według szkicu użytkownika: prostokątny nieruchomy słup, prostokątna obrotowa głowica, krótka lufa i czerwony emiter. Żółta błyskawica jest rysowana proceduralnie na przezroczystej teksturze.

## public/assets/alien-beast.png

Przezroczysty sprite bestii. Pełny prompt:

> Use case: stylized-concept. Asset type: production 2D billboard enemy sprite for a dark science fiction 3D survival game. Primary request: one alien planetary guardian beast, full body, facing the viewer in a slightly three-quarter frontal combat stance, four angular crab-like legs, hunched armored black-violet chitin body, bright coral red bioluminescent fissures and two small glowing eyes. Strong readable silhouette, chunky painterly game illustration, detailed but legible from a distance. Whole creature centered with generous empty margin, feet along same baseline. Actual transparent background. No floor, no shadow plane, no scenery, no text, no UI, no multiple views, no sheet. Square asset. Save output for integration into local game.

## public/assets/turret-metal.png

Tekstura metalowej obudowy głowicy, słupa i platformy. Pełny prompt:

> Use case: stylized-concept. Asset type: seamless square albedo texture for a sci-fi laser turret metal housing in a 3D game. A flat orthographic closeup of one industrial gunmetal gray metal armor plate covering the entire image. Fine brushed steel grain, subtle weathered edges, restrained scratches, small recessed seams near edges and small dark bolts in corners. Mid gray blue steel, readable and reasonably bright. Even neutral diffuse lighting, no directional shadow, no perspective, no object silhouette, no typography, no lettering, no logos, no gun illustration, no scene, no glow, no strong large designs. Tileable uniform material usable across rectangular turret head and tall rectangular support post. Crisp game material texture.

Tekstury podłoża, cieni i emblematu są tworzone w Canvas przy uruchomieniu. Gwiazdy, skały, kryształy, planeta, platforma i wieżyczki to geometria Three.js. Interfejs używa lokalnych fontów systemowych. Dźwięki powstają w Web Audio API.
