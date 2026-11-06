# Walidacja poligonu arsenału — 2026-11-02

Gotowa kompilacja: `native/build/rdzen-game.exe`. Główny launcher: `START.bat`.

## Wykonane sprawdzenia

- Pełna kompilacja Release gry i wcześniejszego laboratorium, bez błędów i ostrzeżeń kompilatora.
- Testy reguł: budowanie i kolizje, usuwanie, obrażenia wszystkich dziewięciu broni, zasięg i odnowienie Tesli, ładowanie i czas działania lancy, stożek płomienia i podpalenie, trucizna i wygasanie chmur, łuk i eksplozja moździerza, utrzymywanie celu oraz bezpieczne ponowne użycie miejsca w puli.
- Testy fal, dwóch odrębnych bossów i przywoływania pomocników przez Królową.
- Dziesięć sekund symulacji 5000 bestii i 90 wieżyczek różnych typów: 2843 eliminacje, skończone współrzędne, stabilna pula przeciwników.
- Wcześniejsze testy laboratorium: 10 000 zapytań siatki porównanych z pełnym przeszukiwaniem oraz 10 sekund z 5000 bestii i 50 000 poruszających się pocisków. Wszystkie przeszły.
- **Końcowy test przez rzeczywisty `START.bat --smoke-test`**: zbudowano dziewięć wieżyczek przez zdarzenia myszy i klawiatury, trzykrotnie zmieniono kamerę, pauza zatrzymała symulację, każdy typ broni strzelił, zmieniono okno z 1920×1080 na 1280×720. Wynik: PASS.
- Oceniono zrzuty widoku taktycznego i pierwszej osoby oraz interfejsu w mniejszym oknie. Poprawiono obrót myszy w poziomie oraz interpretację danych znaków w shaderze. Paski HP przeciwników nie są rysowane.

Wynik automatyzacji: [smoke-result.json](artifacts/arsenal/smoke-result.json). Zrzuty: [pokład](artifacts/arsenal/03-deck-combat.png), [widok taktyczny](artifacts/arsenal/02-tactical-combat.png), [mniejsze okno](artifacts/arsenal/04-small-window.png).

## Pomiar rzeczywistego arsenału

Lokalny komputer z GTX 1660 SUPER, rozdzielczość 1920×1080. 99 wieżyczek różnych typów, dwa początkowe bossy, populacja bestii uzupełniana raz na sekundę do 5000. Trzy sekundy rozgrzewki i około 15 sekund pomiaru. Kamera taktyczna, VSync i dźwięk wyłączone.

| Metryka | Wynik |
| --- | --- |
| Aktywne bestie w próbkach | 3310–5000 |
| Wieżyczki | 99 |
| Średnia przepustowość renderowania | 1664,56 klatki/s |
| p99 czasu klatki | 1,1145 ms |
| Najdłuższa zmierzona klatka | 3,4404 ms |
| Średni krok symulacji | 0,20688 ms |
| Średni czas GPU | 0,450196 ms |
| Eliminacje w przebiegu | 8728 |

Źródła: [stress-result.json](artifacts/arsenal/stress-result.json), [surowe klatki](artifacts/arsenal/stress-frames.csv), [zrzut sceny](artifacts/arsenal/stress-5000.png).

To krótki pomiar konkretnej sceny, a nie gwarancja wydajności przyszłej gry. Liczba bestii spada między uzupełnieniami wskutek eliminacji. Ten scenariusz nie utrzymuje 50 000 ani 100 000 jednoczesnych pocisków — takie sztuczne obciążenie sprawdza oddzielne laboratorium. Zrzuty wykonywane podczas testu są wyłączone z próbek czasu klatki. Wysokie FPS bez VSync oznacza zapas obliczeniowy, nie częstotliwość odświeżania monitora. Zwykła gra startuje z VSync.

## Powtórzenie

- `PRZEBUDUJ.bat` — kompilacja i testy obu symulacji.
- `TEST_ARSENALU.bat` — test interakcji i pomiar nowego arsenału.
- `BENCHMARK.bat` — wcześniejsze scenariusze dużej liczby pocisków.

Raporty i zrzuty są nadpisywane przy kolejnym uruchomieniu danego testu. Podczas automatycznego testu nie steruj oknem gry.
