# Legendarne moduły pozostałych dział

Wszystkie sześć kart jest unikalne, ma rzadkość **Legendary** i losuje się tylko przy wybranym odpowiednim dziale manualnym. Laboratorium pod `;` pozwala dodać je od razu, również pilnując właściwego typu broni. Łącznie katalog ma 38 kart.

| Działo / karta | Efekt | Zmiana wyglądu |
|---|---|---|
| Tesla — Tryb wzmacniania | PPM przełącza atak/wsparcie. Łańcuch do 10 własnych wieżyczek, skoki 56 m. +100% szybkostrzelności przez 10 s; kolejne trafienie odnawia czas, nie sumuje mnożnika. Trafienie ziemi zużywa pierwszy odcinek, tak jak w trybie bojowym. Premia mnoży się z przekaźnikiem tempa. | Fizyczny przełącznik z opisem; błyskawice na wzmocnionych wieżyczkach przez cały czas efektu. |
| Katapulta — Dodatek kleju plazmowego | Zastępuje spowolnienie unieruchomieniem w plamie. Dotyczy również bossów. Zatrucie i jego obrażenia zostają. Po opuszczeniu plamy blokada szybko wygasa; nie blokuje ataków ani zdolności. | Ciemniejsza toksyna w łyżce, pocisku i plamie. |
| Moździerz — Naboje z kauczuku antygrawitacyjnego | Każdy pocisk wybucha trzy razy z pełnymi obrażeniami: trafienie, odbicie 12 m do przodu, kolejne 8 m. Dodatkowe łuki są niższe. Wszystkie etapy korzystają z jednego miejsca w puli pocisków. | Fiolka z żółtą cieczą przy lufie. Napełnia się podczas przeładowania i wtłacza płyn przy strzale. |
| Rakiety Smart — Instant namierzanie | Domyślnie wymagają utrzymania tego samego kandydata w oknie przez 0,5 s. Zmiana kandydata lub jego wyjście z okna resetuje czas. Po uzyskaniu blokady można strzelać co 0,32 s do tego samego celu. Moduł usuwa czas blokady. | Dodatkowy świecący moduł optyczny, bez napisu. |
| Snajper — Podwójna premia dystansu | Zwiększa premię z +1% do +2% obrażeń za metr. Bazowy zasięg pozostaje 750 m. Wzór: bazowe obrażenia po ulepszeniach × (1 + dystans × 0,02). | Drugi, mniejszy ekran CRT po lewej, podniesiony i zamocowany bez odrzutu: RANGE i potencjalne DMG. Pokazuje dystans do środka trafianego przeciwnika, a przy braku celu dystans na linii celowania. Nie renderuje dodatkowej kamery. |
| Miotacz — Podwójny palnik | Szeroki strumień 63 m i skupiony 94,5 m / szerokość 5 m pracują równocześnie. Każdy zadaje 240 bazowych DPS; na przecięciu stożków łącznie 480 DPS. Podpalenie tego samego źródła pozostaje 30 DPS. Usuwa przełączanie PPM. | Druga, dłuższa lufa po lewej, brak przełącznika; oba podświetlenia zasięgu i podwójny celownik. |

Efekt Tesli i czas namierzania zatrzymują się podczas pauzy i otwartych menu. Wzmocnienie nie omija wyłączenia wieżyczki przez bossa. Odbicia nie tworzą dodatkowych obiektów sceny, a błyskawice i napisy korzystają z istniejących partii instancjonowania.

`native/manual_legends_tests.cpp` sprawdza wszystkie sześć kart, ograniczenia do broni, rzadkość i unikalność, łańcuch wsparcia, czas efektu, klej na bossie, trzy pełne eksplozje, blokadę Smart oraz zmianę celu i pauzę, faktyczne obrażenia snajpera i dwa jednoczesne stożki ognia. `Rdzen.exe --legends-test` wykonuje graficzną próbę wszystkich dziewięciu dział z ich modułami i przełącza Teslę przez rzeczywiste zdarzenie PPM.
