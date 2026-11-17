# Progresja przeciwników i łącze z bazą

Pierwsza seria bossów nadchodzi po 3 minutach wydobycia. Kolejne odstępy to **150, 120, 90, 60 i 30 sekund**, następnie stale 30 sekund. Początki serii: **3:00, 5:30, 7:30, 9:00, 10:00, 10:30, 11:00…** Ostrzeżenie pozostaje 10 sekund wcześniej. Gra nie czeka na śmierć poprzedniego bossa. Osiem gatunków zachowuje dotychczasową rotację. Licznik serii jest oddzielony od liczby bossów, więc dodanie jednostki do serii nie przyspiesza harmonogramu.

## Losowanie mutacji

Pierwszy cykl przypada po 180 sekundach aktywnej rozgrywki, następne bazowo co 180 sekund. Pauza, terminale i wybór celu zatrzymują czas. Zwykły widok taktyczny pozwala mu płynąć. Losowania mają osobny generator, niezależny od nagród gracza; szczęście gracza nie wpływa na przeciwników. Stałe mutacje kumulują się do końca ekspedycji i zerują po rozpoczęciu nowej. Efekty czasowe wygasają po podanym czasie aktywnej gry.

| Rzadkość | Szansa | HP wszystkich jednostek | Ilość w zwykłych seriach i hordach | Bazowa prędkość | Obrażenia rdzenia | Krótszy odstęp mutacji |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Common | 40% | +10% | +10% | +3% | +8% | −5 s |
| Uncommon | 32% | +15% | +15% | +5% | +12% | −8 s |
| Rare | 18% | +22% | +25% | +8% | +18% | −12 s |
| Epic | 8% | +32% | +35% | +12% | +25% | −18 s |
| Legendary | 2% | +45% | +50% | +18% | +40% | −25 s |

Najpierw losowana jest rzadkość, potem jedna dostępna mutacja tej rzadkości. Pięć rodzajów procentowych występuje na każdym poziomie; rzadkie poziomy mają również zdolności z tabeli poniżej. Niedostępne gatunki i zdobyte unikalne zdolności nie zajmują losowania.

Premie HP i prędkości obejmują jednostki już obecne oraz przyszłe, także bossów i przywołania. HP zachowuje procent aktualnego zdrowia. Premie HP sumują się względem bazowego zdrowia danego egzemplarza, uwzględniającego dotychczasowe skalowanie czasem. Dzieci Śluzowca nie dostają tej samej mutacji drugi raz przy podziale. Mnożnik obrażeń działa na uderzenia, kwas i zdolności atakujące rdzeń.

Przyrost liczby jednostek zachowuje ułamki między seriami: +10% przy grupach jednoosobowych daje jedną dodatkową jednostkę na dziesięć serii, a nie podwojenie każdej grupy. Osobny licznik obsługuje hordy. Oddech po bossie nadal ogranicza zwykłą grupę do jednej jednostki i wstrzymuje hordy.

Limity: odstęp mutacji nie spada poniżej **60 s**, prędkość nie przekracza **2,5×**, obrażenia i liczebność **4×** ich bazowych wartości. Premia HP może dalej rosnąć. Niedostępne po osiągnięciu limitu premie znikają z puli. Ograniczenia 6000 standardowych aktywnych przeciwników, 20 000 slotów symulacji i kolejki hord nadal obowiązują.

## Zdolności gatunkowe i specjalne

| Rzadkość | Mutacja | Efekt |
| --- | --- | --- |
| Common | Przerośnięty przywódca | Tylko następny boss dostaje 200% bazowego HP, czyli bazę zdrowia ×2 |
| Uncommon | Pęd przywódcy | Tylko następny boss porusza się ×2 szybciej |
| Uncommon | Szybki zwiad | Łowcy poruszają się dodatkowo o 20% szybciej |
| Uncommon | Chitynowa tarcza | Tarany redukują otrzymywane obrażenia o 20% |
| Rare | Samonaprawa tkanek | Tarany regenerują 1% maksymalnego HP/s |
| Rare | Gruczoły kwasowe | Plujce atakują co 2,1 s zamiast 3 s, oblężnicze co 3,5 s zamiast 5 s |
| Rare | Inwazja sabotażystów | Przez 60 s każda nowa elita zostaje zastąpiona Sabotażystą, także elity z Elitarnego potomstwa. Zwykłe jednostki, bossowie i już obecne elity pozostają bez zmian. Harmonogram i liczba pojawień nie zmieniają się |
| Epic | Odruch zwiadowców | Łowcy, także elitarni, mają 50% szans na unik bezpośredniego trafienia. Unik nie zatrzymuje obszaru, trucizny, podpalenia, taranowania ani usług; jest oznaczany błękitnym efektem |
| Epic | Pole widmowe | Widma dalej niż 80 m od rdzenia otrzymują o 35% mniej obrażeń |
| Epic | Szaleństwo rannych | Zwykłe jednostki i elity poniżej 30% HP dostają mnożnik ruchu ×1,5 |
| Legendary | Sygnał przywódców | +1 boss w każdej przyszłej serii. Można powtarzać do 8 bossów w serii; kolejne jednostki wychodzą co 2 s |
| Legendary | Podwójna adaptacja | Od następnego cyklu losowane są dwie mutacje, każda z osobną rzadkością |
| Legendary | Elitarne potomstwo | Nowe jednostki zwykłych serii i hord mają 5% szans na elitarną odmianę, oprócz dotychczasowego harmonogramu elit |

Zdolności poza dodatkowym bossem, mutacjami następnego bossa i czasową Inwazją sabotażystów są unikalne. Występują dopiero po wejściu właściwego gatunku do rozgrywki. Inwazja nie może wylosować się ponownie podczas trwania; po wygaśnięciu wraca do puli Rare. Sabotażyści pozostają na mapie po zakończeniu minuty, ale kolejne elity znowu są dobierane normalnie. Kapitan podaje w meldunku, że efekt trwa 60 s.

Przerośnięty przywódca i Pęd przywódcy czekają bez limitu czasu na jednego nowego bossa. Nie wpływają na bossów już obecnych i nie zmieniają harmonogramu. Każdy efekt osobno zużywa się po udanym pojawieniu bossa; przy kilku bossach w serii obejmuje tylko pierwszego. Jeśli oba czekają jednocześnie, otrzyma je ten sam boss. Ten sam efekt nie może ponownie się wylosować przed zużyciem, ale potem wraca do swojej puli. Nieudane pojawienie z powodu pełnej puli nie zużywa efektu. Przywołanie bossa z bestiariusza również go zużywa; statystyki karty pokazują premię oczekującą na następne przywołanie.

Premie jednego bossa mnożą jego zwykłą bazę po skalowaniu spotkania; następnie naliczane są stałe mutacje roju. Przykład: boss z bazą 7000 HP dostaje 14000 HP, a przy dodatkowej globalnej premii +22% ma 17080 HP. Późniejsze mutacje HP i prędkości zachowują jego jednorazowe premie oraz procent pozostałego życia. Części Śluzowca są kontynuacją tego samego bossa: dziedziczą premie bez mnożenia ich ponownie i nie zużywają efektu dla następnej walki. Podwojona prędkość dotyczy też ruchu bocznego bossów segmentowych. Globalny limit prędkości ×2,5 nadal ogranicza premie stałe; osobny mnożnik jednego bossa działa dodatkowo.

## Komunikaty kapitana

Połączenie z bazą pojawia się w **lewym dolnym rogu HUD-u wyłącznie podczas ogłaszania mutacji**. Panel pokazuje **„PRZECIWNICY MUTUJĄ”**, nazwę, dokładny efekt i rzadkość oznaczoną kolorem. Meldunek trwa 9 s, potem kapitan rozłącza się i cały panel znika. Dwa jednoczesne losowania są kolejkowane; po ostatnim meldunku nie pozostaje portret, licznik ani stały panel połączenia. Kapitan przełącza zgodne portrety z otwartymi i zamkniętymi ustami w rytmie sylab, a nowe losowanie uruchamia krótki sygnał radiowy. Pauza zatrzymuje meldunek i animację; otwarte menu nie zasłania własnych kontrolek portretem.

Grafiki: `public/assets/captain-closed.png` i `public/assets/captain-open.png`. [Pochodzenie i pełne prompty](ASSETS-KAPITAN.md). Implementacja: `native/enemy_mutations.hpp`, `native/captain_comms.hpp`, `native/frontier.hpp`, `native/gameplay.hpp`. Testy mechanik: `native/enemy_mutation_tests.cpp`, `native/next_boss_mutation_tests.cpp`. Lokalne podglądy pięciu rzadkości i obu klatek twarzy: `artifacts/mutations/`.

## Brimstone

Brimstone zadaje **25 000 nominalnych obrażeń łącznie** w sześciu impulsach co 0,5 s, po ostrzeżeniu 3 s. Promień wynosi 10 m. Wskazanie bossa lub ziemi przy nim blokuje wiązkę na jego identyfikatorze i generacji; wiązka śledzi jego pozycję. Po śmierci celu pozostaje w ostatnim punkcie, bez przechwytywania nowego przeciwnika w ponownie użytym slocie.

Usługa nie podlega unikom ani osłonom gatunkowym. Bonus wspólnego celu nadal może zwiększyć obrażenia. Pełna salwa wystarcza na bossów z HP do 25 000; boss, który przekroczy ten próg przez późniejsze skalowanie lub mutacje, może ją przeżyć. Nie ma ukrytej egzekucji omijającej jego HP.
