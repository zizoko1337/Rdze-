# Tarcza energetyczna

Jednorazowa karta **Epic: Tarcza energetyczna** zamawia generator Jan Kowalski Company. Skrzynka jest pomniejszona do 38% pierwotnych wymiarów (około 1,56 m szerokości i wysokości), dron do 28%. Dron schodzi przed bazą, przelatuje pod okapem i nad balustradą pomiędzy słupami, a dopiero wewnątrz opuszcza skrzynkę na podłogę po wschodniej stronie, w wolnym miejscu obok radaru: x=9,5 m, z=4 m. Dostawa trwa 8 s, następnie dron wraca tą samą drogą i odlatuje do 12 s. Generator ma kolizję podczas chodzenia, mieści się pod radarem i nie wystaje w pole ostrzału działa manualnego. Zewnętrzna półka została usunięta.

Po odstawieniu skrzynki trwa **2,2 s fizycznego rozruchu**. Z obu bocznych gniazd przez pierwsze 1,25 s wysuwają się teleskopowe anteny z izolatorami. Od 1,3 s między końcówkami zapala się poszarpany, błękitno-biały łuk elektryczny. Dopiero po zakończeniu rozruchu tarcza otrzymuje pełne HP i zaczyna chronić rdzeń. Podczas dostawy i rozruchu obrażenia przechodzą na rdzeń. Podniesione anteny pozostają pod dachem i w obrysie kolizji generatora.

Po prawej stronie obracają się dwie zębatki o różnych średnicach i przeciwnych kierunkach ruchu. Po lewej są rurki, mosiężne obejmy oraz pracujący tłok. Mechanizm działa wolniej podczas odbudowy, a łuk gaśnie na czas wyłączenia osłony. Po odbudowie łuk wraca. Ruch elementów i zmiany łuku korzystają z zegara symulacji generatora i zamierają podczas pauzy.

Parametry początkowe:

- 1000 HP, początkowo pełne po dostawie i rozruchu.
- Regeneracja 10 HP/s, do maksymalnej pojemności.
- Obrażenia przeciwników najpierw zużywają tarczę; nadmiar z tego samego trafienia przechodzi na rdzeń.
- Zniszczenie całkowicie wyłącza osłonę na 60 s. Dalsze trafienia ranią rdzeń i nie przedłużają tego czasu.
- Po odliczeniu tarcza odbudowuje się do pełna. Rdzeń nie jest przy tym naprawiany.

Pauza, terminale, laboratorium i inne menu zatrzymujące grę zatrzymują także dostawę, regenerację i odliczanie. Przed rozpoczęciem wydobycia zamówiony generator może zostać dostarczony podczas swobodnego chodzenia; czas nie płynie w menu. Śmierć rdzenia zatrzymuje system. Nowa ekspedycja zeruje zakup i ulepszenia.

Cylindryczna osłona ma siatkę heksagonalną, pulsowanie podtrzymania i silniejszy błysk po trafieniu. Sięga od ziemi do 21,15 m, pod okapem dachu (21,4 m). Jest widoczna również od środka; wewnętrzny efekt ma 42% zewnętrznej intensywności, aby pokazywać pracę osłony bez silnego zasłaniania świata. Osłona nie wychodzi nad dach, więc widok z podniesionego działa pozostaje czysty ponad jej krawędzią. Jej geometria nie przechwytuje własnych pocisków i nie zmienia trajektorii dział.

Na skrzynce są **dwa okrągłe, fizyczne liczniki ze wskazówkami**, mosiężnym obrzeżem, jasną tarczą i podziałką. Lewy pokazuje integralność 0–100%, prawy sekundy pozostałe do odbudowy (z maksimum dopasowanym do ulepszeń). Zastępują wcześniejszy prostokątny ekran i pasek. Na przedniej ścianie pozostaje naklejka **Jan Kowalski Company**, pan w koszuli z brzuszkiem i kciukiem w górze oraz hasło **„hydraulika jądrowa, mycie okien i tarcze energetyczne”**. HUD nadal pokazuje dokładne HP, rozruch i sekundy odbudowy. [Grafika i pełny prompt](ASSETS-TARCZA.md).

## Ulepszenia

Dostępne dopiero po kupieniu generatora. Każdy parametr ma niezależne pięć wyborów, kolejno **Common → Uncommon → Rare → Epic → Legendary**. Nie można przeskoczyć poziomu ani wylosować zakończonej ścieżki.

| Parametr | Common | Uncommon | Rare | Epic | Legendary | Po wszystkich pięciu |
|---|---:|---:|---:|---:|---:|---:|
| Dodatkowe maksymalne HP | +125 | +250 | +375 | +500 | +750 | 3000 HP |
| Dodatkowa regeneracja | +1 HP/s | +2 HP/s | +3 HP/s | +4 HP/s | +6 HP/s | 26 HP/s |
| Krótsza odbudowa | −2,5 s | −5 s | −7,5 s | −10 s | −15 s | 20 s |

Większa pojemność nie daje darmowego leczenia: uzupełnia ją regeneracja albo następna pełna odbudowa. Skrócenie odbudowy podczas wyłączenia zachowuje procent pozostałego czasu. Premie obowiązują też podczas trwającej dostawy; generator uruchamia się już z nowymi parametrami.

Implementacja: `native/energy_shield.hpp`, `native/energy_shield_scene.hpp`, integracja `native/expedition.hpp`, `native/command_game.hpp` i `native/command_app.hpp`. Proceduralny shader heksów używa jednej siatki cylindra; nie powstaje osobny obiekt dla każdego heksa.

Testy: `native/energy_shield_tests.cpp`, `native/energy_shield_visual_tests.cpp`. Test graficzny uruchamia ukryte okno i zapisuje kadry oraz `result.json` w `artifacts/command/energy-shield/`. Sprawdza widoczność heksów od środka oraz czysty widok ponad osłoną z podniesionego działa. Testy geometrii sprawdzają prześwit drona i ładunku względem dachu, balustrady, podłogi i słupów.
