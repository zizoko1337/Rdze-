# Ekspedycja 2137 — etap 2: rozwój obrony

Opis zakresu etapu 2. Aktualizacja dodająca trzy wsparcia, limity 4+2 i telewizor: [wsparcie](WSPARCIE.md).

Etap dodaje 22 karty, pięć rzadkości, wszystkie dziewięć wieżyczek w ekspedycji i cztery moduły rdzenia. Standardowa ekonomia, natychmiastowe wypłaty za uran, 210 heksów, pauza podczas rozmieszczania i loty balonów pozostają wspólną pętlą.

## Rzadkości

Bez szczęścia bazowy rozkład rzadkości pojedynczej propozycji wynosi: Common 40%, Uncommon 32%, Rare 18%, Epic 8%, Legendary 2%. Najpierw losowana jest rzadkość, następnie pasująca do niej karta. Trzy propozycje zawsze mają różne rodzaje. Szczęście zwiększa prawdopodobieństwo lepszej rzadkości; +1% nie oznacza dodatkowego punktu procentowego szansy Legendary.

| Bonus | Common | Uncommon | Rare | Epic | Legendary |
| --- | --- | --- | --- | --- | --- |
| Obrażenia manualne | +12,5% | +25% | +37,5% | +50% | +75% |
| Tempo manualne | +10% | +20% | +30% | +40% | +60% |
| Wydobycie | +12,5% | +25% | +37,5% | +50% | +75% |
| Maksymalne HP i naprawa | +125 | +250 | +375 | +500 | +750 |
| Obrażenia wieżyczek | +10% | +20% | +30% | +40% | +60% |
| Tempo / zasięg wieżyczek | +7,5% | +15% | +22,5% | +30% | +45% |

Wszystkie procenty dodają się do wartości bazowej. +1 obrażenie manualne dodawane jest po mnożniku. Drobne bonusy +1 obrażenie i +1% szczęścia występują wyłącznie jako Common.

Nowa wieżyczka ma standardowe parametry danego typu; jej rzadkość jest progiem dostępności technologii, a nie ukrytym mnożnikiem obrażeń. Laser jest dostępny od Common; minigun, toksyny i palnik od Uncommon; Tesla, snajper, rakiety i moździerz od Rare; lanca od Epic. Pierwsza wypłata zawiera przynajmniej jedną kartę nowej wieżyczki.

## Dostawy i bonusy

Magazyn przechowuje typ każdej zdobytej wieżyczki. Zamawiane są kolejno; panel pokazuje następny typ. Podgląd, model pod balonem i docelowa instalacja odpowiadają wybranej karcie. Podgląd zasięgu obejmuje premie. Wszystkie typy wspólnie zużywają limit 210 miejsc, także w magazynie i podczas dostawy. Pełny limit usuwa wszystkie karty nowych wieżyczek z puli. Nieaktualne karty z kolejki są zastępowane dostępnymi bonusami bez zmiany pozostałych propozycji.

Premie obrony działają na obecne i przyszłe instalacje. Obrażenia wzmacniają bezpośrednie trafienia, eksplozje, chmury toksyn, zatrucie i podpalenie. Pociski i chmury już utworzone zachowują moc z chwili wystrzelenia. Szybkostrzelność skraca odstępy między strzałami, odnowienie Tesli i lancy oraz ładowanie lancy. Czas ciągłej wiązki pozostaje równy 3 s.

## Moduły rdzenia

- Radar (od Uncommon): panel w prawym górnym rogu, 350 m, zielona obrona, czerwone bestie i kierunek działa. Znaczniki są rysowane jednym wywołaniem z instancjonowaniem. Paski HP przeciwników nadal są ukryte.
- Impulsy kriogeniczne (od Rare): spowolnienie w promieniu 85 m rośnie płynnie w kierunku rdzenia, do 40% redukcji prędkości. Pierścienie sygnalizują pracę instalacji.
- Wspólny cel (od Common): trafienie laserem manualnym lub pociskiem miniguna oznacza żywy cel na 3 s. Wieżyczki przejmują go, jeśli mieści się w ich własnym zasięgu. Oznaczony cel otrzymuje dodatkowo 0/10/20/30/40% obrażeń dla Common/Uncommon/Rare/Epic/Legendary. Mnożnik obejmuje już strzał oznaczający i wszystkie źródła obrażeń, również efekty czasowe i oczyszczenie. Po wygaśnięciu lub zmianie celu premia znika; kolejne trafienia odnawiają oznaczenie. Oznaczenie nie przechodzi na nowego przeciwnika korzystającego z tego samego miejsca w pamięci.
- Awaryjne oczyszczenie (od Epic): jednorazowo po spadku do 50% HP usuwa zwykłych przeciwników i zadaje bossom obrażenia równe połowie maksymalnego HP. Nie naprawia rdzenia; nie odradza się po leczeniu. Zdobycie modułu poniżej progu uruchamia go przy wznowieniu czasu.

Modułów nie można zdobyć ponownie w tym samym podejściu. Restart usuwa je wraz z pozostałym rozwojem.

## Szybka próba

`START.bat` uruchamia zwykłą ekspedycję. `POLIGON_ROZWOJU.bat` uruchamia osobny wariant z 9 wieżyczkami do rozmieszczenia, trzema gotowymi wyborami kart, szybszym wydobyciem i większym HP. Tab otwiera heksy; E przy bocznej konsoli pozwala wybierać karty. Wariant nie ma osobnych zasad walki ani dostaw.

## Weryfikacja

`progression-tests.exe` sprawdza 10 000 ofert, dostępność każdego rodzaju karty, unikalność propozycji i poprawę losowań przez szczęście. Dostarcza 210 mieszanych instalacji i sprawdza typy oraz wspólny limit. Porównuje rzeczywiste obrażenia wszystkich dziewięciu broni przed i po ulepszeniu, w tym efekty czasowe. Osobno sprawdza tempo, zasięg przyszłych instalacji, ładowanie lancy, faktyczne manualne oznaczenie celu, zasięg namierzania, spowolnienie oraz jednorazowe oczyszczenie z bossem.

`TEST_ROZWOJU.bat` używa zdarzeń klawiatury i myszy do przejścia przez terminal i zrzut Tesli. Test ustawia kontrolowane karty Rare/Epic/Legendary i odblokowany radar, następnie sprawdza dostawę właściwego typu i ostrzał. Te ułatwienia występują wyłącznie w teście. Zrzuty i raport JSON zapisują się w `artifacts/command/upgrades/`.

Kolejny etap pętli to różnorodne fale ekspedycji, elity i bossowie, kryształy oraz wysyłanie drona po nagrody. Rozwój między podejściami i trwałe odblokowania pozostają na później.
