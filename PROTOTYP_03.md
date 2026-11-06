# Centrum dowodzenia — walidacja 2026-11-03

Gotowa kompilacja: `native/build/rdzen-game.exe`. `START.bat` uruchamia wybór działa pokładowego, `ARSENAL.bat` wcześniejszy poligon automatycznych wieżyczek.

## Wyniki

- Kompilacja Release zakończona bez błędów i ostrzeżeń.
- `command-tests.exe`: PASS. Sprawdzono wybór broni, podejście do fotela, blokadę interakcji z daleka, otwieranie włazu przed podnoszeniem, pauzę windy, wejście i wyjście, przerwanie podnoszenia, blokadę chodzenia podczas siedzenia oraz powrót w poprzednie miejsce.
- Laser: potwierdzone zabicie pełzacza jednym trafieniem, trafienie pierwszego celu na linii, sekundowe odnowienie, brak strzałów podczas pauzy, możliwość chybienia i ograniczony zasięg.
- Minigun: pociski mają czas lotu, różne kierunki wskutek rozrzutu, obrażenia przy przecięciu celu i ograniczony czas życia. Sprawdzono przecięcie szybkiego pocisku z celem między krokami symulacji.
- Sprawdzono mnożniki parametrów modułu broni i spokojny napływ pełzaczy bez wzrostu ich HP, bossów ani automatycznych wieżyczek.
- Końcowy test przez `START.bat --command-test`: PASS. Menu obsłużone kliknięciami, podejście przez wejście klawiatury, E, podniesienie nad dach, celowanie myszą, 2 strzały lasera, 1 trafienie i eliminacja, pauza, E i powrót na pokład. Dodatkowo sprawdzono zmianę rozmiaru okna i brak reaktywacji budowania dawnymi skrótami.
- Końcowy test przez `START.bat --command-test --minigun`: PASS. Ten sam cykl z minigunem: 30 pocisków, 4 trafienia, 1 eliminacja. Cel testowy jest nieruchomy; test nie ocenia trudności celowania dla człowieka.
- Regresja `--smoke-test` wcześniejszego arsenału: PASS — 9 zbudowanych wieżyczek, 3 przełączenia kamer, działająca pauza i wszystkie 9 typów broni strzelają. Testy symulacji arsenału również przeszły.
- Oceniono zrzuty menu, interakcji przy fotelu, przejazdu przez dach oraz widoków celowania. Po kontroli poprawiono wysokość kamery względem broni i widoczność bocznej korbki.

Raporty: [laser](artifacts/command/laser/result.json), [minigun](artifacts/command/minigun/result.json). Pole `fixture_dead` jest tylko diagnostyką aktualnej zawartości indeksu puli; po eliminacji indeks może już należeć do kolejnej bestii. Liczniki trafień i eliminacji oraz testy jednostkowe weryfikują efekt strzałów.

Zrzuty: [wybór broni](artifacts/command/laser/01-selection.png), [fotel](artifacts/command/laser/02-seat-prompt.png), [laser](artifacts/command/laser/04-gunnery.png), [minigun](artifacts/command/minigun/04-gunnery.png).

Automatyczne próby wyłączają dźwięk i VSync. Nie stanowią nowego benchmarku dużej liczby jednostek. `TEST_DZIALA.bat` powtarza oba przebiegi i nadpisuje ich raporty.
