# RDZEŃ — Poligon arsenału

Natywny prototyp 2.5D na Windows: świat 3D, bestie jako sprite’y 2D, budowanie obrony z góry i obserwacja z pierwszej osoby na podwyższonym rdzeniu. C++20 + SDL3 + Direct3D 11, bez Unity, Godota i przeglądarki.

## Uruchomienie

Kliknij **ARSENAL.bat**. To wcześniejszy poligon dziewięciu automatycznych wieżyczek. `START.bat` uruchamia teraz centrum dowodzenia z ręcznym działem. Oba tryby są w `native/build/rdzen-game.exe`; potrzebuje sąsiedniego `SDL3.dll` i katalogu `public/assets/`. Zachowaj strukturę folderów. Uruchomienie gotowej kompilacji nie wymaga internetu ani Node.js. Gra działa na Windows x64 z Direct3D 11.

Jeżeli brakuje kompilacji, START przygotuje ją lokalnym kompilatorem. Pierwsze odtworzenie narzędzi na innym komputerze wymaga internetu. **PRZEBUDUJ.bat** kompiluje grę i laboratorium oraz uruchamia testy symulacji. Narzędzia pozostają w `.toolchain/`, bez zmian systemowego PATH.

## Co jest w prototypie

- Kolisty teren o średnicy 6,4 km, skały, kryształy, mgła i odległy horyzont. Przeciwnicy nadchodzą ze wszystkich stron, z obrzeży strefy walki.
- Rdzeń o wysokości 18 m i promieniu 14 m. Można chodzić po jego pokładzie; krawędź i środkowy reaktor ograniczają ruch.
- Darmowe budowanie w promieniu 240 m. Dziewięć różnych wieżyczek stoi od początku wokół rdzenia. Limit prototypu: 512 wieżyczek i 20 000 aktywnych bestii.
- Siedem oddzielnych gatunków: podstawowa bestia, szybki łowca, opancerzony taran, plujący stwór, latająca manta oraz bossowie Tytan i Królowa. Każdy ma własny sprite. Królowa przywołuje mniejsze jednostki.
- Fale stopniowo zwiększają liczebność i wytrzymałość przeciwników. Co czwarta fala przyprowadza bossa. Fale można wyłączyć lub ręcznie dodawać przeciwników.
- Brak pasków HP przeciwników. Rdzeń jest niezniszczalny w tym poligonie, żeby można było swobodnie oglądać broń. Licznik pokazuje przyjęte obrażenia.
- To sandbox: bez ekonomii, zapisu stanu, kampanii i ulepszeń. Animacje bestii są proceduralne; nie mają jeszcze osobnych klatek chodu.

## Arsenał

| Klawisz | Wieżyczka | Działanie i ruchome elementy |
| --- | --- | --- |
| 1 | Laser | Szybkie pewne trafienia; obracana głowica, emiter i chłodzenie. |
| 2 | Minigun | Rozkręcający się zestaw sześciu luf i mechanizm podawania amunicji. |
| 3 | Tesla | Pioruny do wszystkich przeciwników w zasięgu; 5,5 s odnowienia, ruchome ramiona i cewki. |
| 4 | Snajper | Duży zasięg i obrażenia; odrzut długiej lufy i ruch zamka. |
| 5 | Lanca | 6 s ładowania, 3 s ciągłej wiązki, następnie chłodzenie; rozsuwane i obracane elementy głowicy. |
| 6 | Jad | Pociski i trujące chmury z obrażeniami w czasie; zielone zbiorniki i tłoki. |
| 7 | Rakiety | Naprowadzane rakiety z eksplozją obszarową; ruchome wyrzutnie i pokrywy. |
| 8 | Moździerz | Pociski po wysokim łuku, eksplozje obszarowe; odrzut lufy i hydraulika. |
| 9 | Palnik | Stożek ognia i podpalenie; ruch pompy i elementów chłodzenia. |

Bronie wybierające pojedynczy cel utrzymują go do śmierci albo wyjścia z zasięgu. Nie ma losowego pudłowania. Tesla, eksplozje, chmury i palnik obejmują wiele jednostek zgodnie ze swoim obszarem działania. Nieruchoma podstawa pozostaje oddzielona od obracanej głowicy.

## Sterowanie

| Klawisz | Działanie |
| --- | --- |
| Tab | Pokład z pierwszej osoby ↔ budowanie z góry |
| WASD | Chodzenie / przesuwanie kamery taktycznej |
| Kliknięcie terenu + mysz | Rozglądanie na pokładzie |
| Shift | Szybszy ruch na pokładzie |
| 1–9 / karty na dole | Wybór wieżyczki |
| LPM | Postawienie wieżyczki w widoku taktycznym |
| PPM / B | Anulowanie / przełączenie budowania |
| X | Usunięcie wskazanej wieżyczki |
| Kółko / Q i E / C | Zoom / obrót / wycentrowanie kamery taktycznej |
| N / J / H | Kolejna fala / boss / dodatkowa horda 250 bestii |
| F | Automatyczne fale włączone / wyłączone |
| P | Pauza; nadal można budować i przesuwać kamerę |
| R | Nowy poligon |
| M / V | Dźwięk / VSync |
| F1 | Pomoc |
| F11 | Pełny ekran / okno |
| Esc | Zamknięcie pomocy, uwolnienie myszy lub pauza |
| Alt+F4 / Ctrl+Q | Wyjście |

Zielony podgląd oznacza poprawne miejsce budowy, czerwony — kolizję lub miejsce poza strefą. Po anulowaniu budowy wskazanie istniejącej wieżyczki pokazuje jej zasięg. Utrata aktywności okna automatycznie pauzuje grę; P wznawia.

## Testy i wydajność

**TEST_ARSENALU.bat** uruchamia automatyczny test sterowania, budowania, kamer i strzelania, następnie scenę z 99 wieżyczkami i populacją uzupełnianą do 5000 bestii. Raporty JSON, klatki CSV i zrzuty są w `artifacts/arsenal/`. Pomiar wyłącza VSync i dźwięk. Ostatnia walidacja: [PROTOTYP_02.md](PROTOTYP_02.md).

Symulacja działa w stałym kroku 60 Hz. Renderowanie używa instancjonowania, grupowania wspólnych materiałów, siatki przestrzennej do wyszukiwania celów i pul danych z numerami generacji. Efekty mają limity, żeby utrzymać ograniczony koszt pamięci i rysowania.

Oddzielne **TEST_NATYWNY.bat** i **BENCHMARK.bat** zachowują wcześniejsze laboratorium z maksymalnie 10 000 bestii i 100 000 aktywnych pocisków. To inne obciążenie niż zwykła rozgrywka. Historyczne wyniki i ograniczenia: [WYNIKI_WYDAJNOSCI.md](WYNIKI_WYDAJNOSCI.md); metodologia: [native/README.md](native/README.md).

## Pliki

- `native/gameplay.hpp` — reguły, broń, przeciwnicy, fale i efekty walki.
- `native/game_scene.hpp` — arena, modele wieżyczek, animacje i kamery.
- `native/game_ui.hpp`, `native/game_audio.hpp` — interfejs i syntezowane dźwięki.
- `native/game_main.cpp` — aplikacja, sterowanie i testy interakcji.
- `native/renderer.hpp` — renderer Direct3D 11 wspólny z laboratorium.
- `native/game_tests.cpp` — testy mechanik arsenału i symulacji tłumu.
- `public/assets/` — lokalne tekstury; pochodzenie i prompty: [ASSETS.md](ASSETS.md) oraz [nowe bestie](native/ASSETS-ARSENAL.md).

Starsza wersja przeglądarkowa pozostaje pod **WEB_PROTOTYP.bat**, ze źródłami w `src/`. Polecenia `npm run dev`, `npm test`, `npm run test:e2e` i `npm run build` dotyczą wyłącznie tej starszej wersji.
