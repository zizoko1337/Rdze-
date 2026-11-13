# Ekspedycja 2137 — przeciwnicy, bossowie, kryształy i jeździki

`START.bat` uruchamia cały nowy etap w zwykłej rozgrywce. Nowe reguły są w `native/frontier.hpp`, grafika i wybieranie kryształów w `native/frontier_scene.hpp`. Nadal obowiązują 4 bojowe typy wieżyczek, 2 typy wsparcia i 210 stanowisk. Fabryka jest modułem rdzenia i nie zajmuje żadnego z tych limitów.

## Napór i bossowie

| Czas aktywnej gry | Nowe zagrożenie |
| --- | --- |
| Start | 15 pełzaczy, następni stopniowo nadchodzą ze wszystkich stron |
| 1:00 | Łowcy: mniej HP, znacznie większa prędkość |
| 1:30 | Pierwsza elita, następne co 45 s; czterokrotne HP, większy sprite i złoty pierścień |
| 2:00 | Tarany: dużo HP, wolny marsz |
| 3:00 | Plujce: atakują rdzeń z dystansu; pierwszy Tytan |
| 4:00 | Latające widma |
| 6:00, 9:00, 12:00, 15:00… | Kolejne bossy, naprzemiennie Królowa Roju i Tytan |
| 10:00 | Dodatkowe hordy pełzaczy: 120 co 15 s, +30 w każdej kolejnej minucie (maks. 600 na falę) |

Tytan emituje uderzenie co 6 s, gdy dotrze w pobliże rdzenia. Królowa przywołuje pięciu łowców co 8 s. Boss ma alarm 10 s przed nadejściem. Po zabiciu bossa przez 15 s normalne grupy są mniejsze i rzadsze. Zwykli pełzacze zachowują 100 HP; pozostałe gatunki stopniowo wzmacniają się z czasem. Paski HP przeciwników, także bossów, pozostają ukryte.

Początkowy przyrost naporu jest łagodniejszy niż w pierwszej próbie tego etapu. Grupy rosną co 4 minuty, a odstęp między nimi maleje stopniowo, do 0,6 s. Standardowy napór ograniczony jest do 6000 jednostek; limit wspólnej symulacji wynosi 20 000.

## Kryształy i dron

Boss zawsze zostawia kryształ. Elita ma 1% szansy; zwykłe bestie na tym etapie nie zostawiają kryształów. Śmierć przetwarzana jest jednokrotnie dla każdego pokonanego przeciwnika, niezależnie od broni, jeździka, efektu czasowego czy oczyszczenia.

W widoku taktycznym kliknij kryształ. C albo przycisk panelu wysyła drona po najbliższy wolny kryształ. Kolejne wskazania tworzą kolejkę; ponowne kliknięcie tego samego ładunku nie dubluje zlecenia. Dron startuje z niskiego dokowiska po przeciwnej stronie rdzenia niż transport uranu. Najpierw wznosi się ponad dach, następnie leci do celu, schodzi, zbiera przez 2 s i wraca z kryształem. Nie przecina cylindra ani dachu. Ładunek widać pod dronem.

Nagroda powstaje dopiero po dostarczeniu kryształu. Uran działa niezależnie i nadal przyznaje wypłatę natychmiast po zapełnieniu paska. Kolejka nagród zawiera źródło i osobny, unikalny numer. Zamknięcie terminala nie losuje ponownie kart ani rzadkości.

Kryształ bossa daje trzy karty minimum Rare. Bez szczęścia rozkład pojedynczej propozycji wynosi 60% Rare, 30% Epic i 10% Legendary. Szczęście dodatkowo poprawia losowanie. Kryształ elity używa standardowego rozkładu rzadkości. Limity typów i stanowisk nadal wycofują niedostępne karty.

Loty, kryształy, jednostki, produkcja i napór zatrzymują się przy pauzie, terminalu, wybieraniu miejsca na wieżyczkę oraz po porażce. Radar na ekranie i telewizorze pokazuje też jeździki oraz kryształy.

## Produkcja jeździków

Fabryka jest jednorazową kartą od Uncommon. Uruchamia serię **5 jednostek co 12 s**, z pierwszą serią od razu po wznowieniu czasu. Każda maszyna ma sprite od tyłu, szuka najbliższego przeciwnika, dojeżdża do niego z prędkością 24 m/s i wybucha w promieniu **4 m**, za **180 obrażeń w centrum**. Bossowie otrzymują 55% obrażeń; obrażenia maleją z odległością od środka wybuchu. Wspólny cel może zwiększać otrzymywane obrażenia. [Aktualny balans](BALANS_WIEZYCZEK.md).

Maszyny mają sprite 1,75 m (połowa wcześniejszego rozmiaru) i wychodzą we wszystkich kierunkach, na trzech bliskich pierścieniach wokół rdzenia. Omijają cylinder rdzenia. Niewielkie różnice prędkości i boczne korekty rozluźniają kolumny. Raz na klatkę budowana jest siatka ruchu o komórkach 3 m: pojazd sprawdza zagregowane pozycje w dziewięciu komórkach, bez porównywania każdej pary samochodów. Bufory są rezerwowane z góry. Po śmierci celu szukają nowego. Numer generacji przeciwnika chroni przed pościgiem za nową jednostką, która wykorzystała ten sam identyfikator. Jednostki bez celu czekają i ponawiają wyszukiwanie; po 60 s znikają. Limit aktywnych maszyn to 2048, seria do 128, a maksymalne tempo produkcji to jedna seria co 0,5 s.

Hordy od 10:00 uzupełniają zwykły napór. Pełzacze nadal mają 100 HP. Każda fala nadchodzi z trzech zmieniających się kierunków i powstaje stopniowo: najwyżej 12 jednostek na klatkę, co najmniej 0,08 s między porcjami. Obowiązuje wspólny limit 6000 aktywnych przeciwników; nadwyżka czeka w ograniczonej kolejce. Przerwa po pokonaniu bossa wstrzymuje wypuszczanie hord.

Po odblokowaniu fabryki do puli dochodzą dwie karty:

| Rzadkość | Większa seria | Szybsza linia |
| --- | --- | --- |
| Common | +1 jednostka na serię | +10% bazowego tempa |
| Uncommon | +2 | +20% |
| Rare | +3 | +30% |
| Epic | +4 | +40% |
| Legendary | +5 | +50% |

Bonusy sumują się względem bazowego tempa. Przykład: +50% oznacza serię co 8 s, nie co 6 s. Nowa karta ilości działa od następnej serii. Karty produkcji przestają się losować po dojściu do odpowiedniego limitu.

Sprite jest prawdziwym PNG z przezroczystością: [rover-rear.png](public/assets/rover-rear.png). Wygenerowano go wbudowanym `image_gen`; pełny prompt i pochodzenie: [ASSETS-JEZDZIK.md](ASSETS-JEZDZIK.md). Jednostki rysowane są jednym dodatkowym wywołaniem z instancjonowaniem, a nie osobnym wywołaniem dla każdej maszyny.

## Próby i pierwszy balans

`POLIGON_EKSPEDYCJI.bat` startuje od 2:30 z 20 000 HP, czterema działami do rozmieszczenia i sześcioma gotowymi wyborami kart. Fabryka, radar i rozwój produkcji są w pierwszych ofertach. To osobny wariant do prób; normalna ekspedycja zaczyna od 0:00, bez fabryki i gotowych nagród.

`frontier-tests.exe` sprawdza wszystkie rzadkości produkcji, rzeczywiste serie, pojedynczy cel eksplozji, omijanie rdzenia, ponowne namierzanie i limit 2048 jednostek. Sprawdza też statystycznie 1% elit, gwarantowany kryształ bossa, brak duplikowania śmierci, ręczne zlecanie i kolejkę, pauzę transportu, nagrodę dopiero po powrocie, minimum Rare, poprawę szans oraz harmonogram gatunków i bossów.

`TEST_EKSPEDYCJI_02.bat` przechodzi przez prawdziwe zdarzenia klawiatury i myszy: podejście do konsoli, odblokowanie fabryki, wybór większej serii, podniesienie działa, strzał do osłabionego testowo bossa, kliknięcie upuszczonego kryształu, pauzę lotu i odbiór karty przy komputerze. Test skraca oczekiwanie przez zmianę zegara, HP bossa i przygotowane karty. Raport oraz zrzuty: `artifacts/command/frontier/`.

`TEST_BALANSU.bat` symuluje około 15 minut ze standardowym startem, realnym wydobyciem, losowanymi nagrodami i dostawami. Referencyjny automat dokładnie celuje, natychmiast korzysta z komputera, wybiera premie według stałych priorytetów i wysyła drona po kryształy. To optymistyczna kontrola tempa, nie pełny balans człowieka. JSON i tabela co minutę trafiają do `artifacts/frontier/`. Zwykła gra nie ma automatycznego celowania ani automatycznej obsługi komputera.

Rozwój między podejściami, zapis gry, stałe odblokowania i pokazywanie HP przez osobną kartę nadal pozostają na później.
