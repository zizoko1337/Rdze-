# Bazowe parametry wieżyczek

Wartości z `native/gameplay.hpp`. DPS to teoretyczna średnia na jednego przeciwnika stale w zasięgu, bez ulepszeń i oznaczenia Wspólnego celu. Pomija początkowe rozkręcanie miniguna, lot pierwszego pocisku i drobne różnice wynikające z kroku symulacji. Lanca uwzględnia pełny cykl ładowania, wiązki i chłodzenia.

| Wieżyczka | Zasięg | Działanie i obliczenie | Bazowy DPS na cel |
| --- | --- | --- | --- |
| Laser | 65 m | Gwarantowany impuls: 30 obrażeń co 0,42 s | 71,43 |
| Minigun | 54 m | Rozkręcane lufy, gwarantowane trafienia: 8 obrażeń co 0,065 s | 123,08 |
| Tesla | 55 m | 145 obrażeń wszystkim w zasięgu co 5,5 s | 26,36 |
| Snajper | 180 m | Gwarantowany strzał: 380 obrażeń co 3,5 s | 108,57 |
| Lanca | 110 m | Ładowanie 6 s, wiązka 185 DPS przez 3 s, chłodzenie 2 s; 555 / 11 | 50,45 średnio; 185 podczas wiązki |
| Toksyna | 78 m | 18 obrażeń co 1,6 s w promieniu 9 m, plus 22 DPS zatrucia; 18 / 1,6 + 22 | 33,25 przy regularnych trafieniach; samo zatrucie 22 |
| Rakiety | 125 m | Naprowadzana rakieta co 1,35 s, do 140 obrażeń, promień eksplozji 10 m | 103,70 w centrum eksplozji |
| Moździerz | 170 m | Pocisk po łuku co 3,7 s, do 250 obrażeń, promień eksplozji 16 m | 67,57 w centrum eksplozji |
| Palnik | 38 m | Stożek ognia: 7,2 obrażenia co 0,1 s plus 12 DPS podpalenia | 84 na każdego w stożku |

Rakiety i moździerz tracą liniowo do 45% obrażeń na obrzeżu eksplozji. Dlatego ich DPS na pojedynczego trafionego przeciwnika wynosi odpowiednio około 57,04–103,70 i 37,16–67,57, zależnie od odległości od punktu wybuchu.

Tesla trafia wszystkich w zasięgu, palnik wszystkich w stożku, a toksyny i eksplozje wszystkich w swoim obszarze. Ich łączny DPS rośnie wraz z liczbą trafionych jednostek. Na przykład Tesla przy 10 celach daje około 263,64 łącznego średniego DPS.

Chmura toksyny istnieje przez 7 s i odnawia zatrucie trwające 6 s. Podpalenie trwa 3 s od ostatniego trafienia. Trucizna i podpalenie odświeżają czas i zachowują najmocniejszą bieżącą truciznę; nie dodają kolejnych niezależnych stosów DPS. Bezpośrednie trafienia wielu wieżyczek nadal sumują obrażenia.

Wieżyczki domyślnie wybierają najbliższego przeciwnika i trzymają go do śmierci albo wyjścia z zasięgu. Wspólny cel może wymusić zmianę na oznaczonego przeciwnika w zasięgu wieżyczki. Minigun automatyczny trafia gwarantowanie; rozrzut dotyczy manualnego miniguna operatora.

Są też trzy rodzaje wsparcia, które nie zadają obrażeń: przekaźnik tempa (+20% szybkostrzelności sześciu bojowych sąsiadów), przekaźnik zasięgu (+20% zasięgu sześciu sąsiadów) oraz generator śnieżycy (spowolnienie 50%, promień 40 m). W jednej ekspedycji można wybrać 4 typy bojowe i 2 typy wsparcia. [Szczegółowe zasady](WSPARCIE.md).
