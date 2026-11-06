# Ręczne rozmieszczanie wieżyczek i balony transportowe

- 210 stanowisk: regularna siatka heksagonalna o promieniu ośmiu pól z wyłączonym środkowym polem i jego sześcioma sąsiadami. Rdzeń zajmuje wolny środek. Sąsiednie stanowiska są oddalone o 21 m (wcześniej 18 m).
- Karta dowolnej z dziewięciu wieżyczek bojowych albo trzech rodzajów wsparcia daje jedną sztukę właściwego typu do magazynu i przełącza na kamerę taktyczną. Kliknięcie wolnego heksu rezerwuje go i zamawia zrzut. Powrót na pokład nie zużywa nieustawionej wieżyczki. Podgląd pokazuje jej model oraz zasięg z bonusami.
- Ładunek schodzi z wysokości 100 m przez 6 sekund, wyhamowując przy ziemi. Po lądowaniu powstaje aktywna wieżyczka; przedtem nie strzela. Liny odczepiają się, balon uruchamia niebieski napęd i przyspiesza w górę. Po odlocie jest usuwany, a laser pozostaje.
- Pauza, terminal i wybieranie miejsca z niepustym magazynem zatrzymują całą symulację, w tym dostawy. Po zamówieniu ostatniej wieżyczki lub wyjściu z widoku taktycznego czas rusza. Zajęte i zarezerwowane pola nie przyjmują kolejnego zrzutu.
- Limit kart wynosi 210, łącznie z jednostkami w magazynie i dostarczanymi. Pełny limit usuwa nową wieżyczkę z puli losowania. Stare oferty czekające w kolejce są weryfikowane przy otwarciu komputera, aby nie proponowały nieużytecznej karty.
- Stanowisko odbioru uranu przeniesiono poniżej pokładu, w przerwę pomiędzy heksami. Dokujący dron również pozostaje poniżej poziomu operatora. Test sprawdza brak kolizji stanowisk wieżyczek z platformą załadunkową i odsunięcie kadłuba drona od rdzenia.

Sterowanie: Tab — widok taktyczny/pokład, LPM — zamówienie zrzutu, WASD — przesuwanie kamery, Q/E — obrót, kółko — zoom, P — pauza, PPM/Esc — powrót z widoku taktycznego.

Testy mechanik obejmują geometrię i odstępy wszystkich 210 stanowisk, invalid index, rezerwację miejsca, zachowanie magazynu, automatyczne zatrzymanie czasu przy wyborze i wznowienie po ostatnim zamówieniu, pauzę zrzutu, pojedyncze lądowanie, przyspieszenie odczepionego balonu i jego usunięcie. Osobny przypadek dostarcza wszystkie 210 wieżyczek i sprawdza wycofanie kolejnych kart. Wypłata za uran pojawia się po zapełnieniu paska, przed odlotem drona.

`TEST_DOSTAWY.bat` wykonuje rzeczywiste wejście do terminala, wybór karty wieżyczki, kliknięcie heksu, opadanie, pauzę, lądowanie, przyspieszony odlot i powrót na pokład. W teście magazyn uranu jest prawie pełny, a pierwsza oferta zawiera laser; zwykłe losowanie nie jest zmienione.

Raport: [delivery-result.json](artifacts/command/laser/delivery-result.json). Zrzuty: [siatka](artifacts/command/laser/delivery-01-grid.png), [lądowanie](artifacts/command/laser/delivery-03-landed.png), [powrót balonu](artifacts/command/laser/delivery-04-return-boost.png).
