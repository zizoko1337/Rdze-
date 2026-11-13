# Kawa i senność operatora

Po 5 minutach aktywnej ekspedycji operator zaczyna być senny. Przez kolejne 15 sekund kara narasta: ruch i bieg spadają do 45% zwykłej prędkości, a obrót manualnego działa do maksymalnie około 37°/s. Kamera, lufa i celowanie zachowują wspólny kierunek. Obraz świata rozmywa się; co 4,5 s powieki zamykają się, całkowicie zasłaniają ekran na 0,35 s i otwierają się ponownie.

Zegar rusza po czerwonym przycisku wydobycia. Menu, pauza, terminale, radio, laboratorium, bestiariusz i pauza rozmieszczania zatrzymują zegar. Obserwowanie z góry bez magazynu nie zatrzymuje senności. Pomoc i menu pozostają czytelne.

## Ekspres

Ekspres stoi po lewej stronie, za fotelem, patrząc z miejsca rozpoczęcia ekspedycji. Podejdź od frontu, popatrz na urządzenie i naciśnij **E**. Kawa jest bezpłatna i dostępna od początku, również przed uruchomieniem wydobycia.

Postać ustawia się przed ekspresem i pozostaje zablokowana przez cały cykl: kawa leje się do otwartej filiżanki, poziom płynu rośnie, filiżanka unosi się do kamery, a podczas picia przechyla się i opróżnia. Animacja pokazuje wyłącznie filiżankę, bez dłoni i przedramienia. Samo picie zawsze trwa **0,5 s**. Kofeina działa po ostatnim łyku. Po odstawieniu filiżanki sterowanie wraca. **Walka i wydobycie trwają podczas całej animacji.** P lub utrata aktywności okna pauzują również animację; R rozpoczyna nową ekspedycję.

Urządzenie ma podświetloną obudowę, manometr z ruchomą wskazówką, miedziane przewody, tacę ociekową oraz ekran CRT z nazwą kawy, tierem i fazą cyklu. Zasobnik pokazuje jedno z pięciu opakowań dostarczonych przez użytkownika. Kawa paruje, a poziom płynu w filiżance odpowiada fazie animacji.

## Rodzaje kawy

| Tier | Kawa | Rzadkość karty | Czujność po wypiciu |
| --- | --- | --- | --- |
| I | Terran Arabica | Wyposażenie początkowe | 5 min |
| II | Frontier Roast | Common | 6 min |
| III | Luna Nitro | Rare | 7 min |
| IV | Reactor Blend | Epic | 8 min |
| V | Pure Caffeine / Star Dust | Legendary | 10 min |

Każda filiżanka usuwa senność i rozpoczyna pełny czas czujności od nowa. Czas nie kumuluje się. Zdobycie karty zmienia zapas kawy w ekspresie, ale nie usuwa bieżącej senności — trzeba wypić nową filiżankę.

## Ulepszenia ekspresu

| Poziom | Karta | Cały cykl | Wygląd |
| --- | --- | --- | --- |
| I | Wyposażenie początkowe | 3 s | Jeden manometr i kranik |
| II | Podwójne ciśnienie — Rare | 2 s | Dodatkowy manometr, miedziana rurka i drugi kranik |
| III | Caffeine Gatling — Legendary | 1 s | Obrotowy, sześciolufowy minigun kawowy, dodatkowy reaktor i przewód |

Można od razu zdobyć dowolny wyższy tier kawy lub ekspresu. Ta sama karta i wszystkie niższe poziomy znikają z losowania. Dotyczy to także starszych ofert w kolejce oraz dodawania przez laboratorium. Karty mają stałe rzadkości, więc np. kawa II nie zajmuje slotu Legendary. Kawa i ekspres rozwijają się niezależnie. Nowa ekspedycja resetuje oba do poziomu I.

## Kod i sprawdzanie

- `native/coffee.hpp`: zegar, kary, mruganie, fazy i czasy.
- `native/command_game.hpp`: interakcja, kolizje, blokada sterowania, kary ruchu i obrotu.
- `native/expedition.hpp`: sześć kart, stałe rzadkości, eliminacja powtórek i downgrade'ów.
- `native/coffee_scene.hpp`: modele, filiżanka, płyn i opakowania.
- `native/coffee_effects.hpp`: dwukierunkowe rozmycie obrazu i powieki.
- `native/coffee_tests.cpp`: testy mechanik uruchamiane podczas przebudowy.
- `TEST_KAWY.bat`: automatyczna próba aplikacji, wszystkich opakowań i poziomów ekspresu, pauzy, blokady wejścia, zmiany rozmiaru i restartu. Raport oraz zrzuty: `artifacts/command/coffee/`.

Grafiki `public/assets/coffee-tier-1.png` do `coffee-tier-5.png` pochodzą z pięciu załączników użytkownika. Nazwy, kolory i przypisanie tierów zachowano. Model ekspresu i animacja powstają z geometrii 3D w istniejącym rendererze.
