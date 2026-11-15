# JEŹDZIX POL SP z.o.o

Fabryka jeździków to sześć niewielkich zakładów, 17,6 m od środka rdzenia, między najbliższymi heksami. Nie zabierają stanowisk wieżyczkom.

Po odblokowaniu karty sześć dronów transportowych dostarcza zakłady. Drony podlatują przez 12 s z różnych kierunków, z odległości 145–215 m od środka bazy. Mają osobno losowane wysokości i łuki podejścia. Ładunki lecą pod nimi przez całą trasę. Po wyhamowaniu drony opuszczają fabryki, odczepiają zawiesia i odlatują w górę oraz poza bazę. Następnie z rdzenia wysuwają się przewody zasilające. Produkcja startuje po podłączeniu całej szóstki, około 21,2 s od rozpoczęcia dostawy. Dostawa i produkcja zatrzymują się wraz z czasem gry.

Bazowa seria: **6 jeździków co 12 s**, jeden z każdej fabryki. Ulepszenia ilości zwiększają całą serię. Przy 10 sztukach rozdział wynosi 2/2/2/2/1/1; przy 128 sztukach 22/22/21/21/21/21. Reszta jest rotowana, żeby suma produkcji również rozkładała się równo.

Każdy zakład ma bramę, przewód, lampkę i logo. Maszyny wyjeżdżają z sześciu bram, w odstępach 0,09 s wewnątrz każdego zakładu. Oczekujące i aktywne maszyny wspólnie respektują istniejący limit 2048. Kolejki mają sześć liczników, bez dodatkowego AI i osobnych obiektów dla oczekujących maszyn.

## Osłona rdzenia

Wieżyczki automatyczne nie wybierają celów zasłoniętych cylindrem rdzenia. Dotyczy to także wspólnego celu, priorytetów snajpera i lancy, obszarowych wyładowań Tesli oraz stożka palnika. Wystrzelone rakiety i kapsuły trucizny mają sprawdzanie całego odcinka ruchu, więc nie przelatują przez rdzeń między klatkami.

Wyjątkiem jest **moździerz** z wysokim łukiem lotu. Strzały rzeczywiście biegnące ponad cylindrem pozostają dozwolone. Obszary wybuchów i już istniejące efekty nie są prostymi strzałami.

Testy: `native/logistics_tests.cpp`, `native/frontier_tests.cpp`, `--factory-test` i raport `artifacts/command/factories/factories-result.json`.

## Grafika

Wbudowane narzędzie `imagegen`, przezroczyste tło. Finalny plik: `public/assets/jezdzix-pol-logo.png`. Jest współdzielony przez sześć dronów i sześć fabryk.

Pełny prompt:

> Create a finished humorous Polish industrial company logo sticker for a sci-fi rover delivery company. Exact clearly legible text: "JEŹDZIX POL" on the main line and "SP z.o.o" below it. A cheerful cartoon tiny four-wheeled rover wearing a construction hard hat and holding a small wrench beside the text, funny working-class space logistics mascot. Bold chunky lettering, Polish diacritic Ź correct, high contrast yellow and dark petrol navy, small mint-green electric accent. Wide rectangular sticker plaque, approximate 3:1 composition inside a transparent canvas, clean bold graphic suitable on a cargo drone and little robot factories. Flat straight-on design, no perspective, no photographic background, no additional words or watermarks. Genuine transparent background outside the plaque. Finished raster game texture.
