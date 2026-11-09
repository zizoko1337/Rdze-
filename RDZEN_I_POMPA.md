# Regeneracja i centrum dowodzenia

Rdzeń regeneruje bazowo **3 HP/s** od rozpoczęcia wydobycia. Regeneracja działa także w walce, zatrzymuje się podczas pauzy i otwartych menu, nie przekracza maksymalnego HP i nie wskrzesza zniszczonego rdzenia. Śmiertelne obrażenia mają pierwszeństwo. Przekroczenie progu awaryjnego oczyszczenia pozostaje wykrywane także wtedy, gdy regeneracja zaraz podnosi HP ponad próg.

Powtarzalne ulepszenie **Nanoboty regenerujące** trafia do losowania i do laboratorium pod `;` (wyszukaj „nanoboty”).

| Rzadkość | Dodatkowa regeneracja na kopię |
|---|---:|
| Common | +0,5 HP/s |
| Uncommon | +1 HP/s |
| Rare | +1,5 HP/s |
| Epic | +2 HP/s |
| Legendary | +3 HP/s |

Obecna regeneracja jest widoczna na HUD, komputerze ulepszeń i panelu pompy.

Oba komputery są skierowane ekranami w stronę środka platformy. Lewy jest jasną stacją badawczą z szerokim monitorem, klawiaturą, licznikiem nagród na ekranie i zieloną lampką o płynnie pulsującej jasności. Prawy ma cięższą niebieską obudowę, boczny panel przełączników i ekran z liczbą punktów oraz listą usług. Nadal obsługuje się je klawiszem E. Kolizje odpowiadają obrotowi obudów.

Pompa jest po przeciwnej stronie działa niż przycisk startowy, zgodnie ze szkicem: środek `(0, -10,5)` na pokładzie. Dwa tłoki poruszają się w przeciwfazie, korby i zębate koła obracają się, a odsłonięte sprężyny ściskają się i rozciągają między stałym dolnym mocowaniem a ruchomą płytką tłoka. Osobna tabliczka nad klawiaturą została usunięta. Mechanizm uruchamia się wraz z wydobyciem i zatrzymuje podczas pauzy. Panel pokazuje postęp aktualnej partii uranu, tempo wydobycia i regenerację. **Stabilność to procent obecnego HP rdzenia**, bez osobnej nowej statystyki. Analogowe zegary pokazują pulsujące ciśnienie robocze i integralność rdzenia.

Modele składają się z instancjonowanych brył; wyświetlacze wykorzystują istniejący atlas fontu. Nie dodają kamer ani symulacji fizyki tłoków i sprężyn.

`native/core_tests.cpp` sprawdza regenerację, limit HP, śmierć, pauzę i menu, wszystkie rzadkości nanobotów, sumowanie premii, próg oczyszczenia, kierunek komputerów i kolizje. `Rdzen.exe --core-test` sprawdza faktyczną regenerację, wyszukanie i instalację nanobotów, pauzę menu oraz zapisuje widoki obu komputerów i pompy.

Radar HUD i telewizor są domyślnym wyposażeniem od wejścia na platformę. Nie istnieje już karta radaru. Panel HUD zmniejszono z 328 × 273 do 212 × 204; znaczniki obracają się z kamerą, więc kierunek patrzenia zawsze jest na górze, również podczas chodzenia, celowania działem i obracania kamery taktycznej. Zasięg pozostaje 350 m. Telewizor orientuje mapę względem kierunku operatora. Znaczniki są nadal rysowane w jednym przebiegu instancjonowania.

Telewizor radaru przeniesiono na godzinę 2 (obrót o 60° w prawo względem wcześniejszego miejsca) i skierowano do środka. Nie nachodzi na radio. Osiem lamp na słupach pokazuje stan rdzenia: zielony powyżej 50% HP, żółty powyżej 20% do 50%, czerwony przy 20% lub mniej. Poniżej 20% pojawia się lekkie drżenie kamery operatora, zatrzymywane podczas pauzy i menu. Bossowie mają osobne paski HP nad sprite’ami, rysowane w jednej wspólnej partii.
