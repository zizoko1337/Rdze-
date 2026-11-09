# Nowi przeciwnicy i bestiariusz

Klawisz **' (apostrof)** otwiera i zamyka bestiariusz podczas podejścia. Esc też zamyka panel. Kliknięcie karty przywołuje wskazaną bestię 110–146 m przed kierunkiem patrzenia. Przyciski x1 / x10 / x100 zmieniają wielkość grupy. Strzałki wybierają kartę, Enter przywołuje. Panel zatrzymuje symulację, wydobycie, pociski i timery zdolności. Przed rozpoczęciem wydobycia potwory czekają na naciśnięcie czerwonego przycisku.

Karty zawierają prawdziwe sprite’y, HP, prędkość i opis ataku. HP i prędkość odpowiadają jednostkom przywoływanym przy bieżącym czasie wyprawy. Bossowie w naturalnym harmonogramie mają odrębne skalowanie HP wraz z numerem spotkania.

| Nowa bestia | Bazowe HP do testów | Prędkość | Zachowanie |
|---|---:|---:|---|
| Sabotażysta — elita | 1600 | 5 m/s | Wybiera najbliższą wieżyczkę, podchodzi na 4 m, niszczy po 3 sekundach nieprzerwanego ataku. Ogłuszenie zeruje postęp. Gdy nie ma wieżyczek, idzie na rdzeń. |
| Plujec oblężniczy | 900 | 3 m/s | Zatrzymuje się 63 m od środka, przy drugim rzędzie stanowisk. Strzela kwasem za 45 obrażeń co 5 s. Pocisk leci łukiem i zadaje obrażenia po dolocie. |
| Nadumysł — boss mózgowy | 16000 | 2,6 m/s | Co 6 s wysyła impuls o promieniu 42 m, wyłączający wieżyczki na 4 s. Obejmuje wsparcie: wyłącza premie tempa, zasięgu i śnieżycę. |

Zniszczenie wieżyczki zwalnia heks i licznik posiadanych kopii. Odblokowana technologia zostaje zapamiętana. Zakłócone wieżyczki ciemnieją i dostają fioletowy znacznik; po 4 s wracają do działania. Już wystrzelone pociski kontynuują lot. Impulsy kolejnych Nadumysłów przedłużają wyłączenie bez sumowania czasu. Śmierć bossa nie cofa już nałożonego zakłócenia.

Sabotażyści dołączają do elitarnego harmonogramu od 2:15, co drugie spotkanie elity. Plujce oblężnicze dołączają do zwykłych fal od 5:00. Bossowie występują co 3 minuty w cyklu Tytan → Królowa → Nadumysł (pierwszy Nadumysł w 9:00). Nadumysł zawsze zostawia kryształ bossa; sabotażysta korzysta z dotychczasowej 1% szansy elity.

Wcześniejszy plujec strzela co 3 s; usunięto przypadkowe dodatkowe obrażenia kontaktowe zadawane z dystansu.

Implementacja: stałe pule przeciwników; szukanie nowej wieżyczki tylko przez sabotażystów, najwyżej co 0,5 s, stabilny identyfikator celu; impulsy przeglądają wyłącznie wieżyczki, cache wsparcia przebudowuje się po zmianie stanu; maksymalnie 512 pocisków kwasowych. Menu używa już załadowanych tekstur i nie tworzy dodatkowej kamery.

Testy: `native/enemy_tests.cpp` sprawdza pełne 3 s ataku, przerwanie, stabilność celu po usunięciu innej wieżyczki, zwalnianie heksu, tor lotu/cadencję/zasięg plujca, granicę 42 m, wyłączenie i odzyskanie wsparcia, stun bossa, wszystkie karty i partie przywołań, pauzę, kryształ bossa i harmonogram. Zawiera też pomiar samej symulacji 6000 przeciwników i 180 wieżyczek. `Rdzen.exe --enemy-test` sprawdza prawdziwe klawisze i kliknięcia menu oraz zapisuje zrzuty ekranu do katalogu artefaktów działającej kopii.

## Grafiki i prompty

Wygenerowano wbudowanym narzędziem `image_gen` według umiejętności imagegen, z przezroczystym tłem. Pliki produkcyjne są w projekcie i trafiają do samodzielnego EXE.

### `public/assets/alien-saboteur.png`

Use case: stylized-concept. Asset type: single transparent 2D enemy sprite for native 2.5D sci-fi survival game. Subject: alien elite tower saboteur, low powerful six-legged crustacean beast with giant paired cutting mandibles and chisel claws for tearing steel turrets, dark obsidian chitin with glowing amber fissures, visibly dangerous armored elite. Full body front three-quarter view, slightly elevated camera, centered, all feet inside frame, clean readable silhouette, isolated on genuinely transparent background. Detailed painted game sprite with realistic material highlights, same grounded sci-fi monster aesthetic as an alien insect horde, no floor, no scenery, no text, no borders, no cast shadow. Square canvas.

### `public/assets/alien-siege-spitter.png`

Use case: stylized-concept. Asset type: single transparent enemy sprite for a 2.5D sci-fi survival game. A heavy alien artillery spitter monster, crouched six-legged organism with inflated acid sacs along its back and a long upward curved biological mortar snout, dark purple armored chitin with toxic lime green translucent sacs, different silhouette from a clawed melee bug. Detailed painted game sprite realistic texture, full body front three-quarter view from a slightly elevated camera, centered, isolated on genuinely transparent background, no scenery, floor, shadow, lettering or borders. All limbs inside square frame.

### `public/assets/boss-brain.png`

Use case: stylized-concept. Asset type: single transparent boss enemy sprite for a 2.5D sci-fi survival game. Giant alien brain boss: exposed intricate pink violet brain hemispheres within a crown of dark chitin, floating above the ground on trailing curled tentacles, psychic cyan violet glow from veins and eyes. Menacing biological psychic creature, strong distinctive round brain silhouette, detailed painted game sprite realistic material highlights. Full body including tentacles, front three-quarter view from slightly elevated camera, centered and fully contained with margin on square genuinely transparent background. No scenery, floor, cast shadow, lettering or borders.
