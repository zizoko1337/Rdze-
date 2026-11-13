# Działa manualne — zmiany

## Odczucie strzału: kamera i dźwięk

Każdy moduł ma własny krótki przechył kamery i sprężysty ruch do tyłu. Laser, Tesla, katapulta, moździerz, rakiety i snajperka wywołują impuls przy rzeczywistym strzale. Moździerz i snajper mają mocniejszy odrzut niż rakiety; plazmowy wariant lasera ma własny cięższy impuls. Kamera wraca płynnie, a odrzut modelu działa równolegle. Sam przechył i przesunięcie wizualne zachowują centralny kierunek celowania; snajperka ma dodatkowo opisane poniżej krótkie podbicie w górę.

Minigun ma drobne drgania podczas trzymania spustu i cichy mechaniczny podkład. Nawet Extreme Cooling nie kumuluje nieograniczonego odrzutu. Palnik ma delikatną wibrację i szum.

Lanca podczas aktywnej wiązki lekko drga i rzeczywiście znosi lufę: co 0,35–0,75 s wybiera nowy kierunek, płynnie dochodząc do prędkości około 0,275–0,475°/s (o 25% więcej niż początkowo). Na rezerwie uranowej rzeczywiste znoszenie jest dokładnie trzykrotnie większe niż przy zwykłej wiązce. Ruch można korygować myszą. Samo ładowanie nie znosi celownika. Po puszczeniu spustu ruch celowania ustaje. Pozycja kamery, kierunek lufy i wiązka pozostają spójne.

Snajperka oprócz przechyłu podbija kamerę w górę o około 0,28°. To krótki sprężysty impuls po strzale, płynnie wracający do poprzedniego kierunku. Oddany pocisk zachowuje kierunek z chwili naciśnięcia spustu.

Lanca ma narastający ton ładowania, cichy pomruk pulsujący 5,2 razy na sekundę podczas wiązki i łagodne wygaszenie po zakończeniu. Rezerwa uranowa dodaje nieregularne przytłumienia i trzaski; drgania są nieco mocniejsze. Pozostałe moduły dostały dodatkową, cichą warstwę mechanicznego uderzenia pod dotychczasowym dźwiękiem strzału.

Pauza zatrzymuje kamerę i wycisza dźwięk. Widok taktyczny i zejście ze stanowiska wyłączają efekty manualne; pomoc i ekrany przechwytujące sterowanie wyciszają podkład. **M** wycisza także nowe efekty dźwiękowe. Losowość kamery i dźwięku jest oddzielona od losowości walki i nagród.

Implementacja: `native/manual_feedback.hpp`, `native/manual_sound.hpp`. Testy: `native/manual_feedback_tests.cpp`, testy dotychczasowych broni i `native/render_tests.cpp` z rzeczywistymi przechyłami podczas ostrzału. Lokalny odsłuch lancy: `artifacts/manual-feedback/lance-charge-beam-reserve.wav` — ładowanie, podstawowa wiązka, rezerwa i wygaszenie.

## Mechaniki modułów

- **Laser, moździerz, Smart i snajper:** dotychczasowa mechanika.
- **Extreme Cooling:** bazowa szybkostrzelność nadal ×5, rozrzut teraz ×2 zamiast ×3. Ulepszona korba jest mniejsza, obraca się na górze odbiornika. Zwykły minigun zachowuje boczną korbę.
- **Tesla:** skoki wydłużono z 28 do 56 m. Trafienie ziemi staje się początkiem łańcucha; uderzenie w ziemię zajmuje pierwszy z 10 odcinków, pozostawiając do 9 trafionych przeciwników. Trafienie bezpośrednie nadal pozwala trafić do 10 różnych przeciwników. Dystans kolejnego skoku uwzględnia wysokość celu.
- **Toksyczna katapulta:** osobny model otwartej łyżki wyrzutowej, ramion, osi, dwóch mieszalników, tłoczków i przewodów. Po rzucie ramię wraca, a widoczny strumień dolewa toksynę do łyżki przed kolejnym strzałem. Zachowane są dotychczasowe obrażenia, lot, plama, spowolnienie i odnowienie.
- **Lanca:** wiązka jest ciągłą geometrią obliczaną z aktualnego położenia wylotu i kierunku celowania przy każdej renderowanej klatce. Nie zostawia starych odcinków pomiędzy krokami obrażeń. Pierwszy kontakt z daną jednostką podczas jednej serii daje bazowo 200 obrażeń, potem działa 1650 DPS, tak jak w aktywnej wiązce wieżyczki lancy. Ponowne przesunięcie po tej samej jednostce w tej serii nie dodaje kolejnych 200. Nowy przeciwnik w ponownie użytym slocie jest rozpoznawany poprawnie. Premie obrażeń i wspólnego celu działają.
- **REZERWA URANOWA:** unikalny Legendary, dostępny tylko dla wybranej lancy. Po podstawowych 3 sekundach wiązka działa dalej, zużywając 5 uranu/s. Wydobycie trwa równolegle. Wiązka i dodatkowa szara rura zasilająca zmieniają się na zielone. Puszczenie LPM albo zużycie paliwa kończy serię i włącza chłodzenie; podstawowa trzysekundowa seria pozostaje dostępna bez uranu. Można dodać moduł przez `;`, wyszukując „rezerwa”.
- **Miotacz:** PPM przełącza pomiędzy szerokim stożkiem 63 m a skupionym 94,5 m, szerokim na 5 m przy końcu. DPS i podpalenie pozostają takie same. Przełącznik, teleskopowy wylot, osłona i zwężająca się dysza pokazują zmianę. Celownik i podświetlenie podłoża odpowiadają rzeczywistemu trybowi i kierunkowi.

Testy mechanik: `native/manual_revision_tests.cpp` i dotychczasowe testy manualne oraz chłodzenia. Test graficzny `Rdzen.exe --revision-test` przechodzi po wszystkich dziewięciu broniach, sprawdza rzeczywiste trafienia, uruchamia rezerwę lancy i wysyła prawdziwe zdarzenie PPM do przełączenia miotacza. Zrzuty i raport są w `artifacts/command/manual-revision/` uruchomionej kopii.

Nalewak katapulty składa się teraz na przegubie: najpierw czeka podniesiony na powrót ramienia, opuszcza się, nalewa toksynę, podnosi się i dopiero wówczas odblokowuje następny rzut. Cykl skaluje się z szybkostrzelnością. Przewód rezerwy uranowej przeniesiono na górną lewą stronę, aby był widoczny z fotela. Każdy status zatrucia dodaje niezależnie 20 HP/s zwykłym bestiom albo 100 HP/s bossom; ta dodatkowa część nie kumuluje się wielokrotnie. Plama katapulty odświeża zatrucie na 6 s, więc część obrażeń pozostaje po wyjściu z plamy.
