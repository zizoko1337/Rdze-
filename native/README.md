# Natywny fundament wydajnościowy RDZEN

**Etap 1 ekspedycji 2137:** zwykły `START.bat` uruchamia teraz rozgrywkę z HP rdzenia, porażką i restartem, wydobyciem uranu, dronem transportowym oraz terminalem z wyborem 1 z 3 kart. `expedition.hpp` zawiera ekonomię i kolejkę wypłat, `expedition_scene.hpp` model transportowca, `expedition_tests.cpp` testy mechanik. `TEST_EKSPEDYCJI.bat` sprawdza pełną pętlę interakcji. Polecenia `--command-test` i `--minigun` zachowują wcześniejsze kontrolowane próby działa, a `--arsenal` poligon. Szczegóły w [głównej instrukcji](../README.md).

**Aktualizacja 03:** `START.bat` otwiera teraz centrum dowodzenia z wyborem ręcznego lasera lub miniguna, fotelem i windą ponad dach. Dawny poligon działa przez `ARSENAL.bat` (`--arsenal`). Nowe moduły: `command_game.hpp` — stan stanowiska, strzelanie i fizyka pocisków; `command_scene.hpp` — dach, fotel, działo i interfejs; `command_app.hpp` — aplikacja i test interakcji; `command_tests.cpp` — testy mechanik. `TEST_DZIALA.bat` sprawdza oba działa, a zrzuty i raporty zapisuje w `artifacts/command/`. [Aktualna instrukcja](../README.md).

Ten katalog zawiera natywną grę oraz oddzielne laboratorium wydajności, oba w C++20 + SDL3 + Direct3D 11. **`ARSENAL.bat` uruchamia poligon dziewięciu wieżyczek**, z siedmioma gatunkami bestii, budowaniem i kamerą operatora. Sterowanie i opis poligonu: [instrukcja arsenału](../ARSENAL.md). `gameplay.hpp` zawiera wspólną symulację przeciwników i wieżyczek, `game_scene.hpp` modele i kamery poligonu, `game_ui.hpp` jego interfejs, `game_audio.hpp` syntezę dźwięku, a `game_main.cpp` pętlę aplikacji i wybór trybu. `TEST_ARSENALU.bat` sprawdza interakcje oraz obciążenie poligonu; raporty zapisuje w `artifacts/arsenal/`.

Poniższy opis dotyczy **oddzielnego laboratorium `rdzen-native.exe`** z `main.cpp` i `simulation.hpp`. Nie należy utożsamiać jego sztucznego obciążenia ze zwykłą rozgrywką. Starszy prototyp przeglądarkowy pozostaje dostępny przez `WEB_PROTOTYP.bat`.

## Uruchomienie

- `TEST_NATYWNY.bat` — interaktywny podgląd sceny, początkowo 5000 bestii, 50 000 pocisków i 256 dział.
- `BENCHMARK.bat` — siedem pomiarów po 3 s rozgrzewki i 12 s pomiaru, razem około 105 s. Wyniki CSV, informacje o środowisku i zrzuty obrazu trafiają do `artifacts/native/benchmark/`.
- `native/build/rdzen-native.exe --benchmark --only 5000_50000_tactical --seconds 30` — dłuższy pomiar pojedynczej sceny; uruchamiać z głównego folderu projektu.
- `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1` — ponowna kompilacja Release i testy poprawności. Nie zmienia ustawień ExecutionPolicy w systemie.

Gotowy plik EXE potrzebuje znajdującego się obok `SDL3.dll` oraz `public/assets/` w projekcie. Nie potrzebuje Node.js, przeglądarki ani internetu. Wersje bibliotek i kompilatora są przypięte w `setup.ps1`, a pobrane archiwa weryfikowane SHA256 z oficjalnych wydań. Kompilator jest lokalny w `.toolchain/`, bez zmian PATH i instalowania silnika gry.

## Sterowanie w podglądzie

1 / 2 / 3 przełącza odpowiednio 1000 / 5000 / 10 000 przeciwników oraz 10 000 / 50 000 / 100 000 pocisków. Tab przełącza kamerę taktyczną i operatora. WASD porusza operatorem po pokładzie, mysz obraca widok, kółko zmienia wysokość kamery taktycznej. P pauzuje symulację. V przełącza VSync. Esc uwalnia mysz; ponowne Esc zamyka program.

Podgląd to test dla komputera stacjonarnego; przy ekstremalnym przybliżeniu pociski mogą mocno zasłaniać scenę. Początkowy brak limitu liczby efektów jest zamierzony w benchmarku.

## Co faktycznie mierzymy

- Każda bestia ma pozycję, HP, prędkość, rozmiar i numer generacji. Przemieszcza się ku rdzeniowi. Po śmierci lub dojściu do rdzenia odradza się, utrzymując stałą liczbę aktywnych jednostek.
- Każdy pocisk jest aktywnym elementem symulacji: ma pozycję, poprzednią pozycję, prędkość, czas życia i uchwyt celu. Wykonuje ruch naprowadzany i test przecięcia odcinka z kulą celu. Trafienie odejmuje HP. Pocisk wraca do puli, bez tworzenia nowego obiektu.
- Stała populacja pocisków służy obciążeniu; nie odpowiada jeszcze docelowej szybkostrzelności dział. Jeśli wybrana wieżyczka nie ma celu w zasięgu, generator obciążenia przypisuje pociskowi dowolną żywą jednostkę. Reguła 48 m dotyczy normalnego namierzania dział w laboratorium.
- Wyszukiwanie celu korzysta z siatki komórek 8 m. Działo utrzymuje poprawny cel; skanowanie w razie braku celu jest ograniczone do 10 Hz. Siatka jest przebudowywana w każdym kroku symulacji.
- Bestie i pociski mają oddzielne wywołania instancjonowanego rysowania. 50 000 pocisków to jedno wywołanie `DrawInstanced`, a nie 50 000 osobnych obiektów renderera.
- Wszystkie zadane jednostki są przesyłane do renderera, także te poza polem widzenia. Nie stosujemy jeszcze odrzucania niewidocznych obiektów po stronie CPU. GPU naturalnie odrzuca obiekty poza kadrem i zakryte piksele.
- Sprite'y mają odcięcie przezroczystego tła i zapis głębokości; pociski używają mieszania addytywnego bez zapisu głębokości. Tekstury mają mipmapy.
- Scena zawiera arenę, rdzeń, skały, słupy, obracane głowice i interfejs diagnostyczny. Nie zastępujemy przeciwników prostymi punktami.

## Metodologia i ograniczenia

Symulacja pracuje w stałym kroku 60 Hz, niezależnie od liczby renderowanych klatek. Pomiar obejmuje czas całej klatki wraz z `Present`, osobno krok symulacji, przygotowanie danych instancji, wywołania sterownika i czas GPU. Znaczniki czasu GPU są odczytywane asynchronicznie; nie czekamy na GPU w każdej klatce. Błędne próbki `Disjoint` są pomijane. Pamięć oznacza working set procesu, nie zajętość VRAM.

VSync jest wyłączony w pomiarze, żeby odsłonić zapas wydajności. FPS powyżej 60 na ekranie 60 Hz oznacza zapas obliczeniowy, a nie tyle wyświetlonych odświeżeń. p99 czasu klatki oznacza czas, poniżej którego mieści się około 99% klatek. Nie jest to najgorsza klatka ani gwarantowane minimum FPS.

Pomiar nie obejmuje: fizycznego rozpychania tłumu, nawigacji wokół przeszkód, zderzeń dowolnego pocisku ze wszystkimi przeciwnikami, dynamicznych map cieni, bloom, zniszczalnego terenu, finalnego interfejsu ani dźwięku. Te elementy wymagają własnego budżetu. Szczególnie swobodne pociski obszarowe i duże przezroczyste eksplozje mogą istotnie zmienić koszt. Nie wolno przenosić wyników tego testu na dowolną przyszłą zawartość gry.

Ostatni scenariusz, `5000_50000_dense_fps`, gromadzi 5000 bestii i 50 000 pocisków w promieniu 12–42 m od rdzenia. Sprawdza duże pokrycie ekranu i nakładanie efektów w pierwszej osobie. Pozostałe scenariusze rozkładają ruch na dużej arenie. Kamera FPS w benchmarku powoli obiega rdzeń, żeby pomiar nie zależał od jednego nieruchomego kierunku.

Zrzuty ekranu, zapisy raportów, start programu, kompilacja shaderów i wczytanie tekstur odbywają się poza odcinkiem pomiarowym. Dane surowe pozostają w CSV. Podczas pomiaru nie minimalizować okna; inne programy oraz ustawienia sterownika mogą wpływać na wyniki.

## Zastosowane optymalizacje

1. Stałe, zarezerwowane pule danych. Bez alokowania obiektu dla każdego strzału.
2. Numery generacji chronią przed trafianiem nowej jednostki przez pocisk przypisany do poprzedniego mieszkańca tego samego miejsca w puli.
3. Siatka przestrzenna i utrzymywanie celu zamiast pełnego przeszukiwania jednostek co klatkę.
4. Instancing i wspólne materiały. Bufor instancji jest uzupełniany przez `WRITE_NO_OVERWRITE`, z jednym odrzuceniem zawartości przy rozpoczęciu klatki.
5. Kierunek pocisku przesyłany jako wektor: usunięto koszt `atan2` dla każdego pocisku i odtwarzania kąta przez `sin/cos` w shaderze.
6. Pełna kompilacja Release `-O3`; symulacja jest obecnie jednowątkowa. Nie potrzebujemy jeszcze komplikować jej zadaniami wielowątkowymi, żeby uzyskać mierzalny punkt startowy.

## Testy poprawności

`simulation-tests.exe` porównuje 10 000 zapytań siatki z pełnym przeszukiwaniem, weryfikuje utrzymywanie celu, utratę zasięgu, śmierć i ponowne wykorzystanie indeksu. Testuje też trafienia szybkich pocisków oraz 600 kroków symulacji z 5000 bestii i 50 000 pocisków. Sprawdza, czy pozycje są skończone, istnieją trafienia i eliminacje oraz czy tablice nie przeniosły się w pamięci. Osobny pomiar 20 000 identycznych zapytań porównuje koszt siatki i pełnego wyszukiwania; koszt przebudowy siatki jest uwzględniony w pomiarze kroku symulacji, ale nie w tym mikrobenchmarku zapytań.

## Źródła technologii

- [SDL3 — okno, klawiatura i mysz](https://wiki.libsdl.org/SDL3/FrontPage).
- [Direct3D 11 — rysowanie instancjonowane](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawinstanced).
- [Direct3D 11 — poprawność znaczników czasu GPU](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_query_data_timestamp_disjoint).
- [LLVM-MinGW — lokalny kompilator](https://github.com/mstorsjo/llvm-mingw/releases/tag/20261020).

Wybrano bezpośredni Direct3D 11 dla obecnego celu Windows. bgfx pozostaje możliwą warstwą przenośności, ale nie jest zależnością tego testu. Sam wybór API nie zastępuje optymalizacji algorytmów ani pomiarów.
