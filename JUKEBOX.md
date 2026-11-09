# URANIUM FM 2137

Po uruchomieniu wydobycia przycisk chowa się pod pokład. Pokrywy pozostają rozsunięte, a między 1,65 i 3,45 s wyjeżdża jukebox. Radio nie blokuje widoku z podniesionego działa. Jego obudowa i otwór w pokładzie mają kolizję podczas chodzenia.

Projekt: ciemna przemysłowa obudowa, turkusowe listwy, bursztynowe oznaczenia, anteny, dwa głośniki z kratkami, pokrętła i wyświetlacz. Nazwy stacji i piosenki są tekstem na prawdziwym urządzeniu 3D, widocznym także bez otwierania panelu.

Jedna stacja ma cztery utwory dostarczone przez użytkownika, skopiowane bez zmiany zawartości do `public/music/`:

1. I Mine Uraium — `i-mine-uraium.mp3`
2. My Laser Go Brrr — `my-laser-go-brrr.mp3`
3. Ain't Gonna Take My Uranum Again — `aint-gonna-take-my-uranum-again.mp3`
4. This Is Private Property — `this-is-private-property.mp3`

Kolejny utwór zaczyna się automatycznie po zakończeniu poprzedniego. Po ostatnim wraca pierwszy. Startowa głośność wynosi 35%. Podejdź, patrz na radio i naciśnij E. Panel obsługuje mysz oraz lewo/prawo (utwory), Spację (pauza), -/+ (głośność), E/Esc (wyjście). Środkowy przycisk pokazuje jedyną stację 1/1 i bieżącą głośność; wybór tej samej stacji nie restartuje piosenki. Podczas regulacji walka stoi, a muzyka gra dalej. P pauzuje także muzykę, M wycisza dźwięk, R przywraca początkowy przycisk i playlistę.

Odtwarzanie korzysta z Media Foundation w Windows i osobnego strumienia SDL. Dekodowanie odbywa się w wątku roboczym, z niewielkim buforem PCM. Całe utwory nie są ładowane do pamięci. Wizualizacja liczy energię i bas z rzeczywistych próbek PCM w fragmentach po 1024 klatki. Stały bufor 64 pomiarów dopasowuje ruch membran do odtwarzanej części dźwięku. Analizator nie zależy od liczby przeciwników. Napisy, wskaźniki i model korzystają z istniejącego instancjonowania.

`native/music_tests.cpp` sprawdza cztery prawdziwe pliki MP3, metadane, próbki dźwięku, strumieniowanie, zmianę utworów, zmienność wizualizacji, pauzę, wyciszenie i reset. Test używa cichego wirtualnego urządzenia SDL. Jest uruchamiany podczas budowania. `TEST_RADIA.bat` sprawdza przez zdarzenia klawiatury i myszy przycisk, wysunięcie radia, panel i sterowanie. Wyniki i zrzuty: `artifacts/command/radio/`.

Pakowanie do EXE obejmuje wszystkie cztery MP3. Oryginały z folderu Downloads nie są potrzebne do uruchomienia gotowej gry.
