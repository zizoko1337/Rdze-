# Laserowy moździerz i laboratorium ulepszeń

Laser i minigun są podstawowymi działami startowymi. Pozostałe moduły nadal są dostępne w prototypie; odblokowania między rozgrywkami dojdą później.

**Laserowy moździerz** jest jednorazowym ulepszeniem **Legendary**, losowanym wyłącznie w podejściu rozpoczętym z manualnym laserem. Minigun ma własne legendarne ulepszenie Extreme Cooling System.

Opis karty: „Nielegalna technologia eksperymentalna. Nie została zatwierdzona, ale raczej nikt się nie dowie, że jej używasz. Salwa 20 impulsów lasera, rozrzut w promieniu 10 m.”

Głowica ma 17 widocznych końcówek po usunięciu trzech środkowych najbliżej operatora z układu 5×4, przypominających laserową szczotkę, oraz boczne przewody z pulsującymi komorami. Każdy impuls startuje z własnego emitera i leci łukiem. Celownik pokazuje obrażenia salwy, ziemia — okrąg o promieniu 10 m. Fotel, winda i wcześniejsze premie pozostają wspólne.

Salwa kosztuje jeden cykl standardowego lasera. Bazowo: 20×150 = **3000 nominalnych obrażeń na salwę**, raz na sekundę. Premie obrażeń i szybkostrzelności działają jak przed przemianą. Impulsy rozchodzą się po dysku; jeden trafia w jego środek. Każdy może trafić jedną jednostkę. To łączna moc 20 trafień, a nie gwarantowane 3000 obrażeń dla jednego przeciwnika. Strzały mogą chybić i nie mają dodatkowych eksplozji obszarowych.

Pool ma limit 1024 pocisków manualnych. Salwa jest przyjmowana w całości albo pomijana przy braku 20 miejsc. Cel sprawdzany jest na miejscu lądowania, bez wyszukiwania przeciwników w każdej klatce lotu każdego impulsu.

## Panel pod średnikiem

Podczas gry naciśnij **;**. Lista obejmuje wszystkie 31 ulepszeń. Wpisz nazwę, kategorię lub fragment opisu; polskie znaki nie są konieczne, np. „mozdzierz” znajduje „moździerz”.

- Góra/dół lub kółko myszy: wybór.
- Tab lub przyciski rzadkości: Common–Legendary.
- Enter lub „Dodaj ulepszenie”: przyznanie karty.
- Backspace: usuń znak; Ctrl+A / Ctrl+Backspace: wyczyść wyszukiwanie.
- ; lub Esc: zamknij.

Panel zatrzymuje czas. Nie zużywa ani nie losuje ponownie nagród z uranu i kryształów; stosuje rzeczywiste efekty kart. Wieżyczki po zamknięciu panelu rozmieść na heksach. Przy ręcznym dodawaniu pomijany jest limit 4 typów bojowych i 2 wsparcia; nadal obowiązuje 210 stanowisk. Normalne losowanie zachowuje własne reguły.

Unikalnych modułów nie można dublować, a ulepszenia fabryki wymagają jej zdobycia. Minimalna rzadkość jest respektowana: plazma zawsze Legendary, usługa Rare, drobne premie Common. Wszystko dotyczy bieżącej ekspedycji; R zeruje rozwój.

`native/plasma_tests.cpp` sprawdza losowanie, ograniczenie do lasera, zachowanie nagród, 20 oddzielnych trafień, 10 m rozrzutu, lot, premie, pauzę, pool, wyszukiwanie i dodawanie kart. `TEST_PLAZMY.bat` sprawdza zdarzenia SDL: ;, wpisanie „plazm”, Enter, zamknięcie, fotel, windę i ostrzał. Wyniki: `artifacts/command/plasma/`.

Każdy impuls ma promień trafienia 1 m, dodawany do hitboxa przeciwnika. Nadal trafia tylko jedną jednostkę. Prędkość każdego impulsu losowana jest niezależnie w zakresie 60-85 m/s; także przy dalekich strzałach czasy dolotu są różne. Salwa zachowuje 20 impulsów z 17 widocznych emiterów — trzy emitery wysyłają po dwa impulsy.

Dla laserowego moździerza wyłączono linię podglądu trajektorii; widoczny pozostaje okrąg rozrzutu na ziemi. Rzeczywiste impulsy nadal mają swoje smugi.
