# Ślady broni manualnych i ziemia

- Laser: małe osmalenie z krótkim żarem, znika po 3,5 s.
- Minigun: drobny, ciemny ślad po rzeczywistym uderzeniu pocisku w ziemię, 2,5 s.
- Snajperka: nieco większy ślad na końcu przebijającego strzału, 4 s.
- Lanca: połączone odcinki rozżarzonej smugi o szerokości około 0,68 m. Żar stygnie przez 1,6 s, osmalenie znika po 4,5 s. Przerwanie kontaktu z ziemią przerywa linię.
- Tesla: krótkie przypalenie w miejscu uziemienia wyładowania, 3 s.
- Palnik: szersze osmalenie przy bezpośrednim kontakcie strumienia z podłożem, 4,5 s.
- Rakiety i moździerz: osmalenie po wybuchach przy ziemi, 6 s; wybuch wysoko w powietrzu nie maluje podłoża. Plazma zostawia drobny ślad przy trafieniu ziemi. Toksyna zachowuje istniejące kałuże.

Efekty są wyłącznie wizualne. Wspólna pula ma maksymalnie 1024 ślady; kolejne zastępują starsze. Ostrzał jednego miejsca odświeża ślad, zamiast nakładać tysiące warstw. Smuga lancy jest próbkowana do 30 razy na sekundę. Czas życia zatrzymuje się wraz z pauzą. Ślady nie korzystają z generatora losowego walki.

Podłoże używa lokalnej tekstury `public/assets/ground-basalt.png`, bazaltowego pyłu i drobnych kamieni. Materiał powtarza się w skali 10 m i jest delikatnie mieszany z większą skalą 47 m. Ma mipmapy i filtrowanie anizotropowe 16×. Ślady są rysowane w dwóch instancjonowanych partiach: ciemne osmalenie i dodawany żar. Geometria świata zasłania je zgodnie z buforem głębokości.

Testy: `native/ground_effect_tests.cpp` sprawdza rzeczywiste trafienia, smugi, brak śladów po strzałach w niebo i trafieniach w przeciwników, wybuchy, wygaszanie, pauzę i limit pamięci. `native/render_tests.cpp` sprawdza stabilność tekstury podczas niewielkich ruchów kamery i ciągłego ostrzału.

Weryfikacja 2026-11-13: testy efektów, broni manualnych, wariantów Legendary lancy oraz plazmy przeszły. Test Direct3D: 720 klatek zmian celowania i rozmiaru okna oraz 1200 klatek powolnego obrotu podczas ostrzału, bez pustych klatek i czarnych błysków. Zmiana RGB podłoża przy przesunięciu kamery o ułamek piksela: 0,0156893/255, poniżej progu 0,1. Po całkowitym wygaśnięciu śladów obraz podłoża odpowiada obrazowi kontrolnemu (różnica 0).

Lokalne podglądy z testu: [żar](artifacts/render/ground-hot.png), [ostygnięcie](artifacts/render/ground-cooled.png), [zniknięcie](artifacts/render/ground-expired.png). Pliki w `artifacts` są pomijane przez Git.
