# RDZEŃ — wybór technologii i pomiar wydajności

Data pomiaru: 2 listopada 2026, Europe/Warsaw.

**Wybrany fundament dla Windows: C++20 + SDL3 + Direct3D 11.** Przygotowany renderer i symulacja obsłużyły test 10 000 bestii, 100 000 aktywnych pocisków i 512 dział ze znacznym zapasem względem budżetu 16,67 ms na klatkę przy 60 FPS. To potwierdzenie wydajności konkretnego fundamentu, nie gwarancja wydajności dowolnej przyszłej zawartości ani porównanie wszystkich dostępnych API graficznych.

## Sprzęt i warunki

- AMD Ryzen 5 3600, 6 rdzeni / 12 wątków.
- NVIDIA GeForce GTX 1660 SUPER, około 6 GB VRAM (5966 MiB zgłoszone przez DXGI).
- Około 16 GB RAM, Windows, ekran 1920 × 1080 przy 60 Hz.
- Renderowanie sprzętowe Direct3D 11, rozdzielczość 1920 × 1080, VSync wyłączony.
- Kompilacja Release C++20, Clang 23.1.4, `-O3 -DNDEBUG`, SDL 3.4.18.
- Każda scena: 3 sekundy rozgrzewki, następnie 12 sekund pomiaru.
- Symulacja: stałe 60 kroków/s. Zmierzony czas symulowany odpowiadał czasowi rzeczywistemu; nie uzyskano wysokiego FPS przez spowolnienie świata.
- Symulacja jednowątkowa. Wszystkie obiekty przesyłane do GPU; bez odrzucania niewidocznych jednostek po stronie CPU.

## Zmierzone wyniki

| Scena | Bestie | Aktywne pociski | Działa | Średni FPS | p99 klatki | Średni krok symulacji | Średni czas GPU |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Widok taktyczny | 1000 | 10 000 | 128 | 1449 | 3,73 ms | 0,21 ms | 0,39 ms |
| Widok taktyczny | 5000 | 50 000 | 256 | 736 | 4,35 ms | 0,97 ms | 0,67 ms |
| Widok taktyczny | 10 000 | 100 000 | 512 | 414 | 4,54 ms | 1,90 ms | 0,96 ms |
| Pierwsza osoba | 1000 | 10 000 | 128 | 1406 | 3,89 ms | 0,22 ms | 0,38 ms |
| Pierwsza osoba | 5000 | 50 000 | 256 | 735 | 3,91 ms | 0,96 ms | 0,66 ms |
| Pierwsza osoba | 10 000 | 100 000 | 512 | 408 | 4,76 ms | 1,94 ms | 0,94 ms |
| Gęsty tłum przy rdzeniu, pierwsza osoba | 5000 | 50 000 | 256 | 608 | 4,57 ms | 1,30 ms | 0,83 ms |

**p99 klatki** oznacza, że około 99% zmierzonych klatek było szybszych od podanego czasu. To nie jest gwarantowane minimum FPS. Najdłuższa pojedyncza klatka w całym zestawie miała 7,97 ms. Proces zajmował około 78–90 MiB working set; ta wartość nie zawiera zajętości VRAM. W każdym scenariuszu renderer wykonywał 11 wywołań rysowania na klatkę, włącznie z diagnostycznym interfejsem. Dostępne próbki GPU obejmują wszystkie zmierzone klatki tego przebiegu.

Na monitorze 60 Hz wysokie liczby FPS oznaczają zapas obliczeniowy. Interaktywny podgląd domyślnie włącza VSync, aby nie obciążać bez potrzeby GPU. Klawisz V pozwala go przełączyć.

## Co działo się podczas pomiaru

Bestie przemieszczały się do środka, miały HP i były eliminowane. Działa wyszukiwały najbliższe cele w siatce przestrzennej i utrzymywały poprawne namierzenie. Każdy pocisk wykonywał ruch naprowadzany oraz test trafienia odcinkiem, uwzględniający szybki ruch między krokami. Populacje bestii i pocisków były uzupełniane, żeby obciążenie nie znikało wraz z eliminacjami.

Przykładowo w scenie 10 000 / 100 000 z pierwszej osoby w ciągu 12 sekund zarejestrowano **1 295 398 trafień i 3872 eliminacje**. W gęstym tłumie było **3 995 962 trafienia i 15 404 eliminacje**. Liczba wystrzałów służy generowaniu obciążenia; to nie jest jeszcze balans szybkostrzelności finalnej gry.

Wszystkie zadane obiekty uczestniczyły w symulacji i były przekazywane do renderera. W kamerze FPS część świata naturalnie znajduje się poza kadrem lub za innymi obiektami. Ostatni scenariusz dodatkowo gromadził przeciwników w odległości 12–42 m od środka, aby sprawdzić duże pokrycie ekranu.

## Co dały optymalizacje

- Dane w stałych pulach zamiast tysięcy alokowanych osobno obiektów. Ponowne użycie miejsca w puli zabezpieczone numerem generacji.
- Siatka przestrzenna i utrzymywanie celu. Osobny test porównał 10 000 zapytań z pełnym przeszukiwaniem — wyniki były zgodne. Pomiar 20 000 identycznych zapytań dał około **174 ms dla pełnego przeszukiwania i 23,5 ms dla siatki**, około **7,4× szybciej**. Ten mikrobenchmark nie zawiera kosztu przebudowy siatki; pomiar pełnego kroku symulacji go zawiera.
- Instancing: całe populacje sprite'ów i pocisków rysowane zbiorczo. Bufor instancji strumieniowany bez odrzucania całej pamięci przy każdej partii.
- Przesyłanie wektora kierunku pocisku zamiast obliczania `atan2` na CPU i odtwarzania go przez `sin/cos` na GPU. Średnie przygotowanie danych sceny 5000 / 50 000 spadło z około **1,42 ms do 0,58 ms na klatkę**. Pomiar przed zmianą trwał 8 s, a końcowy 12 s; to obserwacja diagnostyczna z dwóch przebiegów, nie izolowany, wielokrotnie powtarzany benchmark samej funkcji.
- Zachowana poprawność: testy blokady celu, zasięgu, śmierci, odrodzenia, szybkich pocisków oraz 600 kroków dużej symulacji zakończyły się powodzeniem.

## Granice wyniku i dalsze zasady projektu

To test fundamentu, jeszcze bez fizycznego rozpychania tłumu, nawigacji wokół przeszkód, dużych eksplozji z przezroczystością, dynamicznych cieni, bloom, pełnego interfejsu i dźwięku. Pociski sprawdzają trafienie przypisanego celu; test nie mierzy 100 000 swobodnych pocisków z dowolnymi kolizjami przeciw wszystkim jednostkom. Te mechaniki będą miały osobny koszt.

Dlatego celem dla rozbudowanej gry będzie **stabilne 60 FPS przy 1080p na tym komputerze**, z zachowaniem rezerwy czasowej, a nie utrzymanie kilkuset FPS z laboratorium. Każdy większy system należy oceniać na tych samych scenach, sprawdzając p99 czasu klatki, czas symulacji, GPU i wykorzystanie pamięci. Wyniki krótkich przebiegów trzeba uzupełniać dłuższymi sesjami przy rozwoju finalnej rozgrywki.

Na obecnym etapie nie ma powodu wybierać bardziej złożonego backendu wyłącznie dla nazwy Vulkan/DX12 ani od razu rozdzielać symulacji na wiele wątków. Aktualny pomiar wskazuje, że najwięcej CPU poza symulacją zajmuje przygotowywanie i przesyłanie danych renderera. Przyszłe decyzje o wielowątkowości, obliczeniach GPU i ograniczaniu efektów należy oprzeć na kolejnych pomiarach.

## Pliki do samodzielnego uruchomienia i weryfikacji

- [TEST_NATYWNY.bat](TEST_NATYWNY.bat) — podgląd z przełączaniem liczby jednostek i kamer.
- [BENCHMARK.bat](BENCHMARK.bat) — powtórzenie siedmiu scen; zapis nadpisuje poprzednie wyniki w katalogu `benchmark`.
- [Instrukcja i metodologia](native/README.md).
- [Zbiorcze wyniki CSV](artifacts/native/benchmark/summary.csv).
- [Parametry sprzętu](artifacts/native/hardware.json).
- [Środowisko i zakres testu](artifacts/native/benchmark/environment.txt).
- [Widok 10 000 bestii / 100 000 pocisków](artifacts/native/benchmark/10000_100000_fps.png).
- [Widok gęstego tłumu](artifacts/native/benchmark/5000_50000_dense_fps.png).

Surowe czasy każdej klatki i każdej próbki GPU są w oddzielnych plikach CSV w `artifacts/native/benchmark/`. Pomiary bazowe przed usunięciem obliczeń kątów są w `artifacts/native/5000_50000_tactical/`. Pierwszy krótki test techniczny jest w `artifacts/native/smoke/`; jego zrzut miał odwrócone kanały R/B podczas zapisu PNG, co zostało poprawione przed końcowym pomiarem i nie dotyczyło renderowania ani jego czasów.
