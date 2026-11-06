# Wieżyczki wsparcia i radar pokładowy

Ekspedycja ma dwa niezależne limity technologii: 4 typy bojowe oraz 2 typy wsparcia. Do limitu typu liczy się już odebrana karta, również gdy wieżyczka czeka w magazynie albo leci balonem. Można zdobywać kolejne egzemplarze wybranych typów. Wszystkie instalacje wspólnie zajmują 210 stanowisk. Nieaktualne propozycje w kolejce są zastępowane dostępnymi premiami przy otwieraniu komputera.

Wcześniej limit czterech typów nie był zaimplementowany; teraz jest egzekwowany osobno dla walki i wsparcia.

| Typ wsparcia | Działanie | Dostępność |
| --- | --- | --- |
| Przekaźnik tempa | Mnoży szybkostrzelność do 6 sąsiednich bojowych wieżyczek przez 1,2. Skraca też ładowanie lancy | Od Uncommon |
| Przekaźnik zasięgu | Mnoży zasięg do 6 sąsiednich bojowych wieżyczek przez 1,2 | Od Uncommon |
| Generator śnieżycy | Mnoży prędkość bestii przez 0,5 w promieniu 40 m. Śnieg i pierścień pulsują co 3 s | Od Rare |

Sąsiad to wieżyczka na jednym z sześciu heksów bezpośrednio stykających się z heksem przekaźnika, w odległości 21 m. Instalacje wsparcia nie wzmacniają siebie ani innych rodzajów wsparcia. Kilka przekaźników tego samego rodzaju nie powiela bonusu, ale premia tempa i zasięgu działają jednocześnie. Te wartości są stałe dla wszystkich rzadkości ich kart.

Premia sąsiedztwa mnoży aktualny parametr, uwzględniający globalne ulepszenia. Przykład: laser o bazowym zasięgu 65 m, z globalną premią +30% i przekaźnikiem, ma zasięg 65 × 1,3 × 1,2 = 101,4 m. Dodanie albo usunięcie przekaźnika aktualizuje sąsiadów; nowe wieżyczki otrzymują premię od razu po lądowaniu. Wsparcie w locie jeszcze nie działa.

Dwie śnieżyce spowalniają przeciwnika tylko raz. Śnieżyca i chłodzenie rdzenia są odrębnymi źródłami i mnożą pozostałą prędkość. Jeśli rdzeń spowalnia w danym miejscu o 30%, a wieża o 50%, bestia porusza się z prędkością 0,7 × 0,5 = 35% bazowej. Aura działa stale pomiędzy wizualnymi pulsami. Globalne premie zasięgu i tempa bojowego nie zmieniają promienia 40 m ani okresu 3 s generatora.

Przekaźniki mają różne ruchome anteny, świecące połączenia i impuls przesuwający się w stronę wspieranych dział. Generator ma obracany wentylator, kryształ, opadające płatki i rozszerzający się pierścień. Podgląd rozmieszczenia pokazuje połączenia z obecnymi bojowymi sąsiadami.

## Telewizor

Karta radaru odblokowuje jednocześnie panel interfejsu i fizyczny monitor podwieszony pod dachem, po przeciwnej stronie fotela od miejsca startu operatora. Monitor pokazuje radar 350 m: czerwone bestie, zieloną obronę, kierunek działa oraz wskaźniki HP rdzenia, uranu i liczby kontaktów. Obraz aktualizuje się na żywo i zatrzymuje razem z symulacją. Ekran znajduje się nad głową i poniżej dachu; nie wystaje ponad stanowisko strzeleckie.

## Wydajność i sprawdzenie

Sąsiedztwo i połączenia są zapamiętywane i przebudowywane przy dodaniu lub usunięciu instalacji. Statyczna siatka obszarów śnieżycy ogranicza liczbę sprawdzeń przy poruszaniu bestii; nie wykonujemy pełnej pętli po wszystkich wieżach dla każdego przeciwnika. Geometria połączeń, płatki i ekran korzystają z istniejącego renderowania instancjonowanego.

`support-tests.exe` sprawdza limity typów, powtarzanie wybranych technologii, usuwanie niedostępnych kart, dokładnie 6 sąsiadów, rzeczywistą zmianę liczby strzałów i skutecznego zasięgu, przyspieszenie ładowania lancy, łączenie i wygaszanie aur oraz puls co 3 s. Sprawdza też, że przekaźnik działa dopiero po wylądowaniu.

`TEST_WSPARCIA.bat` przechodzi przez terminal, wybiera przekaźnik, zamawia go na heksie i sprawdza lądowanie oraz połączenia. Następnie pokazuje śnieżycę i telewizor. Wyłącznie na potrzeby testu odblokowuje radar, ustawia kontrolowaną ofertę i dodaje sześciu bojowych sąsiadów oraz generator śnieżycy. Raporty i obrazy zapisuje w `artifacts/command/supports/`.

Do ręcznych prób służy `POLIGON_ROZWOJU.bat`: 4 bojowe typy w magazynie, 2 wybory wsparcia i 3 wybory modułów/premii. Normalna gra startuje przez `START.bat`.
