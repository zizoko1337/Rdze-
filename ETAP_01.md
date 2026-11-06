# Ekspedycja 2137 — etap 1

Opis historycznego zakresu etapu 1. Aktualny rozwój kart i obrony: [etap 2](ETAP_02.md).

`START.bat` uruchamia zintegrowaną pętlę: obrona rdzenia → wydobycie uranu → odbiór transportu → wypłata przy komputerze → jedno ulepszenie z trzech propozycji → dalsza obrona.

## Zakres

- Rdzeń: 1500 początkowego HP. Obrażenia od przeciwników rzeczywiście obniżają HP. Przy zerze następuje porażka, zatrzymanie symulacji i ekran wyników. R lub przycisk nowej ekspedycji zeruje podejście.
- Wydobycie: 2 uranu/s, 100 uranu na ładunek. Magazyn startuje pusty. Nagroda powstaje natychmiast po zapełnieniu paska, pierwszy raz po około 50 s. Dron odbiera ładunki osobno, w cyklach po 10 s. Wydobycie i wypłaty nie czekają na odbiór.
- Duży transportowiec 3D ma cztery zespoły napędowe z obracanymi elementami, kontener i animowane podejście/odlot. Podwieszony kontener jest zabierany z zewnętrznej platformy załadunkowej.
- Dwie boczne konsole na pokładzie obsługują ten sam terminal. E z bliska otwiera komputer. Wybór odbywa się myszą lub 1/2/3. E/Esc zamyka terminal bez losowania nowych propozycji. Terminal zatrzymuje symulację.
- Pięć kart: +25% bazowych obrażeń manualnych, +20% bazowej szybkostrzelności manualnej, +25% bazowego wydobycia, +250 maksymalnego HP z naprawą 250 HP, jeden automatyczny laser. Każda wypłata daje trzy różne propozycje i pozwala wybrać dokładnie jedną.
- Lasery wybierasz ręcznie w widoku taktycznym spośród 210 heksagonalnych stanowisk, oddalonych od siebie o 21 m. Karta daje jedną wieżyczkę do magazynu, a kliknięcie heksu zamawia zrzut na balonie. Wybieranie miejsca zatrzymuje czas do wydania ostatniej wieżyczki lub powrotu na pokład. Laser strzela dopiero po lądowaniu; balon odczepia się i wraca z napędem w górę. Pełny limit, razem z magazynem i dostawami, wycofuje karty nowych wieżyczek z losowania. Nie wprowadzono jeszcze rzadkości kart, kryształów, bossów w ekspedycji, radaru ani rozwoju między podejściami.
- Początek jest łagodniejszy, a presja rośnie z czasem, niezależnie od liczby wypłat. Paski HP przeciwników pozostają ukryte.

## Weryfikacja

Testy mechanik sprawdzają dokładny próg wydobycia i natychmiastową wypłatę, brak drugiej wypłaty po odlocie, kolejne wypłaty podczas lotu, kolejkę ładunków i nagród, unikalność propozycji, brak ponownego losowania po otwarciu terminala, odrzucanie błędnych wyborów oraz jednokrotne zużycie karty.

Oddzielne przypadki weryfikują rzeczywiste obrażenia rdzenia, zatrzymanie walki i wydobycia w pauzie/terminalu, śmierć i zamrożenie przegranego podejścia. Każda karta ma sprawdzany efekt: obrażenia strzału, zwiększona liczba strzałów w zadanym czasie, tempo wydobycia, maksymalne HP i naprawa oraz laser, który faktycznie eliminuje przeciwnika. Sprawdzane są także limity instalacji i aktualizacja nieaktualnej karty w kolejce.

Naturalna symulacja bez strzelania i bez skracania czasu transportu po 60,5 s dostarczyła pierwszy ładunek, przy pozostających około 813 HP rdzenia. Dalsza bezczynność prowadzi do porażki. To kontrola osiągalności pierwszej wypłaty, nie finalny balans trudności.

Test interakcji `START.bat --expedition-test` przechodzi przez rzeczywiste zdarzenia klawiatury i myszy: start, podejście do konsoli, E, zamrożenie terminala, przylot i odlot, wybór karty, jej zastosowanie, uszkodzenie rdzenia do zera i restart. Aby skrócić tę próbę, wyłącznie test ustawia początkowy magazyn na 99,9 uranu oraz obniża końcowe HP. Normalna gra nie stosuje tych skrótów.

Raport automatyzacji: [expedition-result.json](artifacts/command/laser/expedition-result.json). Zrzuty: [transportowiec](artifacts/command/laser/expedition-02-freighter.png), [wybór karty](artifacts/command/laser/expedition-03-reward.png), [porażka](artifacts/command/laser/expedition-05-defeat.png).

Powtórzenie: `PRZEBUDUJ.bat` uruchamia testy mechanik i kompilację, `TEST_EKSPEDYCJI.bat` sprawdza pełną pętlę interakcji. Dawne próby dział i arsenału pozostają dostępne.
