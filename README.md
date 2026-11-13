# RDZEŃ — Centrum dowodzenia

Natywny prototyp na Windows, C++20 + SDL3 + Direct3D 11. Świat 3D i przeciwnicy jako sprite’y 2D, bez Unity, Godota i przeglądarki.

## GitHub i drugi komputer

Repozytorium zawiera kod, grafiki i skrypty budowania. Gotowy `Rdzen.exe` oraz `Rdzen-prototyp.zip` są dołączane do wydań w zakładce **Releases**; pliki wykonywalne, lokalne kompilatory i wyniki testów nie trafiają do historii Git.

Na drugim laptopie z Windows 10/11 x64 możesz pobrać samo EXE z Releases i uruchomić je dwuklikiem. Do dalszego programowania sklonuj repozytorium przez **Code → Clone**, a następnie uruchom **ZROB_EXE.bat**. Pierwsza kompilacja pobierze lokalnie kompilator i SDL z oficjalnych wydań GitHub oraz zweryfikuje ich SHA256. Nie trzeba instalować Unity, Godota ani Node.js do natywnej wersji gry. Pierwsze budowanie wymaga internetu; uruchamianie gotowej gry działa offline.

Jeśli używasz tego samego konta GitHub, na drugim komputerze wystarczy klonowanie. **Fork** tworzy osobne repozytorium na innym koncie; również można je potem sklonować. Starsza wersja przeglądarkowa jest zachowana w `src/`, lecz główna wersja gry znajduje się w `native/`.

## Uruchomienie i pierwsza próba

**Samodzielny plik: `Rdzen.exe`.** Uruchom go dwuklikiem; możesz przenosić sam EXE bez pozostałych folderów projektu. Ma wbudowany program gry, SDL i wszystkie grafiki. Przy pierwszym starcie zapisuje dane w `%LOCALAPPDATA%\Rdzen\Prototyp\<wersja>`; następne starty korzystają z tej kopii. Nie otwiera konsoli. Wersja do spakowanego przesłania: `Rdzen-prototyp.zip`.

`ZROB_EXE.bat` przebudowuje aktualną grę, uruchamia testy i odtwarza samodzielny EXE oraz ZIP. Sam skrypt `native/package.ps1` pakuje istniejący plik z `native/build`; po zmianach kodu użyj wariantu z przebudową. Wcześniejsze pliki BAT nadal działają.

Kliknij **Rdzen.exe** lub **START.bat**. Wybierz jeden z dziewięciu modułów manualnych (myszą albo 1–9), następnie naciśnij **Enter** lub przycisk rozpoczęcia ekspedycji. Broń platformy wydobywającej uran z obcej planety w roku 2137.

Zaczynasz na pokładzie rdzenia, pod dachem centrum dowodzenia. **Najpierw obejdź działo i podejdź do czerwonego przycisku za nim. E uruchamia wydobycie i napór przeciwników.** Przycisk wciska się i zjeżdża pod pokład. Zapadnia pozostaje otwarta, a z jej wnętrza wyjeżdża **cyber-jukebox URANIUM FM 2137**. Przed uruchomieniem możesz spokojnie chodzić po platformie; czas ekspedycji i wydobycie stoją.

Po wysunięciu radia muzyka rusza automatycznie. Na urządzeniu widać stację i nazwę utworu; membrany i wskaźniki reagują na odtwarzany dźwięk. Podejdź i naciśnij **E**: strzałki lewo/prawo zmieniają utwór, **Spacja** wstrzymuje/wznawia, **-/+** zmieniają głośność. Te same funkcje są dostępne myszą. E/Esc zamyka panel. Na razie jest jedna stacja z czterema dostarczonymi utworami. [Opis radia](JUKEBOX.md).

Podejdź do fotela na środku. Gdy patrzysz na stanowisko z bliska, pojawia się **Press E to control**. Naciśnij **E**: zajmiesz fotel, właz w dachu się rozsunie, a platforma z działem podniesie się o 10 m.

Celuj myszą i strzelaj **LPM** — można go przytrzymać. Ponowne **E** opuszcza stanowisko na pokład i wysadza operatora w miejscu, z którego wsiadł. **R** wraca do wyboru broni i rozpoczyna nowe podejście, zerując dotychczasowy rozwój.

## Uran, transporty i komputer

Rdzeń zaczyna z **1500 HP**, wydobywa **2 jednostki uranu/s**, a pasek mieści **100 uranu**. Każde zapełnienie paska natychmiast daje **jedną wypłatę: wybór 1 z 3 różnych kart**. Pierwsza wypłata jest dostępna po około 50 sekundach aktywnej gry. Duży dron odbiera gotowe ładunki niezależnie od wypłat; jego przylot, przejęcie kontenera i odlot trwają 10 sekund. Wydobycie i kolejne wypłaty działają także podczas lotu drona. Ładunki mogą czekać w kolejce do odbioru.

Po wypłatę podejdź do **lewego komputera** na platformie. Popatrz na niego z bliska i naciśnij **E**. Zielona lampka miga, jeśli są dostępne nagrody, a mały ekran pokazuje ich liczbę. Wybierz kartę **1/2/3** lub kliknięciem. Karta jest natychmiast instalowana, a terminal się zamyka. **E/Esc** pozwala wyjść bez wybierania; propozycje pozostają takie same. Wypłaty mogą czekać w kolejce. Otwarcie terminala zatrzymuje walkę, wydobycie i lot transportowca. **Prawy komputer służy osobno do jednorazowych usług.**

Jest **71 rodzajów kart** i pięć rzadkości: **Common, Uncommon, Rare, Epic, Legendary**. Kolor i nazwa rzadkości są widoczne na karcie, a opis podaje faktyczną wielkość bonusu. Zamknięcie komputera nie zmienia propozycji ani ich rzadkości. Pierwsza wypłata zawsze zawiera przynajmniej jedną propozycję nowej wieżyczki bojowej.

Na ekspedycję wybierasz maksymalnie **4 różne typy bojowe i 2 różne typy wsparcia**. Liczba egzemplarzy wybranego typu może rosnąć aż do wspólnego limitu 210 stanowisk. Po wybraniu czterech typów bojowych pozostałe typy bojowe znikają z losowania; wsparcie ma osobny limit dwóch typów.

| Rodzaj karty | Działanie |
| --- | --- |
| Nowa wieżyczka | Laser, minigun, Tesla, snajper, lanca ciągła, toksyny, rakiety, moździerz lub palnik; jedna sztuka do magazynu |
| Przekaźnik tempa | +20% szybkostrzelności do 6 bojowych wieżyczek na bezpośrednio sąsiednich heksach |
| Przekaźnik zasięgu | +20% zasięgu do 6 bojowych wieżyczek na bezpośrednio sąsiednich heksach |
| Generator śnieżycy | Stałe spowolnienie o 50% w promieniu 40 m, z animacją śnieżycy co 3 s |
| Rozwój działa manualnego | Premia do obrażeń lub szybkostrzelności, zależna od rzadkości |
| Rozwój obrony automatycznej | Premia do obrażeń, tempa lub zasięgu wszystkich obecnych i przyszłych wieżyczek |
| Wydobycie i pancerz | Szybsze wydobycie albo większe maksymalne HP z naprawą |
| Drobne bonusy Common | +1 obrażenie manualne lub +1% szczęścia zwiększającego szansę lepszej rzadkości |
| Radar (domyślny) | Mały panel HUD i telewizor pod dachem od początku. Zasięg 350 m; kierunek patrzenia zawsze u góry. Karta radaru została usunięta. |
| Impulsy kriogeniczne | Spowolnienie narasta wraz ze zbliżaniem się bestii do rdzenia, w promieniu 85 m |
| Wspólny cel | Manualne trafienie oznacza cel na 3 s; wieżyczki w zasięgu przełączają na niego ostrzał. Cel otrzymuje +0/10/20/30/40% obrażeń zależnie od rzadkości |
| Awaryjne oczyszczenie | Raz na ekspedycję, przy 50% HP: zabija zwykłe bestie i zadaje bossom 50% ich maksymalnego HP |
| Fabryka jeździków | Uruchamia produkcję 5 maszyn co 12 s; jeździk szuka najbliższego celu i wybucha w promieniu 4 m, za 180 obrażeń w centrum (mniej bossom) |
| Punkt usług (tylko Rare) | +1 punkt do wydania przy prawym komputerze na jedną z trzech jednorazowych usług |
| Większa seria / szybsza linia | +1/2/3/4/5 jeździków na serię albo +10/20/30/40/50% bazowego tempa produkcji, zgodnie z rzadkością karty |

Powtarzane bonusy parametrów sumują się względem wartości bazowej. Bonus obrażeń wieżyczek obejmuje również truciznę, podpalenie i eksplozje. Premia tempa skraca także ładowanie lancy. Moduły rdzenia są jednorazowymi odblokowaniami w obrębie ekspedycji; po zdobyciu znikają z puli. [Szczegóły etapu 2](ETAP_02.md).

Przekaźniki wzmacniają aktualne parametry bojowego sąsiada o 20%, także ładowanie lancy. Ten sam rodzaj wsparcia nie kumuluje się, ale tempo i zasięg działają razem. Śnieżyce nie kumulują się ze sobą; chłodzenie rdzenia dodatkowo mnoży pozostałą prędkość. Na przykład 50% spowolnienia wieży i 30% rdzenia pozostawia 35% prędkości. [Zasady wsparcia i telewizora](WSPARCIE.md).

### Heksy i zrzuty wieżyczek

**Jednorazowe usługi:** Brimstone po 3 s zadaje sześć impulsów po 10 000 obrażeń (co 0,5 s) w promieniu 10 m; fala po 3 s ogłusza wszystkich obecnych przeciwników na 5 s; przeciążenie po 3 s poświęca wybraną wieżyczkę i zadaje 50 000 obrażeń w promieniu 40 m. Każda kosztuje 1 punkt. Wybór celu zatrzymuje czas, a PPM/Esc/Tab anuluje go bez straty punktu. Zniszczony heks można ponownie zabudować; wybrana technologia pozostaje w limicie 4+2. [Szczegółowe zasady](USLUGI.md).

Wokół rdzenia jest **210 stanowisk na regularnej siatce heksagonalnej**, tworzącej duży sześciokąt z wolnym środkiem na platformę. Odstęp między stanowiskami wynosi **21 metrów**. Po wybraniu karty dowolnej wieżyczki automatycznie przechodzisz do widoku taktycznego. **LPM na wolnym heksie** zamawia dostawę. Podgląd pokazuje właściwy model i jego aktualny zasięg, wraz z bonusami. Miejsce zostaje zarezerwowane, a wieżyczka opada na linach pod balonem. Po około **6 sekundach** ląduje i zaczyna strzelać. Balon odczepia liny, uruchamia napęd i szybko wraca w górę.

**Tab** przełącza pokład i widok z góry, **WASD** przesuwa kamerę, **Q/E** obraca, **kółko** przybliża. **Tab / PPM / Esc** wraca na pokład. Niewykorzystane wieżyczki pozostają w magazynie. **Podczas wybierania miejsca czas stoi**: walka, wydobycie, drony i balony czekają. Czas rusza po zamówieniu ostatniej wieżyczki z magazynu albo po powrocie na pokład. Samo obserwowanie z góry z pustym magazynem nie pauzuje gry. **P** pozwala dodatkowo włączyć zwykłą pauzę. Nie można zamówić drugiej wieżyczki na zajęty albo już zarezerwowany heks.

Każda zdobyta karta wieżyczki rezerwuje jedną jednostkę globalnego limitu 210 — także wtedy, gdy czeka w magazynie. Po wykorzystaniu limitu nowe karty wieżyczek przestają się losować; pozostałe ulepszenia nadal są dostępne. Dotyczy to też aktualizacji starszych wypłat czekających w kolejce.

Platforma załadunkowa uranu i zawis drona zostały obniżone poniżej pokładu operatora. Stanowiska wieżyczek omijają platformę załadunkową.

Przeciwnicy uszkadzają rdzeń po dojściu do niego. Przy **0 HP** ekspedycja się kończy; ekran pokazuje czas obrony, eliminacje, transporty i liczbę ulepszeń. Naciśnij **R** lub przycisk nowej ekspedycji, aby wrócić do wyboru działa.

Obsługa działa ma lekką bezwładność kamery: krótki, płynny dojazd do kierunku wskazanego myszą, bez kołysania. Wygładzanie działa niezależnie od liczby klatek; celownik, lufa i strzały korzystają z tego samego kierunku. Rozglądanie podczas chodzenia pozostaje bezpośrednie.

## Dziewięć modułów manualnych

Zwykły minigun ma stopniowo czerwieniejące końcówki luf. Jego przemiana **Legendary: Extreme Cooling System** dodaje kriogeniczne dysze z mgiełką przy gorących lufach, **bazową szybkostrzelność x5 (60/s)** i **rozrzut x2**. Wcześniejsze premie nadal działają. Testowo dodaj ją przez ; i wyszukaj „cooling”. [Opis chłodzenia](EXTREME_COOLING.md).

Laser i minigun są podstawowymi działami startowymi; pozostałe odblokowania dojdą później. Laser otrzymał jednorazową przemianę **Legendary: Laserowy moździerz** (eksperymentalna technologia plazmowa). Głowica ma 17 widocznych emiterów i strzela łukiem salwami o łącznej sile 20 zwykłych impulsów lasera, rozłożonymi w promieniu 10 m. Bazowo daje 3000 nominalnych obrażeń na salwę przy niezmienionej szybkostrzelności.

**; otwiera laboratorium wszystkich 71 ulepszeń**: wyszukiwanie, wybór rzadkości i dodawanie do bieżącego podejścia bez zużywania nagród. Enter dodaje kartę, Tab zmienia rzadkość, strzałki/kółko przewijają, ; lub Esc zamyka panel. [Zasady plazmy i laboratorium](PLAZMA_LAB.md).

**' (apostrof) otwiera bestiariusz** z grafikami, statystykami i przywoływaniem po 1, 10 lub 100 przeciwników. Czas stoi podczas wyboru. Nowe bestie: elita sabotażysta niszczący wieżyczkę po 3 s, oblężniczy plujec strzelający z 63 m i boss Nadumysł wyłączający wieżyczki impulsami na obszarze dwóch heksów. [Mechanika, testy i grafiki](BESTIARIUSZ.md).

| Broń | Parametry | Zachowanie |
| --- | --- | --- |
| Laser impulsowy | 150 obrażeń, 1 strzał/s, zasięg 500 m | Precyzyjna pojedyncza wiązka. Trafienie zabija podstawowego pełzacza jednym strzałem. |
| Minigun korbowy | 26 obrażeń/pocisk, 12 pocisków/s, zasięg do 420 m | Pociski lecą z prędkością 210 m/s, mają rozrzut i niewielki opad. Obracają się lufy i boczna korbka; widać odrzut i wyrzucane łuski. |
| Tesla / łańcuch | 340 obrażeń na cel, co 3,2 s, 440 m | Trafienie bezpośrednie: do 10 celów. Trafienie ziemi: do 9 celów od punktu uderzenia. Skoki 56 m, bez powtarzania celów. |
| Toksyczna katapulta | Co 2,8 s, 360 m | Lot łukiem; plama o promieniu 14 m utrzymuje się 8 s i zadaje 65 DPS, spowalniając o 50%. |
| Moździerz boczny | 1100 obrażeń, co 3,5 s, 460 m | Lufa po prawej; pocisk leci łukiem i po dolocie wybucha w promieniu 15 m. Podgląd trajektorii i czasu dolotu. |
| Rakiety / Smart | 180 obrażeń, co 0,32 s, 500 m | Namierzają przez 0,5 s cel najbliżej środka okienka, potem lecą po zakrzywionej trasie i wybuchają w promieniu 9 m. Bez celu nie strzelają. |
| Lanca próżniowa | 1650 DPS + 200 przy pierwszym kontakcie, 550 m | Przytrzymaj LPM: ładowanie 1,8 s, wiązka przez 3 s, chłodzenie 2 s. Puszczenie przerywa ładowanie lub ostrzał. |
| Snajper / Railgun | 750 bazowych obrażeń × (1 + dystans w m / 100), co 2,4 s, 750 m | Przebija do ośmiu przeciwników; każdy otrzymuje własny bonus dystansu. Ekran na broni pokazuje rzeczywisty widok z celownika w zbliżeniu około 8,3×. |
| Miotacz / Piekło | 240 DPS; PPM: 63 m szeroko / 94,5 m wąsko (5 m) | Stożek o kącie około 57°, zasięg trzech odstępów heksów; podpalenie 30 DPS przez 3 s. Podświetlenie ziemi pokazuje obszar działania, jego krawędzie i odstępy co 21 m. |

Wszystkie dziewięć modułów jest obecnie dostępnych od początku. Laser trafia pierwszego przeciwnika na linii celowania; minigun może chybić. Wyjątkiem od ręcznego celowania są rakiety Smart oraz kolejne skoki Tesli. Kolorowy błysk celownika sygnalizuje trafienie. Amunicja jest nieograniczona. Modele, animacje i celowniki modułów są różne; fotel i winda pozostają wspólne. [Pełny opis i testy](DZIALA_MANUALNE.md).

Broń jest oddzielnym modułem od fotela i windy. Parametry z tabeli są wartościami początkowymi; karty zdobyte w danym podejściu modyfikują obrażenia i szybkostrzelność.

## Plansza i napór przeciwników

- Platforma rdzenia na wysokości 18 m: centralny fotel, teleskopowa winda, konsole i kolumny. Okrągły dach ma kształt niskiego ściętego stożka z rozsuwanym okrągłym włazem. Opada ku brzegom, odsłaniając więcej terenu podczas celowania z podniesionego stanowiska. Barierki biegną po zewnętrznej krawędzi pokładu; można podejść do nich bliżej.
- Można chodzić i rozglądać się po pokładzie. Kolizje chronią przed wejściem w stanowisko, kolumny i konsole oraz spadnięciem z krawędzi.
- Po uruchomieniu wydobycia nie ma natychmiastowej grupy przy bazie. Przeciwnicy zaczynają nadchodzić pojedynczo z obrzeży, w odległości 280–320 m od rdzenia; pierwszy pojawia się po około 0,5 s, następni początkowo co około 2,8 s. Po 60 s dochodzą szybcy łowcy, po 120 s tarany, po 180 s plujące bestie, po 240 s latające widma. Każdy gatunek ma oddzielny sprite i inne parametry. Elity pojawiają się od 90 s, są większe, mają złote oznaczenie i czterokrotne HP.
- Co 3 minuty nadchodzi boss: naprzemiennie Tytan Bazaltu oraz Królowa Roju. Alarm pojawia się 10 s wcześniej. Tytan emituje uderzenia blisko rdzenia, Królowa przywołuje łowców. Pokonanie bossa daje 15 s zmniejszonego naporu.
- Z bossa zawsze wypada kryształ; z elity jest 1% szansy. W widoku z góry kliknij kryształ, aby wysłać drona. **C** wysyła drona po najbliższy wolny kryształ. Można kolejkować kolejne zlecenia. Nagroda trafia do komputera dopiero po powrocie; kryształ bossa daje propozycje ulepszeń minimum Rare, z większą szansą Epic i Legendary; karta budowy zachowuje stałą rzadkość swojego typu. Uran nadal daje wypłatę natychmiast po zapełnieniu paska.
- Standardowy napór ma limit 6000 aktywnych przeciwników; wspólna symulacja obsługuje do 20 000, uwzględniając przywołania. Limit jeździków wynosi 2048. Obrona i produkcja pochodzą z kart, bez sklepu.
- Podstawowe bestie mają stale 100 HP, ale ich paski zdrowia pozostają ukryte. Brak zapisu stanu i rozwoju między podejściami.
- Podłoże ma subtelną teksturę bazaltowego pyłu z mipmapami i filtrowaniem anizotropowym. Broń manualna zostawia krótkie osmalenia; lanca rysuje wygasającą rozżarzoną smugę. [Efekty podłoża](EFEKTY_PODLOZA.md).
- Broń manualna ma delikatne przechyły kamery, drgania i dodatkowe mechaniczne dźwięki. Lanca pulsuje podczas wiązki, zakłóca dźwięk na rezerwie uranowej i lekko znosi lufę, co można korygować myszą. [Szczegóły efektów](MANUALNE_ZMIANY.md).

Jeździki używają przezroczystego sprite’a widzianego od tyłu. Wybierają najbliższego przeciwnika przy wyjeździe, omijają cylinder rdzenia, a po utracie celu namierzają nowego. Eksplozja zadaje obrażenia także bardzo bliskim przeciwnikom. Karty produkcji pojawiają się dopiero po zdobyciu fabryki. [Aktualny balans wieżyczek i fabryki](BALANS_WIEZYCZEK.md).

## Sterowanie

| Klawisz | Działanie |
| --- | --- |
| 1–9, Enter | Wybór broni i start w menu |
| 1 / 2 / 3 w terminalu | Wybór jednej karty |
| WASD / Shift | Chodzenie / szybszy ruch po platformie |
| Mysz | Rozglądanie i ręczne celowanie |
| E | Wejście za działo / opuszczenie stanowiska / obsługa pobliskiego komputera |
| Tab | Widok z góry z heksami / powrót na pokład |
| LPM w widoku z góry | Zamówienie zrzutu na wolny heks |
| LPM na krysztale / C | Wysłanie drona po wskazany / najbliższy wolny kryształ |
| WASD, Q/E, kółko w widoku z góry | Przesuwanie, obrót i przybliżenie kamery |
| LPM | Strzelanie; po uwolnieniu kursora pierwsze kliknięcie przejmuje mysz |
| P | Pauza / wznowienie |
| R | Nowa ekspedycja i powrót do wyboru broni |
| F1 | Pomoc |
| M / V | Dźwięk / synchronizacja pionowa |
| F11 | Pełny ekran / okno |
| Esc | Zamknięcie pomocy / uwolnienie myszy / pauza; w menu wyjście |
| Alt+F4 / Ctrl+Q | Wyjście |

Utrata aktywności okna pauzuje grę i zwalnia spust. P wznawia. Podczas podnoszenia i opuszczania stanowiska nie można strzelać. E podczas podnoszenia pozwala zawrócić windę. Z widoku taktycznego można wrócić do tego samego stanowiska manualnego.

## Pozostałe tryby

- **POLIGON_ROZWOJU.bat** — szybka próba: 4 typy bojowe do rozmieszczenia przez Tab i 5 gotowych wypłat przy komputerze. Dwie pierwsze pozwalają wybrać wsparcie, następne moduły rdzenia i premie. Wydobycie wynosi 8 uranu/s, rdzeń ma 20 000 HP. To osobny wariant testowy; zwykły START.bat zachowuje standardowy początek.
- **POLIGON_EKSPEDYCJI.bat** — szybki dostęp do nowego etapu: start od 2:30, boss za 30 s aktywnego czasu, cztery działa w magazynie i sześć gotowych wyborów, w tym fabryka jeździków, nanoboty oraz rozwój produkcji. Rdzeń ma 20 000 HP. Normalny START.bat zaczyna od zera.
- **ARSENAL.bat** — wcześniejszy poligon dziewięciu automatycznych wieżyczek, budowanie z góry, gatunki bestii i bossowie. [Instrukcja arsenału](ARSENAL.md).
- **TEST_NATYWNY.bat / BENCHMARK.bat** — oddzielne laboratorium dużej liczby jednostek i pocisków. [Metodologia](native/README.md), [historyczne wyniki](WYNIKI_WYDAJNOSCI.md).
- **WEB_PROTOTYP.bat** — starszy prototyp przeglądarkowy. Polecenia npm dotyczą tylko tej wersji.

## Kompilacja i testy

Gotowy plik `native/build/rdzen-game.exe` korzysta z sąsiedniego `SDL3.dll` i katalogu `public/assets/`. Zachowaj strukturę projektu. Do uruchomienia nie potrzeba internetu ani Node.js. Wymagany Windows 10/11 x64 i Direct3D 11.

**PRZEBUDUJ.bat** kompiluje wersję Release i uruchamia testy mechanik wszystkich natywnych trybów. Lokalny kompilator i SDL są w `.toolchain/`; odtworzenie brakujących narzędzi wymaga internetu, ale nie zmienia systemowego PATH.

**TEST_DZIALA.bat** uruchamia dwie automatyczne próby: wybór broni, podejście do fotela, E, podniesienie ponad dach, strzelanie, pauzę, powrót windą i zmianę wielkości okna. Podczas testów nie steruj grą. Raporty JSON i zrzuty trafiają do `artifacts/command/laser/` oraz `artifacts/command/minigun/`. **TEST_ARSENALU.bat** nadal sprawdza wcześniejszy poligon.

Nowe pliki: `native/command_game.hpp` (reguły i pociski), `command_scene.hpp` (modele i interfejs), `command_app.hpp` (obsługa aplikacji), `command_tests.cpp` (testy). Używają istniejącej symulacji, renderera z instancjonowaniem oraz lokalnych grafik; [pochodzenie tekstur](ASSETS.md).

**TEST_EKSPEDYCJI.bat** sprawdza cały nowy cykl: fizyczne podejście do terminala, zatrzymanie symulacji, ukończenie transportu, wybór karty myszą, jej zastosowanie, porażkę i restart. Wyłącznie w automatycznej próbie magazyn zaczyna niemal pełny, a końcowe HP jest obniżane, aby skrócić oczekiwanie; zwykła gra zaczyna z pustym magazynem i pełnym HP. `expedition-tests.exe` dodatkowo sprawdza pierwszą minutę zwykłej symulacji bez takich skrótów.

Pierwszy etap: `native/expedition.hpp` — wydobycie, transporty i kolejka kart; `native/expedition_scene.hpp` — model drona i kontenera; `native/expedition_tests.cpp` — testy ekonomii i ulepszeń.

**TEST_DOSTAWY.bat** sprawdza kartę wieżyczki, wejście w widok z góry, kliknięcie heksu, pauzę opadania, lądowanie, odczepienie i przyspieszony powrót balonu oraz powrót na pokład. Raport: `artifacts/command/laser/delivery-result.json`. Geometria siatki i reguły dostaw: `native/turret_slots.hpp`; modele balonów: `native/sentry_drop_scene.hpp`.

**TEST_OBRAZU.bat** sprawdza renderowanie podczas ciągłego strzelania i obrotów kamery, również przy przełączaniu pełnego ekranu. Raporty: `artifacts/render/`. [Opis poprawki migotania i zakres sprawdzeń](POPRAWKA_OBRAZU.md).

**TEST_ROZWOJU.bat** sprawdza nowy terminal z rzadkościami, wybór Tesli, pauzę rozmieszczania, dostawę właściwego typu i ostrzał oraz radar. Raporty i zrzuty: `artifacts/command/upgrades/`. `progression-tests.exe`, uruchamiany przy przebudowie, sprawdza losowanie 71 kart, szczęście, 210 mieszanych dostaw, rzeczywiste bonusy wszystkich broni i moduły rdzenia.

**TEST_WSPARCIA.bat** sprawdza wybór i dostawę przekaźnika, 6 połączeń, puls śnieżycy oraz podwieszony telewizor. Raporty i zrzuty: `artifacts/command/supports/`. `support-tests.exe` sprawdza limity 4+2, sąsiedztwo heksów, rzeczywiste tempo i zasięg, łączenie spowolnień, usuwanie instalacji oraz uruchomienie wsparcia dopiero po lądowaniu.

**TEST_EKSPEDYCJI_02.bat** sprawdza przez klawiaturę i mysz odblokowanie fabryki, ulepszenie serii, strzał do bossa, kliknięcie kryształu, pauzę lotu drona i odbiór nagrody bossa. Raporty: `artifacts/command/frontier/`. `frontier-tests.exe` dodatkowo sprawdza małe wybuchy obszarowe, bezpieczne namierzanie, omijanie rdzenia, 2048 jeździków, losowanie kryształów i harmonogram przeciwników.

**TEST_BALANSU.bat** uruchamia szybką symulację około 15 minut ekspedycji z prawdziwymi nagrodami, falami i walką. Automat celuje dokładnie i natychmiast obsługuje komputer, więc jest to kontrola tempa, a nie gwarancja poziomu trudności dla człowieka. Raport JSON i CSV: `artifacts/frontier/`.

Rdzeń regeneruje **3 HP/s**. Powtarzalne **Nanoboty regenerujące** zwiększają tę wartość zależnie od rzadkości. Dwa różne komputery z ekranami skierowanymi do środka oraz animowana pompa uranu z miernikami tworzą centrum dowodzenia. [Zasady regeneracji i pompy](RDZEN_I_POMPA.md).

Legendarny moduł lancy **REZERWA URANOWA** przedłuża ostrzał po 3 sekundach kosztem 5 uranu/s; wiązka i dodatkowy przewód stają się zielone. [Aktualne zmiany dział manualnych](MANUALNE_ZMIANY.md).

Radar na platformie jest na godzinie 2. Nalewak katapulty składa się do góry przed rzutem, a przewód rezerwy lancy jest widoczny po lewej stronie z fotela. Zatrucie manualne dodaje 20 HP/s bestiom i 100 HP/s bossom; automatyczna toksyna ma osobne, kumulowane dawki. Bossowie mają paski HP. Lampy bazy są zielone powyżej 50%, żółte przy 20–50%, czerwone przy 20% lub mniej; poniżej 20% kamera operatora lekko drży.

Wszystkie dziewięć dział ma teraz własny legendarny moduł. Tesla odblokowuje PPM ze wzmacnianiem wieżyczek, katapulta klej unieruchamiający, moździerz dwa odbicia, Smart Instant namierzanie, snajper +2% obrażeń za metr i drugi ekran CRT, miotacz dwa jednoczesne strumienie. [Pełne zasady i nazwy kart](LEGENDY_MANUALNE.md).

Paczka ZIP zawiera również Uruchom-Linux.sh i LINUX.txt: uruchamianie buildu Windows przez Wine albo Steam Proton. To nie jest natywny port Linux; działanie na Linux nie zostało tutaj zweryfikowane.

Czterech nowych bossów: Żmij segmentowy, Śluzowiec (łącznie 24000 HP), Pożeracz oraz Plujec alfa. Wszystkich przywołasz z bestiariusza pod apostrofem. Mechaniki i statystyki: [BOSSY.md](BOSSY.md).

## Kawa i senność

Po 5 minutach aktywnej ekspedycji operator staje się senny: wolniej chodzi i obraca działo, obraz rozmywa się, a mruganie zasłania ekran. Ekspres stoi po lewej stronie za fotelem. Podejdź i naciśnij **E**: nalewanie i picie trwa 3 s, w tym 0,5 s picia. Walka trwa dalej.

Pięć kaw daje 5 / 6 / 7 / 8 / 10 minut czujności. Karty Common / Rare / Epic / Legendary odblokowują odpowiednio tier II / III / IV / V. Ulepszenia ekspresu Rare i Legendary skracają cykl do 2 s i 1 s oraz rozbudowują model. Powtórki i niższe poziomy nie wypadają. [Zasady, grafiki i testy](KAWA.md).

Propozycja kolejnych etapów: [karty, balans pierwszych ekspedycji, osiągnięcia i planety](PLAN_ROZWOJU.md).

Globalne obrażenia, tempo i zasięg mają osobne limity po 5 wyborów. Każdy typ bojowej wieżyczki ma dodatkowo trzy ścieżki rozwoju kolejno Common → Uncommon → Rare → Epic → Legendary. Karty budowy mają stałe rzadkości, a oferta ma minimum 40% szansy na bojową wieżyczkę (60% dla uranu przed 10 min). [Aktualne zasady, wartości premii i losowanie](ULEPSZENIA_WIEZYCZEK.md).
