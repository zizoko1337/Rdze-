#pragma once
#include "command_scene.hpp"

namespace command {
inline constexpr const char* ENEMY_ROLES[] = {"RÓJ",
                                              "SZYBKI",
                                              "PANCERNY",
                                              "DYSTANSOWY",
                                              "LATAJĄCY",
                                              "ELITA / SABOTAŻ",
                                              "ARTYLERIA",
                                              "BOSS / SZTURM",
                                              "BOSS / PRZYWOŁANIE",
                                              "BOSS / ZAKŁÓCENIA"};
inline constexpr const char* ENEMY_BEHAVIOR[] = {
    "Atakuje rdzeń: 4 obrażenia/s.",
    "Szybki szturm. Rdzeń: 4 obrażenia/s.",
    "Dużo HP. Rdzeń: 4 obrażenia/s.",
    "Kwas: 25 obrażeń co 3 s. Zasięg 55 m.",
    "Leci ponad ziemią. Rdzeń: 4 obrażenia/s.",
    "Poluje na wieżyczki. Niszczy po 3 s; ogłuszenie przerywa atak.",
    "Zatrzymuje się na 63 m. Kwas: 45 obrażeń co 5 s + czas lotu.",
    "Rdzeń: 80 obrażeń/s. Co 6 s uderza za dodatkowe 30 obrażeń.",
    "Rdzeń: 80 obrażeń/s. Co 8 s przywołuje 5 łowców.",
    "Co 6 s wyłącza wieżyczki na 4 s. Promień 42 m (2 heksy)."};
static_assert(std::size(ENEMY_ROLES) == SpeciesCount && std::size(ENEMY_BEHAVIOR) == SpeciesCount);

// No renderer dependency in spawn logic, allowing the actual menu action to be tested headlessly.
struct Bestiary {
  int selected = 0, count = 1;
  uint64_t serial = 0;
  std::string notice;
  int summon(Session& s, Species type) {
    if (s.mode == Mode::Selection || s.expedition.dead())
      return 0;
    int made = 0;
    float facing = s.seated() ? s.gunYaw : s.yaw;
    for (int n = 0; n < count && s.world.active.size() < Frontier::MAX_ACTIVE; ++n) {
      float angle = facing + (int(serial++ % 31) - 15) * .014f;
      float radius = 110 + (serial % 13) * 3;
      int id =
          s.frontier.spawnFoe(s.world, type, std::sin(angle) * radius, std::cos(angle) * radius);
      if (id >= 0) {
        if (type == Brain)
          s.world.foes[id].ability = 2;
        ++made;
      }
    }
    s.world.grid.rebuild(s.world.foes, s.world.active);
    notice = "Przywołano: " + std::string(SPECIES[type].name) + " x " + std::to_string(made) +
             (s.expedition.enabled && !s.expedition.miningStarted
                  ? ". Uruchom wydobycie czerwonym przyciskiem [E], aby bestie ruszyły."
                  : ". Zamknij panel, aby ruszył czas.");
    return made;
  }
  static Rect card(const GameUi& ui, int index) {
    float width = (ui.width - 100) / 5;
    return {50 + index % 5 * width, 153 + index / 5 * 323.f, width - 12, 309};
  }
  static Rect batch(const GameUi& ui, int index) {
    return {ui.width - 420 + index * 120.f, 88, 110, 41};
  }
  void click(GameUi& ui, Session& s, float px, float py) {
    float x = px / ui.scale, y = py / ui.scale;
    constexpr int batches[] = {1, 10, 100};
    for (int i = 0; i < 3; ++i)
      if (batch(ui, i).contains(x, y)) {
        count = batches[i];
        return;
      }
    for (int i = 0; i < SpeciesCount; ++i)
      if (card(ui, i).contains(x, y)) {
        selected = i;
        summon(s, Species(i));
        return;
      }
  }
  void draw(GameUi& ui, Renderer& r, const GameScene& scene, const Session& s) {
    ui.resize(r.width, r.height);
    ui.rect(r, {0, 0, ui.width, ui.height}, {.009f, .018f, .03f}, .98f);
    ui.text(r, 50, 35, "BESTIARIUSZ / SYMULATOR ZAGROŻEŃ", 30, GameUi::mint);
    ui.text(r, 50, 92, "['] / Esc: zamknij  |  Kliknij bestię: przywołaj", 16, GameUi::white);
    ui.text(r, 50, 121, "HP i prędkość dla bieżącego czasu wyprawy. Czas stoi podczas wyboru.", 12,
            GameUi::muted);
    constexpr int batches[] = {1, 10, 100};
    for (int i = 0; i < 3; ++i) {
      auto b = batch(ui, i);
      ui.panel(r, b);
      ui.text(r, b.x + 17, b.y + 14, "x " + std::to_string(batches[i]), 16,
              count == batches[i] ? GameUi::mint : GameUi::muted);
    }
    for (int i = 0; i < SpeciesCount; ++i) {
      auto b = card(ui, i);
      auto type = Species(i);
      Color c = type >= Titan      ? Color{.8f, .4f, 1}
                : type == Saboteur ? Color{1, .65f, .2f}
                                   : GameUi::mint;
      ui.panel(r, b);
      ui.rect(r, {b.x, b.y, b.w, 3}, selected == i ? c : Color{.11f, .19f, .23f});
      ui.text(r, b.x + 12, b.y + 14, ENEMY_ROLES[i], 10, c);
      float imageHeight = 130, imageWidth = imageHeight * scene.aspect[i];
      float maxWidth = b.w - 30;
      if (imageWidth > maxWidth) {
        imageHeight *= maxWidth / imageWidth;
        imageWidth = maxWidth;
      }
      Instance image{(b.x + b.w / 2) * ui.scale,
                     (b.y + 98) * ui.scale,
                     0,
                     imageWidth * ui.scale,
                     imageHeight * ui.scale,
                     1,
                     0,
                     1,
                     1,
                     1,
                     1,
                     0};
      r.draw(r.quad, &image, 1, scene.textures[i], 16);
      ui.text(r, b.x + 12, b.y + 170, SPECIES[i].name, 18, GameUi::white);
      ui.text(r, b.x + 12, b.y + 201,
              "HP " + std::to_string(int(Frontier::spawnHp(type, s.world.time))) + "  |  " +
                  number(Frontier::spawnSpeed(type, s.world.time), 1) + " m/s",
              13, c);
      ui.paragraph(r, b.x + 12, b.y + 227, ENEMY_BEHAVIOR[i], int((b.w - 24) / 7), 12,
                   GameUi::muted);
      ui.text(r, b.x + 12, b.y + 285, "[LPM] PRZYWOŁAJ", 11, c);
    }
    ui.text(r, 50, 824, notice, 14, GameUi::mint);
    ui.text(r, 50, 858,
            "Bestie pojawiają się 110-146 m przed tobą. Liczba aktywnych: " +
                std::to_string(s.world.active.size()) + " / " +
                std::to_string(Frontier::MAX_ACTIVE),
            13, GameUi::muted);
    r.flushText();
  }
};
} // namespace command
