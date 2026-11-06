# Poprawka migotania obrazu — 2026-11-03

## Jednolite podłoże — 2026-11-04

Na życzenie usunięto z podłoża cały proceduralny wzór: ziarnienie i linie. Ziemia ma teraz jednolity, matowy kolor, na który wpływają tylko oświetlenie i mgła. Nie jest próbkowana żadna tekstura podłoża.

Po dalszym zgłoszeniu usunięto również stary model podłoża, który był ogromną kostką z górną powierzchnią złożoną tylko z dwóch trójkątów rozciągniętych na 6,4 km. Nowe podłoże jest osobną płaską siatką 128 × 128 pól, bez boków i spodniej ściany. Test obrazu zawiera teraz po 600 klatek dla obu broni: działo jest skierowane 30° w dół, obraca się z prędkością około 3,4°/s i strzela bez przerwy.

## Doprecyzowanie: migotanie tekstur — 2026-11-04

Po informacji, że błysk przypomina błąd tekstury, zbadano osobno podłoże. Jego wzór był wyliczany z losowej wartości dla każdej komórki, bez wygładzania granic ani uwzględnienia rozmiaru piksela. Drobny ruch kamery zmieniał wzór w oddali skokowo. Zastąpiono go płynnie interpolowanym szumem, wygaszanym do średniej, gdy szczegół jest mniejszy niż piksel. Wygładzono także cienkie linie podłoża. Tekstury materiałów korzystają teraz z filtrowania anizotropowego 16×.

Pomiar izoluje podłoże i 32 razy renderuje niemal tę samą kamerę, przesuwaną o ułamek piksela. Średnia bezwzględna zmiana próbkowanych kanałów RGB (skala 0–255) spadła z **0,357407 do 0,00216049**. To wynik tej konkretnej sceny pomiarowej, nie miara wszystkich możliwych błysków. Test zgłasza teraz błąd, gdy zmiana przekroczy 0,1. Dodatkowe 720 klatek ciągłego strzelania nadal przeszło bez pustych klatek.

Porównanie: [przed](artifacts/render/before-terrain-filter.json), [po](artifacts/render/result.json). Ta poprawka usuwa potwierdzone migotanie proceduralnego podłoża; nie dowodzi, że odtworzono dokładnie każdy opisany przez użytkownika błysk.

Zgłoszenie: sporadyczne migotanie lub bardzo krótki czarny obraz podczas obsługi dział.

Zmiany w `native/renderer.hpp`:

- Pomijanie ponownego tworzenia buforów obrazu, gdy SDL zgłasza niezmieniony rozmiar okna. Wcześniej każde takie zgłoszenie wywoływało `ResizeBuffers`.
- Prezentacja przez `DXGI_SWAP_EFFECT_FLIP_DISCARD` zamiast starszego `DISCARD`. Bufor renderowania jest ponownie przypinany na początku każdej klatki. Wymaga Windows 10 lub nowszego. To model prezentacji zalecany przez [dokumentację Microsoft](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/for-best-performance--use-dxgi-flip-model).
- Stabilny wektor szerokości wiązki, także gdy kamera leży na jej osi albo odcinek ma zerową długość. Usunięto normalizowanie zerowego iloczynu wektorowego w tych przypadkach.
- Krótkie odczekanie, gdy DXGI zgłasza całkowite zasłonięcie okna.

## Weryfikacja

`native/render_tests.cpp` wykonuje 720 klatek dla lasera i miniguna. Kamera obraca się podczas ciągłego strzelania i obejmuje pełny zakres pochylenia. Test przełącza okno i pełny ekran oraz VSync, dodaje wiązki pokrywające się z osią patrzenia i odcinki zerowej długości. Sprawdza także 24 powtórne zgłoszenia tego samego rozmiaru okna.

Wynik: **720 sprawdzonych klatek, 0 pustych/czarnych klatek w próbkowanych obszarach bufora, 24 poprawne próby powtórnego rozmiaru**. Testy mechanik i sterowania obu dział również przeszły.

Raport: [result.json](artifacts/render/result.json). Dane klatek: [frames.csv](artifacts/render/frames.csv). Powtórzenie: `TEST_OBRAZU.bat`.

Nie udało się jednoznacznie odtworzyć pierwotnego, sporadycznego błysku użytkownika. Test odczytuje wyrenderowany bufor przed prezentacją; nie mierzy fizycznego obrazu monitora ani zakłóceń powstających dalej w sterowniku lub torze wyświetlania. Wprowadzone poprawki usuwają znalezione problemy i stabilizują prezentację, ale ich skuteczność wobec konkretnego zgłoszenia wymaga sprawdzenia podczas zwykłej gry.
