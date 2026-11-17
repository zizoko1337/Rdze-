# Odnawialne wsparcie orbitalne

**Łącze wsparcia orbitalnego jest gwarantowaną propozycją w jednym z pierwszych pięciu rozdań.** Jeśli nie wypadnie wcześniej, pojawi się w piątym jako Rare. Liczą się wszystkie normalne rozdania: z uranu i kryształów. Wybranie tej karty pozostaje dobrowolne; gwarancja nie instaluje ulepszenia automatycznie. Zdobyte łącze nie pojawia się ponownie.

Lewy komputer nadal obsługuje nagrody. Prawy pozostaje zgaszony i niedostępny do czasu zdobycia unikalnej karty **Łącze wsparcia orbitalnego (Rare)**. Karta włącza ekran oraz kontrolki i daje po jednym gotowym użyciu każdej usługi. Działa po rozpoczęciu wydobycia.

Każda z trzech usług ładuje się **osobno, bazowo przez 240 s**. Czas płynie podczas walki i poruszania się po pokładzie oraz w zwykłym widoku taktycznym. Pauza, terminale, wybieranie celu i inne ekrany zatrzymujące symulację zatrzymują także ładowanie. Zapas i postęp widać na fizycznym ekranie oraz w panelu komputera. Pełny magazyn zatrzymuje ładowanie; dodatkowy czas nie jest zachowywany ponad limit.

| Karta | Rzadkość | Działanie |
| --- | --- | --- |
| Łącze wsparcia orbitalnego | Rare | Jednorazowe odblokowanie wszystkich usług i po jednym gotowym użyciu |
| Szybsze ładowanie usług | Uncommon | +30% bazowego tempa ładowania, do 5 kart; czas wynosi `240 / (1 + 0,3 × liczba kart)` s |
| Magazyn wsparcia | Rare | +1 miejsce w osobnym magazynie każdej usługi, do 3 kart; maksymalnie po 4 użycia |

Karty tempa i magazynu pojawiają się dopiero po odblokowaniu komputera i znikają po osiągnięciu limitów. Ulepszenie tempa zachowuje procent rozpoczętego ładowania. Powiększenie magazynu tworzy wolne miejsca do naładowania. Reset ekspedycji zeruje cały rozwój wsparcia.

| Usługa | Cel | Efekt |
| --- | --- | --- |
| Brimstone | Punkt na ziemi lub boss | Po ostrzeżeniu 3 s: 25 000 obrażeń łącznie w sześciu impulsach co 0,5 s, promień 10 m. Wskazany boss jest śledzony |
| Paraliż | Bez wskazywania miejsca | Po 3 s ogłusza wszystkich obecnych przeciwników, również bossów, na 5 s; trucizna i podpalenie działają dalej |
| Cyberświnie | Wolny heks bez wieżyczki ani zamówionej dostawy | Cztery drony na czterech linach opuszczają klatkę przez 6 s. Po lądowaniu odlatują. Po kolejnych 2 s klatka rozpada się i wypuszcza dwie cyberświnie |

Wybór usługi odbywa się klawiszami 1/2/3 lub myszą. E/Esc zamyka komputer. Brimstone i cyberświnie przechodzą do wskazania celu na mapie; PPM/Esc/Tab anuluje bez zużycia użycia. Usługa pobiera własny ładunek dopiero po poprawnym zamówieniu. Fala pobiera go od razu przy wyborze. Usługi nie korzystają już z punktów ani z poświęcania wieżyczek.

## Cyberświnie

Dwie jednostki biegną w przeciwnych kierunkach po pierścieniu o promieniu równym odległości wybranego heksu od rdzenia. Podgląd przy celowaniu pokazuje ten pierścień. Prędkość biegu wynosi 28 m/s, więc dalszy zrzut działa dłużej. Każda świnia wykonuje dokładnie pięć okrążeń, potem przestaje atakować i ucieka na zewnątrz mapy; po 18 s ucieczki znika.

- Lasery: 200 obrażeń co 0,4 s, do 70 m, z ograniczeniem do przedniej części pola widzenia. Rdzeń zasłania strzał. Lasery mogą trafiać jednostki latające.
- Taranowanie: 1500 nominalnych obrażeń przy przecięciu ścieżki naziemnego przeciwnika. Każda świnia może trafić tę samą jednostkę raz na okrążenie. To chroni przed naliczaniem obrażeń w każdej klatce przy stałym kontakcie. Nowe jednostki korzystające z ponownie użytego slotu nie dziedziczą odporności.
- Podczas biegu i ucieczki pozostaje krótki kurz. Profil świni jest odbijany zgodnie z ruchem względem aktualnej kamery. [Grafika i pełny prompt](ASSETS-CYBER-SWINIE.md).

Heks jest zarezerwowany do uwolnienia świń. W tym czasie nie można zamówić tam drugiej klatki ani wieżyczki; istniejąca lub nadlatująca wieżyczka uniemożliwia zrzut. Po uwolnieniu heks jest ponownie dostępny. Żadna wieżyczka nie jest niszczona przez usługę. Wspólna pula jest ograniczona do 64 aktywnych świń, z uwzględnieniem zamówionych par. Przy braku miejsca zamówienie zachowuje ładunek. Kontakt używa siatki przestrzennej i stałego indeksowania po jednostce, bez wyszukiwania w historii trafień.

## Próby

`POLIGON_USLUG.bat` włącza komputer, daje po 4 użycia i 20 000 HP oraz trzy wybory ulepszeń. Przycisk wydobycia nadal trzeba nacisnąć. Normalny `START.bat` rozpoczyna z wyłączonym komputerem.

`native/services_tests.cpp` zachowuje testy przycisku, konsol, Brimstone i paraliżu. `native/cyber_pig_tests.cpp` sprawdza odblokowanie i karty, osobne odnawianie, pauzę, pojemność, heksy i rezerwacje, czas dostawy, obrażenia, pięć okrążeń, ucieczkę, limit pamięci i scenariusz 6000 przeciwników z 64 świniami. `TEST_USLUG.bat` wykonuje interakcje przez klawiaturę i mysz oraz zapisuje podglądy i raport w `artifacts/command/services/`.
