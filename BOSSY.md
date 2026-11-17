# Nowi bossowie

| Boss | Bazowe HP | Zachowanie |
|---|---:|---|
| Żmij segmentowy | 14 000 | Głowa i osiem kulistych segmentów. Marsz do rdzenia 3,2 m/s, niezależny szeroki zygzak na boki (do 72 m/s, nominalna szerokość około 96 m). Jedna pula HP. Segmenty odtwarzają przebytą trasę głowy jak w Snake’u, co 7 m wzdłuż trasy, zamiast skracać zakręty. Można w nie strzelać; zatrzymanie głowy zatrzymuje też całe ciało. |
| Śluzowiec | 8000 | Po śmierci dwa po 4000 HP; każdy z nich dzieli się na dwa po 2000 HP. Łącznie siedem ciał i 24 000 HP. Mniejsze części są szybsze. Jeden kryształ dopiero po pokonaniu całej rodziny. |
| Pożeracz | 12 000 | Osobne, jednorazowe fazy przy 75%, 50%, 25% HP. Każda przechwytuje istniejących przeciwników w promieniu do 50 m. Nie przywołuje nowych ani nie zmienia ich HP. Przez 2 s można ich ostrzelać, potem są przyciągani do paszczy (14 m/s). Inni bossowie nie są wchłaniani. Boss leczy się za ich pozostałe HP, maksymalnie do pełnego życia. Ofiary zabite lub zatrzymane do końca fazy nie leczą. |
| Plujec alfa | 15 000 | Zasięg 63 m. Ciągły rozpylacz kwasu z końcówki trąby: 40 obrażeń/s, czyli 200 w 5 s. Efekt jest przyczepiony do grafiki potwora także przy zmianie kamery. Ogłuszenie, śmierć i wyjście z zasięgu przerywają atak. |
| Tunelarz | 22 000 | Biało-szara głowa z okrągłą zębatą tarczą wiertniczą i 32 segmenty co 7 m (224 m ciała). Przy pojawieniu losuje kierunek okrążania rdzenia: lewo albo prawo. Ruch boczny 42 m/s, do rdzenia 2,6 m/s. Pionowa sinusoida ma amplitudę 18 m i okres 112 m ruchu bocznego. Głowa i segmenty kolejno nurkują i wynurzają się, odtwarzając trasę głowy. Przy 70/40/10% HP jednorazowo zmienia kierunek. |

Tunelarz ma wspólną pulę HP, ale oddzielne trafienia dla głowy i segmentów. Pod ziemią części są niewidoczne i nie można w nie celować ani zadawać im obrażeń bronią, wybuchami, Brimstone lub taranowaniem. Trucizna, jej chmury i nałożone wcześniej zatrucie działają również pod ziemią. Podpalenie przestaje ranić, gdy pod ziemię zejdzie przypalony segment. Wieżyczki i broń naprowadzana wybierają wynurzone części, także gdy głowa jest zakopana lub poza zasięgiem. Jeden wybuch lub przebijający strzał zadaje obrażenia tylko raz, niezależnie od liczby trafionych segmentów. Paraliż i klej zatrzymują cały ruch. Przy wejściu w ziemię widać kurz i ślad drążenia; grafika jest przycinana na poziomie gruntu.

Pożeracz zatrzymuje się podczas ośmiosekundowej fazy. Leczenie nie odnawia wyzwalaczy. Jeśli jedno trafienie przekroczy kilka progów, fazy trafiają do kolejki. Śmiertelne trafienie nie uruchamia wchłaniania. Ogłuszenie zatrzymuje jego zegar, a zabicie bossa uwalnia ocalałe ofiary.

Naturalna rotacja: Tytan → Królowa → Nadumysł → Żmij → Śluzowiec → Pożeracz → Plujec alfa → Tunelarz. Pierwsza seria po 3 min, kolejne odstępy 150/120/90/60/30 s, potem stale 30 s. Mutacje mogą dodawać kolejnych bossów do serii. [Progresja i mutacje](MUTACJE_PRZECIWNIKOW.md). Bossów można od razu przywołać kliknięciem w bestiariuszu pod apostrofem `'`.

Wydajność: segmenty żmija są instancjami sprite'a i mają wspólny obiekt AI. Dodatkowe testy kolizji dotyczą wyłącznie segmentowych bossów. Pożeracz jednorazowo wybiera obecnych wrogów z istniejącej puli; pociski plujca z ograniczonej puli pocisków wroga. Podział śluzowca respektuje limit puli.

Weryfikacja: `native/boss_tests.cpp`, `native/worm_path_tests.cpp`, pełny `native/build.ps1 -GameOnly`, test graficzny `rdzen-game.exe --boss-test` i raport `artifacts/command/bosses/bosses-result.json`.

Tunelarz: `native/tunneler_tests.cpp`, test graficzny `native/build/tunneler-visual-tests.exe` (ukryte okno), kadry i raport w `artifacts/command/tunneler/`. Historia trasy zajmuje stałe 640 próbek na bossa; segmenty nie mają osobnego AI ani własnych wpisów w puli przeciwników.

Grafiki: pięć nowych PNG w `public/assets/boss-{wyrm,wyrm-segment,slime,devourer,siege}.png`. Wygenerowane wbudowanym narzędziem `imagegen`; prompty zapisano w `BOSSY_GRAFIKI.md`.

Grafiki Tunelarza: [pliki i pełne prompty](ASSETS-TUNELARZ.md), wygenerowane wbudowanym `image_gen` z użyciem skilla `imagegen`.
