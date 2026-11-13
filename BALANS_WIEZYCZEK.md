# Role automatycznej obrony

Opis dotyczy automatycznych wieżyczek i jeździków. W późniejszej zmianie manualna lanca otrzymała tę samą bazową siłę ciągłej wiązki: 1650 DPS. Zachowuje własne ładowanie 1,8 s, zasięg 550 m i bonus 200 obrażeń pierwszego kontaktu; jej DPS jest powiązany w kodzie z bazową wartością automatycznej lancy.

## Parametry bazowe

| Typ | Zasięg | Obrażenia | Tempo | Zastosowanie |
| --- | ---: | ---: | --- | --- |
| Laser | 80 m | 40 | Co 0,4 s: 100 DPS | Uniwersalny, większy zasięg i szybsze przenoszenie ognia niż minigun |
| Minigun | 55 m | 12 | Co 0,08 s: 150 DPS po rozkręceniu | Większy DPS kosztem zasięgu i wolniejszego obracania głowicy |
| Palnik | 32 m | 220 DPS + 20 DPS podpalenia | Stożek, trafienia co 0,1 s; podpalenie 3 s | Mocny przeciw grupom blisko stanowiska; ograniczony zasięgiem |
| Snajper | 210 m | 1000 | Co 3,5 s | Cel pojedynczy; priorytet boss → elita → zwykły |
| Toksyna | 85 m | 8 przy trafieniu + 32 DPS na dawkę | Pocisk co 1,8 s | Kumulowane zatrucie i 35% spowolnienia; wymaga czasu na narastanie obrażeń |
| Rakiety | 125 m | 180 w centrum, spadek do 55% na brzegu | Co 1,2 s; wybuch 4 m | Małe skupiska. Bossowie dostają dodatkowo tylko 55% obrażeń |
| Moździerz | 180 m | 600 w centrum, spadek do 55% na brzegu | Co 3,8 s; wybuch 16 m | Duży obszar i zasięg; opóźnienie lotu 1,3 s + dystans/65 |
| Lanca | 130 m | 1650 DPS podczas wiązki | 6 s ładowania + 3 s ostrzału + 2 s chłodzenia | Priorytet boss → elita → zwykły; bardzo wolny obrót |
| Tesla | 60 m | 160 zwykłym, 56 elitom, 16 bossom | Co 1,8 s, wszystkie cele w zasięgu | Oczyszcza hordy; zwykłym ocalałym daje 0,15 s ogłuszenia |

To wartości przed kartami i wsparciem. Jedna dawka podpalenia jest odświeżana; dawki zatrucia wieżyczek sumują się. Podpalenie i zatrucie wzmacniają właściwe karty obrażeń.

## Namierzanie i obrót

Głowica automatycznej lancy nawiązuje do manualnej: dwie ciemne szyny z wewnętrznymi pasami energii, trzy obracające się pierścienie, metalowe żebra chłodzenia i odsłonięty kanał wiązki. Szyny i pierścienie rozsuwają się w miarę ładowania. Ten sam model jest używany na stanowisku, podczas dostawy i w podglądzie rozmieszczania.

Laser obraca się do 10 rad/s, minigun do 3 rad/s, snajper do 1,4 rad/s, a lanca do 0,25 rad/s (około 14,3°/s). Broń pojedynczego celu musi najpierw ustawić lufę na celu. Palnik zadaje obrażenia w rzeczywistym stożku swojej głowicy, a Tesla razi dookoła.

Snajper i lanca sprawdzają priorytet co około 0,15 s przez istniejącą siatkę przestrzenną. Nowy boss przejmuje celowanie nawet wtedy, gdy dotychczasowa elita żyje. Cel tej samej klasy jest utrzymywany, aby ograniczyć bezcelowe obracanie. Śmierć lub utrata zasięgu uruchamia ponowne namierzanie. Ręczne oznaczenie kartą Wspólny cel ma pierwszeństwo; po jego zakończeniu wraca zwykły priorytet.

Lanca może ładować się podczas obracania, ale wiązka zadaje obrażenia tylko przy prawidłowym ustawieniu. Rozpoczęta wiązka pozostaje widoczna przez pełne 3 s, także po zabiciu celu, podczas szukania kolejnego i obracania. Bez celu strzela przed lufę, zachowując ostatnie nachylenie wiązki. Nowy cel może przejąć pozostałą część ostrzału bez ponownego ładowania. Czas nie odnawia się przy zmianie celu; po 3 s zaczyna się chłodzenie. Pauza zatrzymuje cały cykl, a wyłączenie wieżyczki przez przeciwnika nadal wstrzymuje jej działanie.

Nominalny średni DPS lancy to `1650 × 3 / (6 + 3 + 2) = 450`, czyli trzy miniguny po 150 DPS. Automatyczny pomiar 330 s na nieruchomym, wytrzymałym celu sprawdza rzeczywisty wynik, z rozkręcaniem miniguna, ładowaniem lancy i stałym krokiem symulacji. Ruch celu i obracanie lancy mogą ten wynik obniżyć.

## Zatrucie i koszt CPU

Trafienie automatycznej toksyny nakłada jedną dawkę na każdy cel w obszarze 9 m. Trafienia kolejnych pocisków dodają kolejne dawki, również z innych wieżyczek. Każda wnosi bazowo 32 DPS przez około 6–6,5 s; wygaszanie jest grupowane w półsekundowych przedziałach.

Na przeciwniku znajduje się 13 stałych liczników wygasania i jedna suma DPS. Dawka dopisuje obrażenia do właściwego przedziału. Podczas symulacji odejmujemy wygasłe przedziały i wykonujemy jedno naliczenie sumarycznych obrażeń. Nie ma listy obiektów dawek, osobnych timerów ani alokacji na trafienie. Śmierć i ponowne użycie miejsca w puli zerują zatrucie; pauza zatrzymuje czas.

Chmura żyje 7 s i sprawdza obszar co 0,25 s. Spowalnia o 35% i zatruwa przeciwników, którzy nie mają dawki. Jej kolejne odświeżenia nie kumulują trucizny na już zatrutych celach. Stosy pochodzą z rzeczywistych trafień pocisków.

Zatrucie automatycznej toksyny jest oddzielone od wcześniejszego zatrucia działa manualnego. Dzięki temu nowe sumowanie nie zmienia manualnej katapulty ani jej legendarnego kleju. Jeżeli oba spowolnienia działają równocześnie, obowiązuje mocniejsze; aury chłodzenia nadal działają według zasad wsparcia.

## Fabryka jeździków

- Bazowo **5 maszyn co 12 s**; pierwsza seria rusza po zdobyciu fabryki i wznowieniu czasu.
- Trafienie powoduje wybuch o promieniu **4 m**, za **180 obrażeń w centrum**, z takim samym spadkiem obszarowym i mnożnikiem 55% przeciw bossom jak automatyczna rakieta.
- Wybuch jest liczony przez siatkę przestrzenną. Segmenty żmija nie zwielokrotniają obrażeń jego wspólnej puli HP.
- Karty liczebności nadal dają +1/2/3/4/5 maszyn na serię, a karty tempa +10/20/30/40/50% bazowego tempa produkcji. Wypadają po zdobyciu fabryki.
- Pozostają ograniczenia 128 maszyn na serię, mnożnik tempa do 24 i 2048 aktywnych maszyn. Ruch omija rdzeń i używa zagregowanej siatki ruchu.
- Obrażenia maszyn nie są dopisywane do statystyk rakiet ani do ich specjalizacji. Wieżyczki i fabryka zachowują oddzielny rozwój.

## Weryfikacja

`native/tower_balance_tests.cpp` sprawdza role starterów, priorytety, wspólny cel, utratę celu, ograniczenie obrotu lancy, rzeczywisty średni DPS, obrażenia Tesli według klasy, rakiety, stosy i wygasanie toksyny, spowolnienie, pauzę, ponowne użycie indeksu oraz obszarowe trafienia jeździków. Test 6000 jednocześnie zatrutych celów mierzy czas kroku bez renderowania i sprawdza stały adres pamięci puli.

Testy mechanik nie gwarantują jeszcze oceny trudności całej ekspedycji. Po tej zmianie warto porównać zestawy przeciw tej samej fali i sprawdzić rozgrywkę człowieka, zwłaszcza kombinację spowolnienia, Tesli i skupionego ostrzału bossów.
