# Plazmowa dubeltówka

Dziesiąte działo manualne, wybierane w menu myszą lub klawiszem **0**. Na razie jest dostępne bez osiągnięcia.

- Dwa osobne strzały, naprzemiennie z lewej i prawej lufy. Każdy wymaga osobnego naciśnięcia LPM; przytrzymanie nie zużywa drugiej baterii.
- Każdy strzał wypuszcza **10 pocisków po 3000 obrażeń**, czyli 30 000 nominalnie. Dwie lufy dają łącznie 60 000 przed przeładowaniem, jeśli wszystkie pociski trafią. Rozrzut, czas lotu i przeszkody nadal mają znaczenie.
- Minimalny odstęp między strzałami: 0,65 s. Zasięg 360 m, prędkość plazmy 220 m/s. Każdy pocisk trafia tylko pierwszy cel.
- Każdy strzał podnosi rzeczywisty kierunek celowania o **10°**, do mechanicznego ograniczenia stanowiska. Pociski opuszczają lufę w kierunku sprzed odrzutu. Dodatkowo kamera mocno przechyla się i cofa, a syntetyzowany huk ma basowe uderzenie oraz elektryczny ogon.
- Po drugim strzale automatycznie zaczyna się **12-sekundowe przeładowanie**. Premia tempa skraca je, ale nie poniżej 10 s; zakres mechanizmu to 10–15 s. Pauza i otwarty terminal zatrzymują animację.

Dwa teleskopowe siłowniki najpierw unoszą zespół luf o 1,2 m i odsuwają go o 55 cm od operatora na prowadnicach nad obniżonym, nieruchomym korpusem; dopiero potem lufy łamią się na zawiasie. Za fotelem stoi stały magazynek z rozsuwanymi osłonami. Jego moduł ładowania wysuwa się na górnych prowadnicach i opuszcza nad obie komory. Dwa uchwyty jednocześnie chwytają zużyte baterie, wyciągają je i zabierają do magazynka. Po schowaniu ogniw moduł wraca z dwiema naładowanymi bateriami, osadza je jednocześnie, zwalnia uchwyty i chowa się za fotelem. Cały automat pozostaje połączony z prowadnicami i siłownikami. Zużyte ogniwa są przygaszone, świeże mają jasne pierścienie plazmowe. Po zamknięciu luf dźwignia na górze korpusu wykonuje widoczny ruch do tyłu i do przodu, obracając zębatki i przesuwając tłok. Dopiero po jej powrocie podnośnik opuszcza lufy do pozycji strzeleckiej. Cały cykl nadal trwa 12 s. Mechanizm działa bez dłoni i modelu postaci. HUD pokazuje pozostały strzał, etap obsługi i czas do gotowości.

Model ma dwie duże otwarte lufy, stalowy korpus, drewniane akcenty, mosiężne obręcze i świecące emitery. Nowe tekstury i ich pełne prompty: [ASSETS-DUBELTOWKA.md](ASSETS-DUBELTOWKA.md).

W pozycji spoczynkowej w bazie moduł ładowania jest obniżony o 55 cm pod zamkniętym dachem; płynnie wraca do wysokości roboczej wraz z windą stanowiska. Środkowa listwa ma od strony operatora stalowy mostek ze śrubami i wspornikami łączącymi ją z obiema lufami. Mocowanie podnosi się i łamie razem z lufami.

`native/plasma_shotgun_tests.cpp` sprawdza dwie salwy po 30 000 faktycznych obrażeń, osobny spust, naprzemienne lufy, odrzut 10°, pauzę, blokadę trzeciego strzału i ciągłość ruchu modułu, powrót do magazynka i wymianę pary ogniw. `native/plasma_shotgun_visual_tests.cpp` zapisuje widoki gotowej broni, 15 etapów przeładowania, widoku operatora, menu oraz próbkę dźwięku w `artifacts/command/plasma-shotgun/`.
