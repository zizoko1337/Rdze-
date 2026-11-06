#pragma once
#include "game_scene.hpp"
#include <sstream>
#include <iomanip>

namespace arsenal {
inline std::string number(double value, int decimals = 0) {
  std::ostringstream s;
  s << std::fixed << std::setprecision(decimals) << value;
  return s.str();
}
struct Rect {
  float x, y, w, h;
  bool contains(float px, float py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
  }
};
class GameUi {
public:
  float scale = 1, width = 1600, height = 900;
  static constexpr Color mint{.57f, .92f, .78f}, muted{.48f, .64f, .65f}, white{.86f, .92f, .9f};
  void resize(int w, int h) {
    scale = std::min(w / 1600.f, h / 900.f);
    width = w / scale;
    height = h / scale;
  }
  Rect card(int index) const {
    float cardWidth = (width - 48 - 8 * 8) / 9;
    return {24 + index * (cardWidth + 8), height - 135, cardWidth, 84};
  }
  Rect action(int index) const { return {width - 236, 105 + index * 46.f, 212, 38}; }
  Rect viewButton() const { return {width - 236, height - 42, 212, 30}; }
  int click(float px, float py, bool tactical, bool help) const {
    float x = px / scale, y = py / scale;
    if (help)
      return 200;
    if (viewButton().contains(x, y))
      return 100;
    for (int i = 0; i < 4; ++i)
      if (action(i).contains(x, y))
        return 110 + i;
    if (tactical) {
      for (int i = 0; i < WeaponCount; ++i)
        if (card(i).contains(x, y))
          return i;
    }
    return -1;
  }
  bool covered(float px, float py, bool tactical, bool help) const {
    float x = px / scale, y = py / scale;
    if (help)
      return true;
    if (y < 78 || y > height - (tactical ? 151 : 52))
      return true;
    if (Rect{22, 105, 240, 220}.contains(x, y))
      return true;
    if (Rect{width - 236, 105, 212, 184}.contains(x, y))
      return true;
    if (tactical && Rect{22, height - 355, 326, 180}.contains(x, y))
      return true;
    return false;
  }
  void rect(Renderer& r, Rect box, Color color, float alpha = 1) {
    r.rect(box.x * scale, box.y * scale, box.w * scale, box.h * scale, color.r, color.g, color.b,
           alpha);
  }
  void text(Renderer& r, float x, float y, const std::string& s, float size = 16, Color c = white) {
    r.text(x * scale, y * scale, s, size * scale, c.r, c.g, c.b);
  }
  void panel(Renderer& r, Rect box) {
    rect(r, box, {.18f, .31f, .33f}, .8f);
    rect(r, {box.x + 1, box.y + 1, box.w - 2, box.h - 2}, {.025f, .065f, .08f}, .94f);
  }
  void paragraph(Renderer& r, float x, float y, const std::string& content, int maxLength = 41,
                 float size = 13, Color c = muted) {
    std::istringstream input(content);
    std::string word, line;
    while (input >> word) {
      if (line.size() + word.size() + 1 > size_t(maxLength)) {
        text(r, x, y, line, size, c);
        y += size * 1.5f;
        line.clear();
      }
      if (!line.empty())
        line += ' ';
      line += word;
    }
    if (!line.empty())
      text(r, x, y, line, size, c);
  }
  void draw(Renderer& r, const Game& g, const ViewState& v, float fps, float gpu, bool vsync,
            bool help, bool sound, bool captured) {
    resize(r.width, r.height);
    rect(r, {0, 0, width, 78}, {.022f, .052f, .065f}, .95f);
    rect(r, {0, 77, width, 1}, {.21f, .37f, .36f}, .8f);
    text(r, 26, 12, "RDZEŃ", 31, mint);
    text(r, 159, 20, "/  POLIGON ARSENAŁU", 14);
    text(r, 28, 51, "KEPLER-186F  /  PROTOTYP 02", 10, muted);
    text(r, width * .45f, 19, "FALA " + std::to_string(g.wave), 23);
    text(r, width * .45f, 51,
         g.autoWaves ? "Następna za " + number(std::max(0.f, g.nextWave - g.time)) + " s"
                     : "Fale automatyczne wyłączone",
         11, muted);
    text(r, width - 243, 18, number(fps) + " FPS   GPU " + number(gpu, 2) + " ms", 13, mint);
    text(r, width - 243, 45,
         std::string("VSync ") + (vsync ? "WŁ." : "WYŁ.") + "   [M] Dźwięk " +
             (sound ? "WŁ." : "WYŁ."),
         10, muted);
    panel(r, {22, 105, 240, 220});
    text(r, 38, 118, "PLATFORMA WYDOBYWCZA", 11, mint);
    text(r, 38, 145, "RDZEŃ NIEZNISZCZALNY", 16);
    text(r, 38, 175, "Tryb swobodnego testowania", 11, muted);
    rect(r, {38, 206, 206, 1}, {.2f, .35f, .35f});
    text(r, 38, 219, "NA POLU", 10, muted);
    text(r, 153, 219, "ELIMINACJE", 10, muted);
    text(r, 38, 239, std::to_string(g.active.size()), 25);
    text(r, 153, 239, std::to_string(g.kills), 25);
    text(r, 38, 284, "Wieżyczki: " + std::to_string(g.towers.size()) + " / 512", 12, mint);
    text(r, 38, 305, "Pokład: +18 m", 10, muted);
    const char* buttons[] = {"[N] Następna fala", "[J] Przywołaj bossa", "[H] Horda +250",
                             "[F] Fale: "};
    for (int i = 0; i < 4; ++i) {
      auto b = action(i);
      panel(r, b);
      text(r, b.x + 12, b.y + 7,
           i == 3 ? std::string(buttons[i]) + (g.autoWaves ? "AUTO" : "RĘCZNE") : buttons[i], 13,
           i == 1 ? Color{.94f, .64f, .44f} : white);
    }
    text(r, width - 229, 300, v.tactical ? "WIDOK TAKTYCZNY" : "WIDOK OPERATORA", 12, mint);
    text(r, width - 229, 322, "[F1] Sterowanie", 11, muted);
    int bosses = 0;
    std::string bossName;
    for (int id : g.active)
      if (g.foes[id].hp > 0 && g.foes[id].type >= Titan) {
        ++bosses;
        bossName = SPECIES[g.foes[id].type].name;
      }
    if (bosses) {
      text(r, width * .42f, 100, "BOSS: " + bossName, 16, {1, .6f, .43f});
      if (bosses > 1)
        text(r, width * .42f, 125, "Aktywni bossowie: " + std::to_string(bosses), 12,
             {1, .6f, .43f});
    }
    if (v.tactical) {
      panel(r, {22, height - 355, 326, 180});
      const Tower* hovered =
          v.hovered >= 0 && v.hovered < int(g.towers.size()) ? &g.towers[v.hovered] : nullptr;
      Weapon chosen = (!v.building && hovered) ? hovered->type : v.selected;
      auto& w = WEAPONS[chosen];
      text(r, 38, height - 345, v.building ? "BUDOWANIE / BEZ KOSZTÓW" : "OBSERWACJA", 10, mint);
      text(r, 38, height - 319, w.name, 25, w.color);
      text(r, 38, height - 282, w.role, 10, muted);
      paragraph(r, 38, height - 259, w.description, 43, 12);
      text(r, 38, height - 205, "Zasięg: " + number(w.range) + " m", 12, mint);
      if (hovered && !v.building) {
        std::string status;
        if (hovered->type == Lance)
          status = hovered->state == 0   ? "Ładowanie " + number(hovered->phase, 1) + " / 6 s"
                   : hovered->state == 1 ? "Wiązka aktywna"
                                         : "Chłodzenie";
        else
          status = "Odnowienie " + number(hovered->cooldown, 1) + " s";
        text(r, 177, height - 205, status, 11);
      }
      rect(r, {0, height - 150, width, 150}, {.022f, .052f, .065f}, .96f);
      text(r, 26, height - 147, "ARSENAŁ / 1-9", 10, muted);
      for (int i = 0; i < WeaponCount; ++i) {
        auto c = card(i);
        bool chosenCard = v.selected == i;
        panel(r, c);
        if (chosenCard)
          rect(r, {c.x, c.y, 3, c.h}, WEAPONS[i].color);
        rect(r, {c.x + 12, c.y + 12, 24, 23}, WEAPONS[i].color, .22f);
        text(r, c.x + 18, c.y + 14, std::to_string(i + 1), 12, WEAPONS[i].color);
        text(r, c.x + 46, c.y + 11, WEAPONS[i].name, 17, chosenCard ? white : muted);
        text(r, c.x + 12, c.y + 45, number(WEAPONS[i].range) + " m", 12, WEAPONS[i].color);
        text(r, c.x + 12, c.y + 64,
             i == Lance   ? "6 s ładowania"
             : i == Flame ? "ogień ciągły"
                          : number(WEAPONS[i].cooldown, 2) + " s / strzał",
             9, muted);
      }
      std::string hint =
          v.building
              ? (v.placement ? (g.canBuild(v.placeX, v.placeZ) ? g.canBuild(v.placeX, v.placeZ)
                                                               : "LPM - postaw wieżyczkę")
                             : "Wskaż miejsce na powierzchni")
              : "1-9 wybór broni   |   X usuwa wskazaną wieżyczkę";
      text(r, width * .34f, height - 179, hint, 13,
           v.placement && g.canBuild(v.placeX, v.placeZ) ? Color{1, .45f, .4f} : mint);
    }
    rect(r, {0, height - 44, width, 44}, {.015f, .04f, .055f}, .98f);
    text(r, 26, height - 33,
         v.tactical
             ? "WASD kamera  |  Q/E obrót  |  Kółko zoom  |  PPM/B anuluj  |  X usuń  |  P pauza"
             : "WASD ruch  |  Mysz widok  |  Shift bieg  |  P pauza  |  Esc kursor  |  F1 pomoc",
         12, muted);
    auto b = viewButton();
    panel(r, b);
    text(r, b.x + 12, b.y + 5, v.tactical ? "[TAB] Powrót na pokład" : "[TAB] Budowanie z góry", 12,
         mint);
    if (!v.tactical) {
      rect(r, {width / 2 - 5, height / 2, 10, 1}, white, .6f);
      rect(r, {width / 2, height / 2 - 5, 1, 10}, white, .6f);
      if (!captured && !help) {
        panel(r, {width / 2 - 185, height - 114, 370, 42});
        text(r, width / 2 - 170, height - 104, "Kliknij w teren, aby rozejrzeć się", 13, mint);
      }
    }
    if (g.time < g.announcementUntil && !help) {
      float y = v.tactical ? height - 215 : height - 166;
      panel(r, {width / 2 - 300, y, 600, 34});
      text(r, width / 2 - 286, y + 5, g.announcement, 12, mint);
    }
    if (g.paused && !help) {
      panel(r, {width / 2 - 208, 170, 416, 58});
      text(r, width / 2 - 188, 178, "PAUZA / [P] wznów", 21, mint);
      text(r, width / 2 - 188, 211, "Budowanie i kamera pozostają dostępne", 11, muted);
    }
    if (help) {
      rect(r, {0, 0, width, height}, {.005f, .02f, .035f}, .88f);
      panel(r, {width / 2 - 300, height / 2 - 265, 600, 530});
      text(r, width / 2 - 270, height / 2 - 240, "POLIGON ARSENAŁU", 26, mint);
      text(r, width / 2 - 270, height / 2 - 193, "Eksperymentuj. Budowanie nic nie kosztuje.", 15);
      text(r, width / 2 - 270, height / 2 - 151,
           "TAB       Pokład / widok taktyczny\n1-9       Wybierz wieżyczkę\nLPM       Postaw / "
           "wybierz w menu\nPPM / B   Anuluj budowanie\nX         Usuń wskazaną wieżyczkę\nWASD    "
           "  Ruch / kamera\nQ / E     Obrót kamery taktycznej\nKółko     Przybliżenie widoku z "
           "góry\nN / J / H Fala / boss / dodatkowa horda\nF         Automatyczne fale\nP         "
           "Pauza\nR         Nowy poligon\nM / V     Dźwięk / VSync\nF11       Pełny ekran\nF1 / "
           "Esc  Zamknij pomoc",
           16, white);
    }
    r.flushText();
  }
};
} // namespace arsenal
