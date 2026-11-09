# Extreme Cooling System

Zwykły manualny minigun stopniowo rozgrzewa końcówki sześciu luf podczas strzelania. Metal przy wylocie czerwienieje i zaczyna świecić. Po puszczeniu spustu stygnie. Temperatura jest efektem wizualnym — nie dodaje zacięć ani kar do podstawowej broni.

**Extreme Cooling System** to jednorazowe ulepszenie **Legendary**, dostępne w nagrodach wyłącznie podczas ekspedycji rozpoczętej z manualnym minigunem. Można je też znaleźć i dodać pod ;, wpisując „cooling”.

- Bazowa szybkostrzelność: **12 → 60 pocisków/s**.
- Dotychczasowe premie mnożą nową bazę: np. +50% daje 90 pocisków/s.
- Rozrzut zwiększa się **dwukrotnie**, z 0,025 do 0,05 radianów parametru stożka, przed innymi modyfikatorami rozrzutu.
- Obrażenia jednego pocisku pozostają równe 26. Nominalne bazowe DPS wynosi **1560** zamiast 312, przy założeniu wszystkich trafień.

Po bokach pojawiają się kriogeniczne zbiorniki, przewody i dysze skierowane na wyloty. Rozgrzane lufy uruchamiają krótkie strumienie białoniebieskiej mgiełki. Chłodzenie ma dwa progi temperatury, więc nie miga co klatkę na granicy załączenia. Zatrzymuje się po ostygnięciu. W celowniku widać temperaturę, stan chłodzenia i większy rozrzut.

Pociski nadal mają lot, rozrzut i sprawdzanie przecięcia odcinka z przeciwnikami. Bufor ma limit 1024 pocisków manualnych. Mgła powstaje w ograniczonym tempie, po cztery cząstki co 0,05 s i żyje 0,3 s. Nie zależy od liczby przeciwników. Modele i cząstki korzystają z istniejącego instancjonowania.

`native/cooling_tests.cpp` sprawdza rzeczywisty rozrzut i liczbę strzałów (12/60/90 na sekundę), narastanie i wygaszanie temperatury, mgłę przy gorących lufach, pauzę, unikalność i ograniczenie do miniguna. `TEST_CHLODZENIA.bat` przechodzi przez wybór miniguna, laboratorium, instalację, fotel i ostrzał. Próba graficzna używa rozgrzanej lufy jako przygotowanej sytuacji; naturalne rozgrzewanie jest sprawdzane oddzielnie. Wyniki: `artifacts/command/cooling/`.
