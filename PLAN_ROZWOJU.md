# Plan rozwoju RDZENIA

Status: plan wdrażany etapami. Wykonano usunięcie dłoni i przedramienia oraz zasady kart opisane w `ULEPSZENIA_WIEZYCZEK.md`: limity globalne, rozwój typów, dopasowanie do działa, rzadkości budowy i szanse na obronę. Dalszy balans fal, osiągnięcia i planety pozostają propozycją. Konkretne progi poniżej są wartościami startowymi do prób z graczami; aktualne reguły kart opisuje wskazany dokument.

## 1. Docelowa pętla gry

Wybierz planetę i odblokowane działo → rozpocznij wydobycie → buduj obronę i rozwijaj wybraną specjalizację → wykonuj cele ekspedycji → zginij albo zakończ misję → zobacz postęp osiągnięć i nowe odblokowania → spróbuj ponownie.

Pierwsze podejście na pierwszej planecie powinno zwykle trwać około 8–12 minut. To cel balansu, nie zaprogramowana śmierć w dziesiątej minucie. Dobry gracz musi móc przeżyć dłużej już za pierwszym razem. Kolejne próby wydłuża lepsze rozmieszczanie obrony, priorytety celów, obsługa terminali i poznawanie przeciwników. Odblokowania dodają nowe możliwości budowania zestawów.

Na początku wystarczą nowe bronie i planety jako trwały rozwój. Nie dokładamy równocześnie stałych premii obrażeń/HP: utrudniłyby ocenę, czy gracz wygrywa dzięki nauce, czy liczbie rozegranych prób. Nowe działo powinno otwierać inny styl walki; szczegółowy balans ich siły pozostaje do osobnej rozmowy.

## 2. Karty dopasowane do wyposażenia

Obecny kod już ogranicza legendarne przemiany do właściwego działa. Trzeba rozszerzyć tę zasadę na każdy przyszły efekt i uporządkować dane kart.

Każda karta powinna mieć: kategorię, dozwolony cel, wymagania, dostępne rzadkości, limit/powtarzalność i opis rzeczywistego efektu. Kategorie: działo manualne, wieżyczki bojowe, wsparcie, gospodarka/rdzeń, kawa/ekspres. Cel: konkretne działo, rodzina broni, wszystkie bojowe wieżyczki albo rdzeń.

Zasady kwalifikacji:

- Ulepszenie manualnej trucizny wymaga wybranej katapulty toksycznej. Nie pojawia się dla manualnego miniguna.
- Ulepszenie konkretnej wieżyczki wymaga jej technologii zdobytej w bieżącym podejściu, także gdy egzemplarz czeka w magazynie lub dostawie.
- Manualny minigun z toksyczną wieżyczką może dostać kartę wyraźnie opisaną jako ulepszenie tej wieżyczki. Nie może dostać toksycznej przemiany swojej broni manualnej.
- Karty wspólne, np. obrażenia działa manualnego, nadal działają dla każdej wybranej broni. W opisie pokazujemy nazwę odbiorcy efektu.
- Karta odblokowująca nowy typ wieżyczki jest dostępna przed posiadaniem tego typu. Nie należy blokować jej jak karty ulepszającej już posiadaną technologię.
- Po dojściu do limitu 4 typów bojowych i 2 wsparcia nadal wypadają kolejne egzemplarze wybranych typów. Po wykorzystaniu 210 stanowisk wypadają pozostałe ulepszenia.
- Te same wymagania obowiązują przy losowaniu, wyświetlaniu starej oferty i zatwierdzaniu wyboru. Otwarcie terminala nie służy do ponownego losowania.

Najpierw filtrujemy karty według wyposażenia, dopiero potem losujemy. Losowanie błędnej karty i późniejsza zamiana na przypadkową gospodarkę mogłyby odebrać graczowi należną propozycję wieżyczki.

## 3. Dostępność wieżyczek

Przyjmuję „40% na nową wieżyczkę” jako co najmniej 40% szansy, że **cała oferta trzech kart zawiera przynajmniej jedną nową sztukę bojową do zbudowania**. Kolejny egzemplarz posiadanego typu też spełnia ten warunek. Wsparcie nie zastępuje wieżyczki bojowej w tej gwarancji. To rozwiązuje problem zbyt małej liczby dział, również po zapełnieniu limitu typów.

Propozycja pierwszej wersji:

| Warunek | Reguła |
| --- | --- |
| Pierwsza wypłata | Gwarantowana propozycja wieżyczki bojowej |
| Kolejne wypłaty z uranu, do 10 min | 60% szansy na ofertę z wieżyczką bojową |
| Później | Co najmniej 40% |
| Dwie kolejne oferty bez wieżyczki | Następna gwarantuje propozycję bojową |
| Kryształy elit i bossów | Co najmniej 40%, z zachowaniem jakości nagrody |
| Limit wszystkich stanowisk osiągnięty | Propozycja bojowa znika, pozostają użyteczne karty |

Licznik pecha mierzy brak **propozycji**, nie brak wyboru. Gracz może świadomie wybrać inną kartę. Podgląd tej samej oferty nie zmienia licznika. Przy trzech kartach i 40% na każdy slot szansa na przynajmniej jedną wieżyczkę wyniosłaby około 78,4% przed uwzględnieniem zależności; dlatego nie należy mylić tych dwóch definicji.

Losowanie: ustal kategorię gwarantowanego slotu → zbierz pasujące typy → wylosuj zgodną rzadkość i kartę → dobierz dwie różne, użyteczne propozycje. Nie zmieniamy rzadkości Rare na Common, aby wcisnąć do oferty laser. W nagrodzie bossa minimum Rare musi zostać zachowane. Szansa kategorii bojowej działa niezależnie od liczby nowych kart kawy, ekonomii i przyszłych systemów.

Przy bazowym wydobyciu 2/s nagroda przychodzi co 50 s: około 12 wypłat przez 10 min. Przy 40% mówimy o około 5 ofertach z wieżyczką, zanim uwzględnimy pierwszą gwarancję i inne nagrody. To może być za mało na obecną dziesiątą minutę. Dlatego proponuję 60% na początku, zamiast uznać samo 40% za rozwiązany balans. Wstępny cel dla gracza wybierającego głównie obronę: około 6–9 bojowych wieżyczek przed 10 min. Zweryfikujemy go w grze.

## 4. Rzadkość musi odpowiadać znaczeniu efektu

| Rzadkość | Rola |
| --- | --- |
| Common | Mała, użyteczna poprawa podstawowego parametru; tanie technologie początkowe; kawa II |
| Uncommon | Rozwój podstawowej obrony i proste specjalizacje |
| Rare | Wyraźne narzędzie do budowania zestawu; efekty wymagające odpowiedniej broni; kawa III; ekspres 2 s |
| Epic | Silna synergia lub zmiana zachowania; kawa IV |
| Legendary | Unikalna przemiana broni lub urządzenia; kawa V; ekspres 1 s |

Stałe przemiany mają stałą rzadkość. Powtarzalne premie mogą mieć różne poziomy jakości, ale ich efekt musi rzeczywiście rosnąć. Np. ten sam egzemplarz podstawowej wieżyczki bez dodatkowej korzyści nie powinien kosztować wyboru Legendary. Dla pierwszej iteracji nowe typy wieżyczek dostają sensowne, ograniczone rzadkości; nie tworzymy przy okazji całego systemu jakości egzemplarzy.

Przed wdrożeniem robimy tabelę wszystkich obecnych 44 kart: do czego pasują, kiedy mogą wypaść i co daje każda ich rzadkość. Sprawdzamy także kolejki starszych ofert, limity technologii, jednorazowe efekty i brak downgrade’ów kawy.

## 5. Trwałe osiągnięcia i działa manualne

Od początku dostępne są tylko **laser i minigun**. Pozostałe siedem widać w menu jako zablokowane, z czytelnym warunkiem i licznikiem postępu. Zdobyte odblokowanie jest ogłaszane podczas ekspedycji, a nową broń wybieramy na następną próbę.

Proponowane warunki do pierwszych testów:

| Działo manualne | Osiągnięcie sumowane między podejściami |
| --- | --- |
| Tesla | 1000 eliminacji przypisanych automatycznym wieżyczkom Tesli |
| Katapulta toksyczna | 500 eliminacji przypisanych toksycznym wieżyczkom |
| Moździerz | 500 eliminacji przypisanych automatycznym moździerzom |
| Rakiety Smart | 750 eliminacji przypisanych wyrzutniom rakiet |
| Snajper | 300 eliminacji przypisanych snajperom |
| Miotacz | 750 eliminacji przypisanych wieżyczkom ogniowym |
| Lanca | 500 eliminacji przypisanych automatycznym lancom |

To progi startowe, nie ostateczny balans. Kalibrujemy je według czasu do odblokowania: pierwsza dodatkowa broń powinna być osiągalna po około 2–3 udanych próbach specjalizacji. Wolne bronie pojedynczego celu wymagają mniej eliminacji niż broń obszarowa. Licznik 1000 Tesli pozostaje zgodny z założeniem użytkownika.

Automatyczne odpowiedniki tych broni muszą być dostępne przed odblokowaniem manualnych. Inaczej powstałoby wymaganie niemożliwe do wykonania. Ekran osiągnięć powinien podpowiadać właściwą wieżyczkę i pokazywać postęp także po porażce.

Zasady techniczne: rzeczywisty sprawca śmiertelnych obrażeń, zachowany także dla trucizny i podpalenia; jedna eliminacja naliczana raz; bez zaliczania laboratorium, bestiariusza i testów. Postęp zapisujemy atomowo podczas gry, po osiągnięciu i po zakończeniu próby. Profil leży poza wersjonowanym katalogiem paczki EXE, np. `%LOCALAPPDATA%/Rdzen/profile.json`, aby aktualizacja nie kasowała osiągnięć. Brak lub uszkodzenie pliku nie może blokować startu gry; format ma wersję i obsługę migracji.

## 6. Pierwsza planeta i cel około 10 minut

Największy obecny skok jest w `frontier.hpp`: od 600 s dochodzi seria początkowo 120 wrogów co 15 s, równolegle do zwykłego naporu. To około 8 dodatkowych wrogów/s. Większa szansa wieżyczek nie powinna maskować tak gwałtownego progu.

Proponowany przebieg:

- **0–2 min:** czas na pierwszą wypłatę, pierwszą dostawę i poznanie platformy. Wczesna obrona musi być osiągalna laserem i minigunem.
- **2–5 min:** pierwsze ważne decyzje o specjalizacji i boss. Gracz uczy się, które kierunki wymagają obrony.
- **5–8 min:** pierwsza kawa; szybsze i wytrzymalsze jednostki wymagają współdziałania działa manualnego z wieżyczkami.
- **8–10 min:** zapowiedziany większy napór, ale nadal możliwy do odparcia podstawowym wyposażeniem.
- **10–12 min:** stopniowo rosnące roje i cel pierwszego kontraktu. Nie dokładamy nagle pełnego roju co 15 s bez przerwy.
- **Po kontrakcie:** wybór zakończenia ekspedycji albo dalszej obrony dla rekordu. Dalsze ryzyko nie odbiera już zdobytych osiągnięć ani odblokowania następnej planety.

Oddzielamy gatunki, liczbę, tempo i wytrzymałość przeciwników. Nie zwiększamy wszystkich naraz. Alarm wskazuje nadchodzący większy rój i kierunek, a po dużym zagrożeniu następuje chwila oddechu. Kolejne planety stopniowo wprowadzają bardziej złożonych przeciwników i bossów; pierwsza nie musi pokazywać wszystkich mechanik naraz.

## 7. Planety jako osobne wyzwania

Każda planeta ma własną pulę przeciwników, bossów, harmonogram naporu i cel kontraktu. Wspólne pozostają zasady broni, karty i profil. Nie wprowadzamy ukrytych odporności odbierających sens wybranemu działu.

| Planeta / roboczy motyw | Przeciwnicy i decyzje | Proponowane odblokowanie |
| --- | --- | --- |
| I: Bazaltowy przyczółek | Czytelni pełzacze, łowcy i tarany; nauka obrony oraz obsługi platformy | Dostępna od początku |
| II: Zarodnikowe mokradła | Nowe bestie rozdzielające się i plujące, obszary zarodników z wyraźnym ostrzeżeniem | Kontrakt na planecie I: 12 min i pokonanie 2 bossów w jednej próbie |
| III: Księżyc burzowy | Nowe jednostki latające i skokowe, nacisk na zasięg i pokrycie kierunków | Kontrakt na planecie II |
| IV: Martwa fabryka | Maszyny oblężnicze, eskorty sabotażystów i cele wymagające szybkiego priorytetu | Kontrakt na planecie III |

Najpierw projektujemy i wdrażamy tylko planetę II, z 2–3 rzeczywiście nowymi gatunkami i własnym bossem. Różnice muszą dotyczyć zachowania, a nie samego koloru sprite’a. Reszta tabeli wyznacza kierunek i nie zobowiązuje do tworzenia czterech lokacji naraz. Można wracać na wcześniejsze planety, ćwiczyć i kończyć osiągnięcia broni.

## 8. Kolejność wdrożenia i pomiary

1. **Karty i dostępność obrony:** tabela kart, filtr wymagań, kategorie losowania, minimum 40%, większa szansa na początku i ochrona przed długą serią bez wieżyczki. Zachować 4+2 i 210 stanowisk.
2. **Profil i odblokowania:** zapis, przypisywanie eliminacji, ekran osiągnięć, menu z dwoma działami początkowymi. Ukończenie osiągnięcia nie zmienia broni w trakcie walki.
3. **Balans pierwszej planety:** próby laserem i minigunem, poprawa skoku po 10 min, telemetria. Szczegółowy balans pozostałych dział omawiamy osobno.
4. **Kontrakt i zakończenie ekspedycji:** czytelny cel, wynik, dalsza obrona i odblokowanie kolejnej planety.
5. **Druga planeta:** nowi przeciwnicy, jeden boss, własny napór i oprawa, testy z tym samym wyposażeniem referencyjnym.

Mierzymy: czas i przyczynę porażki, liczbę wieżyczek w 3/5/8/10 min, otrzymane i wybrane karty, straty rdzenia, tempo eliminacji według źródła, czas poza działem, liczbę wypitych kaw i czas do osiągnięć. Osobno analizujemy pechowe losowania oraz decyzje gracza. Oczekiwanie na powrót do terminala lub odbiór kryształu nie może wyglądać jak problem samego DPS.

Symulacja z wieloma ziarnami sprawdza rozkład ofert i wykrywa niemożliwe do wygrania początki; automat doskonale celujący nie określa trudności dla człowieka. Cel 8–12 min weryfikujemy z osobami pierwszy raz uruchamiającymi grę, a dłuższe przeżycie porównujemy przy tym samym wyposażeniu. Testy regresji obejmują niedozwolone karty, rzadkości, gwarancje wieżyczek, brak miejsca, stare oferty, zapis/migracje i brak naliczania testowych eliminacji.
