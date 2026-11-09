# Stanowiska dowodzenia i jednorazowe usługi

Normalny start: wybór broni, Enter, przygotowanie na platformie, czerwony przycisk za działem [E]. Dopiero wtedy ruszają wydobycie, zegar naporu i pierwsze bestie. Po naciśnięciu grzybek opada, dwie połówki zapadni rozsuwają się, a kolumna zjeżdża pod pokład. Zapadnia pozostaje otwarta i wyjeżdża z niej radio URANIUM FM 2137. Pełna sekwencja trwa około 3,45 s i respektuje pauzę. Przycisk nie pojawia się ponownie w tym podejściu; R resetuje ekspedycję i przywraca go.

Lewy komputer, patrząc od miejsca startu w kierunku działa, służy do odbierania ulepszeń. Ma zieloną lampkę migającą przy dostępnej nagrodzie i trzycyfrowy ekran licznika (0–999). Stary żółty poziomy pasek dostępności usunięto. Prawy komputer obsługuje jednorazowe usługi i ma osobny niebieski licznik punktów.

## Punkty

Nowa powtarzalna karta „Punkt usług” występuje wyłącznie jako niebieskie Rare. Wybranie jej przy lewym komputerze zużywa normalną nagrodę i daje dokładnie jeden punkt usług. Nie jest to osobna, darmowa wypłata. Punkty nie znikają po zamknięciu komputera; reset ekspedycji usuwa je wraz z pozostałym rozwojem.

Prawy komputer otwiera się przez E z bliska. Każda usługa kosztuje 1 punkt. Wybór myszą albo 1/2/3. E/Esc zamyka panel. Usługi są dostępne po uruchomieniu wydobycia.

| Usługa | Cel | Efekt |
| --- | --- | --- |
| Brimstone | Dowolne poprawnie wskazane miejsce na powierzchni | Ostrzeżenie 3 s, potem orbitalna wiązka przez 3 s. 6 impulsów co 0,5 s, każdy po 10 000 obrażeń w promieniu 10 m: do 60 000 na nieruchomy cel |
| Fala ogłuszająca | Bez wskazywania miejsca | Po 3 s ogłusza wszystkich obecnych, żywych przeciwników na 5 s, także bossów. Zatrzymuje marsz, ataki i zdolności, w tym przywoływanie. Trucizna i podpalenie nadal działają |
| Przeciążenie | Własna, wylądowana wieżyczka bojowa lub wsparcie | Po 3 s niszczy ją i zadaje 50 000 obrażeń każdemu przeciwnikowi w promieniu 40 m. Okrąg zasięgu widać podczas wybierania celu |

Brimstone i przeciążenie otwierają widok taktyczny. Czas stoi podczas wybierania celu. Punkt zostaje wydany dopiero po poprawnym kliknięciu; pusty heks przy przeciążeniu, niepoprawny punkt i już przeciążana instalacja nie zużywają punktu. PPM/Esc/Tab anuluje celowanie i wraca do panelu usług. Fala jest zamawiana od razu po wybraniu i wtedy pobiera punkt.

Po potwierdzeniu wraca zwykły upływ czasu. Otwarcie terminala, pauza lub trwający wybór miejsca na inną wieżyczkę zatrzymują także odliczanie usług. Brimstone działa w stałym punkcie; przeciwnik może opuścić jego promień. Fala obejmuje jednostki istniejące w chwili wybuchu, nie przyszłe spawny. Kolejne ogłuszenie odnawia czas zamiast dodawać go bez ograniczeń. Wspólny cel zwiększa obrażenia usług tak samo jak innych źródeł.

## Poświęcenie wieżyczki

Przeciążenie usuwa aktywny model i zwalnia heks oraz jednostkę limitu 210. Usunięcie przekaźnika lub generatora natychmiast przelicza jego wsparcie dla sąsiadów i obszary spowolnienia. Można ponownie zdobyć i dostarczyć wybrany wcześniej typ. Zniszczenie ostatniego egzemplarza nie zmienia jednak technologii wybranych na to podejście: limit 4 typów bojowych i 2 wsparcia pozostaje zachowany.

Odliczanie wiąże się z trwałym identyfikatorem instalacji, a nie indeksem tablicy. Jednoczesne poświęcenie kilku wież nie zmienia celów po usunięciu wcześniejszych. Gdy cel usługi przestanie istnieć przed detonacją, punkt jest zwracany; nowa wieżyczka na tym samym miejscu nie zostaje przypadkiem zniszczona.

## Próby

`POLIGON_USLUG.bat` daje 6 punktów usług, 3 gotowe wybory niebieskiej karty i 20 000 HP rdzenia. Przycisk nadal trzeba nacisnąć; potem można od razu testować prawy komputer. Normalne `START.bat` i `Rdzen.exe` nie dają początkowych punktów.

`services-tests.exe` sprawdza przygotowanie przed startem, jednokrotne uruchomienie, role konsol, dokładnie Rare, wydawanie i zachowywanie punktów, sześć impulsów po 10 000 obrażeń Brimstone, 10 m zasięgu, pauzę, paraliż bossów i ataków oraz promień 40 m przeciążenia. Sprawdza też zwalnianie miejsca, pamięć wybranego typu, utratę aur, jednoczesne detonacje i zwrot za unieważniony cel.

`TEST_USLUG.bat` wykonuje interakcje przez klawiaturę i mysz: naciska przycisk, czeka na jego schowanie, odbiera punkt z lewego komputera, otwiera prawy i zamawia wszystkie usługi. Kontrolowane nieruchome cele i dwa dodatkowe punkty są wyłącznie danymi testu. Zrzuty i wynik JSON: `artifacts/command/services/`.

Dotychczasowe testy ekspedycji zaczynają kontrolowaną symulację już po starcie wydobycia; nowy test oddzielnie weryfikuje rzeczywiste naciśnięcie czerwonego przycisku. Po zmianie gry samodzielny EXE odtwarza `ZROB_EXE.bat`.

Obie ofensywne usługi mają dedykowany basowy dźwięk syntetyzowany lokalnie. Brimstone pulsuje szeroką kolumną z białym rdzeniem, rozbłyskiem i bocznymi wyładowaniami. Przeciążenie tworzy rozszerzającą się falę, gorącą kulę, iskry i dym. Dźwięki respektują wyciszenie M i pauzę, a liczba głosów jest ograniczona.
