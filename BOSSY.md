# Nowi bossowie

| Boss | Bazowe HP | Zachowanie |
|---|---:|---|
| Żmij segmentowy | 14 000 | Głowa i osiem kulistych segmentów. Marsz do rdzenia 3,2 m/s, niezależny szybki zygzak na boki (do 32 m/s, szerokość około 27 m). Jedna pula HP; segmenty podążają za poprzednimi i można w nie strzelać. |
| Śluzowiec | 8000 | Po śmierci dwa po 4000 HP; każdy z nich dzieli się na dwa po 2000 HP. Łącznie siedem ciał i 24 000 HP. Mniejsze części są szybsze. Jeden kryształ dopiero po pokonaniu całej rodziny. |
| Pożeracz | 12 000 | Osobne, jednorazowe fazy przy 75%, 50%, 25% HP. Każda przechwytuje istniejących przeciwników w promieniu do 50 m. Nie przywołuje nowych ani nie zmienia ich HP. Przez 2 s można ich ostrzelać, potem są przyciągani do paszczy (14 m/s). Inni bossowie nie są wchłaniani. Boss leczy się za ich pozostałe HP, maksymalnie do pełnego życia. Ofiary zabite lub zatrzymane do końca fazy nie leczą. |
| Plujec alfa | 15 000 | Ten sam zasięg co plujec oblężniczy: 63 m. Balistyczny pocisk za 200 obrażeń co 5 s; obrażenia po dolocie. |

Pożeracz zatrzymuje się podczas ośmiosekundowej fazy. Leczenie nie odnawia wyzwalaczy. Jeśli jedno trafienie przekroczy kilka progów, fazy trafiają do kolejki. Śmiertelne trafienie nie uruchamia wchłaniania. Ogłuszenie zatrzymuje jego zegar, a zabicie bossa uwalnia ocalałe ofiary.

Naturalna rotacja co 3 minuty: Tytan → Królowa → Nadumysł → Żmij → Śluzowiec → Pożeracz → Plujec alfa. Nowych bossów można od razu przywołać kliknięciem w bestiariuszu pod apostrofem `'`.

Wydajność: segmenty żmija są instancjami sprite'a i mają wspólny obiekt AI. Dodatkowe testy kolizji dotyczą wyłącznie segmentowych bossów. Pożeracz jednorazowo wybiera obecnych wrogów z istniejącej puli; pociski plujca z ograniczonej puli pocisków wroga. Podział śluzowca respektuje limit puli.

Weryfikacja: `native/boss_tests.cpp`, pełny `native/build.ps1 -GameOnly`, test graficzny `rdzen-game.exe --boss-test` i raport `artifacts/command/bosses/bosses-result.json`.

Grafiki: pięć nowych PNG w `public/assets/boss-{wyrm,wyrm-segment,slime,devourer,siege}.png`. Wygenerowane wbudowanym narzędziem `imagegen`; prompty zapisano w `BOSSY_GRAFIKI.md`.
