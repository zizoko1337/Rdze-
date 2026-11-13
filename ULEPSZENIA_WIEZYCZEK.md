# Karty i rozwój wieżyczek

## Limity globalne

Każdy z trzech parametrów wszystkich bojowych wieżyczek ma osobny limit **5 wyborów na ekspedycję**: obrażenia, szybkostrzelność i zasięg. Rzadkość zmienia wielkość premii, ale każda karta zużywa jeden wybór. Można więc zdobyć 5 kart obrażeń, 5 szybkostrzelności i 5 zasięgu w dowolnych rzadkościach.

Po piątym wyborze karta znika z puli. Starsza oferta czekająca w kolejce również jest sprawdzana przed wyświetleniem i wyborem. Na karcie globalnej widać wykorzystany limit.

## Rozwój konkretnego typu

Dziewięć typów bojowych ma po trzy osobne ścieżki: obrażenia, szybkostrzelność i zasięg. To **27 nowych rodzajów kart**, łącznie 71 rodzajów w katalogu.

Każda ścieżka ma dokładnie pięć kroków:

**Common → Uncommon → Rare → Epic → Legendary**

Przykład: po zdobyciu pierwszej toksycznej wieżyczki mogą wypaść Common obrażeń, Common szybkostrzelności i Common zasięgu toksyny. Zdobycie Common zasięgu dopuszcza wyłącznie Uncommon zasięgu. Pozostałe dwa parametry nadal zaczynają się od Common. Druga wieżyczka toksyczna nie tworzy nowego licznika — rozwój dotyczy całego typu.

Nie można przeskoczyć rzadkości, ponownie zdobyć ukończonego poziomu ani wziąć szóstego ulepszenia. Laboratorium również dodaje kolejny legalny krok, niezależnie od ustawionej w nim rzadkości. Karty typu wymagają technologii zdobytej w danej ekspedycji, także gdy dostawa jeszcze trwa lub egzemplarz jest w magazynie. Zniszczenie wieżyczki nie kasuje zdobytej technologii i jej ulepszeń.

| Parametr | Common | Uncommon | Rare | Epic | Legendary |
| --- | ---: | ---: | ---: | ---: | ---: |
| Obrażenia globalne | +10% | +20% | +30% | +40% | +60% |
| Tempo lub zasięg globalny | +7,5% | +15% | +22,5% | +30% | +45% |
| Obrażenia konkretnego typu | +15% | +30% | +45% | +60% | +90% |
| Tempo lub zasięg konkretnego typu | +10% | +20% | +30% | +40% | +60% |

Wszystkie te premie sumują się względem bazowego parametru, także między rozwojem globalnym i rozwojem typu. Przykład: globalne Common obrażeń (+10%) i Common obrażeń toksyny (+15%) dają toksynie łącznie +25%, a pozostałym wieżyczkom +10%. Wsparcie heksowe nadal wzmacnia wynik zgodnie ze swoimi zasadami.

Premie obejmują obecne i przyszłe egzemplarze, podgląd zasięgu dostawy, pociski, eksplozje, truciznę i podpalenie. Tempo lancy skraca również ładowanie. Nie dotyczą dział manualnych ani przekaźników i generatora śnieżycy.

## Stałe rzadkości budowy

| Rzadkość | Typy bojowe |
| --- | --- |
| Common | Laser, minigun, toksyna, palnik |
| Uncommon | Snajper, rakiety, moździerz |
| Rare | Tesla, lanca |

Przekaźnik tempa i przekaźnik zasięgu mają stałą rzadkość Uncommon, generator śnieżycy Rare. Budowa tej samej zwykłej wieżyczki nie zajmuje już wyboru Epic lub Legendary. Każda karta budowy daje jedną nową sztukę do rozmieszczenia. Pozostają limity 4 typów bojowych, 2 wsparcia i 210 łącznych stanowisk wraz z magazynem i dostawami.

Unikalne przemiany manualne pozostają Legendary i są przypisane do konkretnej wybranej broni. Manualny minigun nie otrzymuje np. kleju plazmowego katapulty. Wspólne premie obrażeń i tempa nadal działają dla każdej broni manualnej. Jeśli gracz ma toksyczną wieżyczkę i manualny minigun, może rozwijać toksyczną wieżyczkę — jest to niezależny odbiorca efektu.

## Szansa na obronę w ofercie

Szansa oznacza **co najmniej jedną propozycję nowej sztuki bojowej w całej ofercie trzech kart**. Nie jest to szansa na każdy slot ani gwarancja odblokowania nowego typu. Kolejne egzemplarze posiadanych typów również się liczą; wsparcie nie zastępuje bojowej propozycji.

- Pierwsza wypłata z uranu gwarantuje propozycję bojową.
- Kolejne wypłaty z uranu przed 10 minutą aktywnej ekspedycji mają co najmniej **60%** szansy.
- Późniejsze wypłaty oraz nagrody z kryształów mają co najmniej **40%** szansy.
- Po dwóch kolejnych ofertach bez wieżyczki następna gwarantuje propozycję bojową.
- Pozostałe sloty też mogą wylosować wieżyczkę, więc faktyczna częstość może być wyższa.
- Liczy się zaproponowanie karty, nie jej wybranie. Ponowne otwarcie terminala nie zmienia oferty ani licznika pecha.
- Gdy 210 stanowisk jest już wykorzystanych, nie proponujemy dodatkowej budowy. Po zwolnieniu miejsca propozycje wracają.

Kategoria bojowa jest losowana przed rzadkością karty i niezależnie od rozmiaru pozostałego katalogu. Dodawanie kolejnych kaw, ekonomii lub specjalizacji nie obniża jej szansy. Przy czterech wybranych typach bojowych gwarancja wybiera kolejną sztukę jednego z nich.

**Nagroda bossa:** karta budowy zachowuje rzadkość swojego typu, więc może być Common lub Uncommon także z kryształu bossa. Pozostałe propozycje z takiego kryształu nadal mają minimum Rare i zwiększoną szansę Epic/Legendary. Pozwala to zachować 40% na obronę również przy zestawie złożonym wyłącznie z czterech typów Common.

## Weryfikacja

`native/reward_tests.cpp` sprawdza oddzielne limity, wszystkie 27 ścieżek, niedozwolone skoki rzadkości, dopasowanie do broni, efekty w rzeczywistej walce, sumowanie premii, podgląd dostawy, 120 000 ofert z różnych źródeł, serie pecha, pełny magazyn i powtarzanie sztuk przy limicie typów. Test uruchamia się podczas standardowej przebudowy.

Zmiana nie wdraża jeszcze osiągnięć odblokowujących bronie manualne ani kolejnych planet. Plan tych etapów pozostaje w `PLAN_ROZWOJU.md`.
