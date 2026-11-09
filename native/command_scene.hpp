#pragma once
#include "game_ui.hpp"
#include "command_game.hpp"
#include "command_roof.hpp"
#include "expedition_scene.hpp"
#include "radar_scene.hpp"
#include "manual_scene.hpp"
#include "deck_instruments.hpp"

namespace command {
inline void drawConsoleNumber(GameScene& scene, Vec3 center, int value, Color color) {
  constexpr unsigned masks[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
  value = std::clamp(value, 0, 999);
  for (int digit = 0; digit < 3; ++digit) {
    int divisor = digit == 0 ? 100 : digit == 1 ? 10 : 1;
    unsigned mask = masks[(value / divisor) % 10];
    float x = (digit - 1) * .22f;
    constexpr Vec3 positions[] = {{0, .14f, 0},  {.075f, .07f, 0},   {.075f, -.07f, 0},
                                  {0, -.14f, 0}, {-.075f, -.07f, 0}, {-.075f, .07f, 0},
                                  {0, 0, 0}};
    for (int segment = 0; segment < 7; ++segment)
      if (mask & (1u << segment)) {
        bool horizontal = segment == 0 || segment == 3 || segment == 6;
        scene.box(center + Vec3{x, 0, 0} + positions[segment],
                  horizontal ? Vec3{.14f, .025f, .015f} : Vec3{.025f, .115f, .015f}, color, 0, 0,
                  true);
      }
  }
}
inline void buildStartButton(GameScene& scene, const Session& s) {
  if (!s.expedition.enabled || s.mode == Mode::Selection)
    return;
  Vec3 base{Session::START_BUTTON.x, CORE_HEIGHT + .28f, Session::START_BUTTON.z};
  auto ease = [](float t) {
    t = std::clamp(t, 0.f, 1.f);
    return t * t * (3 - 2 * t);
  };
  float age = s.expedition.miningStarted ? s.buttonAge : 0;
  float opening = s.expedition.miningStarted ? ease((age - .15f) / .4f) : 0;
  scene.box(base, {2.05f, .08f, 2.05f}, {.04f, .07f, .08f});
  for (float sign : {-1.f, 1.f})
    scene.box(base + Vec3{sign * (.49f + opening * 1.05f), .07f, 0}, {.96f, .06f, 1.92f},
              {.24f, .32f, .35f});
  if (!s.expedition.miningStarted || age < 1.7f) {
    float sink = s.expedition.miningStarted ? ease((age - .6f) / 1.05f) * 2.2f : 0;
    Vec3 p = base + Vec3{0, -sink, 0};
    scene.box(p + Vec3{0, .08f, 0}, {.8f, .14f, .8f}, {.12f, .18f, .2f});
    scene.box(p + Vec3{0, .53f, 0}, {.46f, .85f, .46f}, {.45f, .52f, .55f});
    scene.box(p + Vec3{0, 1.0f, 0}, {.95f, .18f, .8f}, {.14f, .2f, .23f});
    float press = s.expedition.miningStarted ? ease(age / .2f) * .12f : 0;
    scene.tube(p + Vec3{0, 1.14f - press, 0}, {.32f, .18f, .32f}, {1, .025f, .02f}, 0, 0, true);
    for (float side : {-1.f, 1.f})
      scene.box(p + Vec3{side * .43f, 1.12f, 0}, {.06f, .08f, .72f}, {1, .7f, .16f});
  }
}
inline void buildCommandDeck(GameScene& scene, const Session& s) {
  Color steel{.57f, .64f, .67f}, dark{.12f, .18f, .21f}, mint{.25f, .8f, .7f};
  const float roof = Session::ROOF;
  buildExpedition(scene, s);
  appendRadarTelevision(scene, s);
  buildStartButton(scene, s);
  appendWarningLamps(scene, s);
  // Roof shell and circular hatch are immutable geometry in CommandRoof.
  for (float side : {-1.f, 1.f}) {
    scene.box({side * s.hatch * 4.1f, roof + .07f, 0}, {.06f, .035f, 7.2f}, mint, 0, 0, true);
  }
  for (int i = 0; i < 8; ++i) {
    float a = i * PI / 4, x = std::sin(a), z = std::cos(a);
    float column = Session::roofHeight(11.8f) - CORE_HEIGHT - .14f;
    scene.box({x * 11.8f, CORE_HEIGHT + column / 2, z * 11.8f}, {.4f, column, .4f}, steel, a);
    scene.box({x * 11.56f, CORE_HEIGHT + column / 2, z * 11.56f}, {.08f, column * .6f, .08f}, mint,
              a, 0, true);
    scene.rod({x * 4, Session::roofHeight(4) - .19f, z * 4},
              {x * 13.9f, Session::roofHeight(13.9f) - .19f, z * 13.9f}, .035f, mint, true);
  }
  for (int i = 0; i < 64; ++i) {
    float a = i * 2 * PI / 64, next = (i + 1) * 2 * PI / 64;
    Vec3 p{std::sin(a) * Session::RAIL_RADIUS, CORE_HEIGHT + 1, std::cos(a) * Session::RAIL_RADIUS};
    Vec3 q{std::sin(next) * Session::RAIL_RADIUS, CORE_HEIGHT + 1,
           std::cos(next) * Session::RAIL_RADIUS};
    scene.rod(p, q, .045f, steel);
    if (i % 4 == 0)
      scene.rod({p.x, CORE_HEIGHT + .2f, p.z}, p, .055f, steel);
    scene.rod({std::sin(a) * Session::ROOF_RADIUS, Session::EAVE - .1f,
               std::cos(a) * Session::ROOF_RADIUS},
              {std::sin(next) * Session::ROOF_RADIUS, Session::EAVE - .1f,
               std::cos(next) * Session::ROOF_RADIUS},
              .045f, dark);
  }
  appendCommandConsoles(scene, s);
  appendUraniumPump(scene, s);
  scene.tube({0, CORE_HEIGHT + .18f, 0}, {2.85f, .25f, 2.85f}, dark);
  for (int i = 0; i < 12; ++i) {
    float a = i * PI / 6;
    scene.box({std::sin(a) * 2.7f, CORE_HEIGHT + .32f, std::cos(a) * 2.7f}, {.25f, .03f, .4f},
              {1, .7f, .18f}, a, 0, true);
  }
  // Telescopic mast and guide pistons remain attached to the deck during elevation.
  scene.tube({0, CORE_HEIGHT + (s.lift * Session::LIFT) / 2, 0},
             {.65f, .4f + s.lift * Session::LIFT, .65f}, steel);
  for (float side : {-1.f, 1.f})
    scene.rod({side * 1.9f, CORE_HEIGHT + .2f, 0}, {side * 1.9f, s.baseHeight(), 0}, .12f, steel);
  scene.tube({0, s.baseHeight(), 0}, {2.6f, .3f, 2.6f}, dark);
  scene.tube({0, s.baseHeight() + .16f, 0}, {2.5f, .035f, 2.5f}, {.31f, .38f, .41f});
  auto seatPoint = [&](Vec3 p) {
    return Vec3{p.x * std::cos(s.gunYaw) + p.z * std::sin(s.gunYaw), s.baseHeight() + p.y,
                -p.x * std::sin(s.gunYaw) + p.z * std::cos(s.gunYaw)};
  };
  auto seatBox = [&](Vec3 p, Vec3 size, Color c) { scene.box(seatPoint(p), size, c, s.gunYaw); };
  seatBox({0, .35f, -1.25f}, {.32f, .7f, .32f}, steel);
  seatBox({0, .7f, -1.2f}, {1.05f, .22f, .95f}, {.13f, .2f, .23f});
  if (!s.seated()) {
    seatBox({0, 1.3f, -1.72f}, {1.1f, 1.35f, .23f}, dark);
    seatBox({0, 1.35f, -1.57f}, {.8f, 1, .1f}, {.32f, .4f, .42f});
    seatBox({0, 2.05f, -1.69f}, {.65f, .34f, .3f}, dark);
  }
  for (float side : {-1.f, 1.f}) {
    seatBox({side * .62f, .99f, -1.15f}, {.2f, .15f, .85f}, steel);
    seatBox({side * .45f, .3f, -.25f}, {.38f, .12f, .65f}, steel);
    seatBox({side * .62f, .65f, .35f}, {.16f, 1.2f, .65f}, steel);
  }
  const Color accent = s.lanceReserveActive ? Color{.25f, 1, .18f} : s.module.spec().color;
  auto gunBox = [&](Vec3 p, Vec3 size, Color c, bool glow = false) {
    p.z -= s.recoil * .16f;
    scene.box(s.gunPoint(p), size, c, s.gunYaw, -s.gunPitch, glow);
  };
  auto barrel = [&](Vec3 a, Vec3 b, float radius, Color c, bool glow = false) {
    a.z -= s.recoil * .16f;
    b.z -= s.recoil * .16f;
    scene.rod(s.gunPoint(a), s.gunPoint(b), radius, c, glow);
  };
  gunBox({0, 0, .4f}, {1.1f, .65f, 1.4f}, steel);
  gunBox({0, -.4f, .3f}, {.65f, .25f, 1.2f}, dark);
  for (float side : {-1.f, 1.f}) {
    barrel({side * .5f, -.1f, -.55f}, {side * .5f, .3f, -.55f}, .07f, dark);
    gunBox({side * .57f, .1f, .25f}, {.04f, .13f, .8f}, accent, true);
  }
  if (s.module.kind == Gun::Pulse && !s.module.plasma) {
    barrel({0, 0, 1}, {0, 0, 3.1f}, .19f, dark);
    barrel({0, 0, 3.08f}, {0, 0, 3.2f}, .24f, accent, true);
    for (int i = 0; i < 5; ++i) {
      float z = 1.15f + i * .32f;
      gunBox({0, 0, z}, {.68f, .65f, .1f}, steel);
      gunBox({0, .34f, z}, {.3f, .04f, .1f}, accent, true);
    }
    float ready = 1 - std::clamp(s.cooldown, 0.f, 1.f);
    gunBox({0, .34f, .15f}, {.7f * std::max(.05f, ready), .03f, .22f}, accent, true);
  } else if (s.module.kind == Gun::Crank) {
    float heat = std::clamp((s.barrelHeat - .12f) / .5f, 0.f, 1.f);
    Color hot{steel.r * (1 - heat) + heat, steel.g * (1 - heat) + .1f * heat,
              steel.b * (1 - heat) + .025f * heat};
    for (int i = 0; i < 6; ++i) {
      float a = s.spin + i * PI / 3, x = std::sin(a) * .26f, y = std::cos(a) * .26f;
      barrel({x, y, .95f}, {x, y, 2.55f}, .085f, steel);
      barrel({x, y, 2.55f}, {x, y, 3.2f}, .085f, hot, heat > .25f);
      barrel({x, y, 3.2f}, {x, y, 3.23f}, .09f, heat > .15f ? hot : dark, heat > .25f);
    }
    barrel({0, 0, 1.2f}, {0, 0, 1.35f}, .4f, dark);
    barrel({0, 0, 2.75f}, {0, 0, 2.85f}, .39f, dark);
    gunBox({-.9f, -.15f, .3f}, {.65f, .8f, 1.1f}, dark);
    // The hand crank is a real off-axis rotating lever, separate from the barrel drum.
    Vec3 axle = s.module.extremeCooling ? Vec3{.55f, .59f, .6f} : Vec3{.8f, -.05f, 1.05f};
    Vec3 end = s.module.extremeCooling
                   ? axle + Vec3{std::cos(s.spin) * .18f, .08f, std::sin(s.spin) * .18f}
                   : Vec3{.95f, -.05f + std::cos(s.spin) * .4f, 1.05f + std::sin(s.spin) * .4f};
    barrel(s.module.extremeCooling ? Vec3{.55f, .32f, .6f} : Vec3{.55f, -.05f, 1.05f}, axle, .07f,
           steel);
    barrel(axle, end, .065f, accent);
    barrel(end, end + (s.module.extremeCooling ? Vec3{0, .15f, 0} : Vec3{.3f, 0, 0}), .075f, dark);
    if (s.module.extremeCooling) {
      Color frost{.4f, .8f, 1};
      for (float side : {-1.f, 1.f}) {
        gunBox({side * 1.06f, -.2f, .65f}, {.4f, .75f, 1.25f}, dark);
        gunBox({side * 1.08f, .21f, .65f}, {.25f, .06f, .9f}, frost, true);
        barrel({side * 1.06f, 0, 1.25f}, {side * .62f, .18f, 2.72f}, .055f, steel);
        barrel({side * .62f, .18f, 2.72f}, {side * .62f, .18f, 3.03f}, .065f, frost, true);
        barrel({side * .62f, .18f, 3.03f}, {side * .44f, .1f, 3.13f}, .065f, steel);
        for (int i = 0; i < 4; ++i)
          gunBox({side * 1.29f, -.2f + i * .15f, .65f}, {.045f, .04f, .9f}, steel);
      }
    }
  } else
    buildExtraManualGun(scene, s);
  if (s.shotFlash > 0) {
    if (s.module.kind == Gun::Pulse && s.module.plasma) {
      for (int i = 0; i < 20; ++i)
        if (PlasmaSpec::visibleEmitter(i))
          scene.hot.push_back(
              instance(s.plasmaMuzzle(i), {.25f, .25f, 1}, accent, 0, 0, s.shotFlash * 7));
    } else
      scene.hot.push_back(instance(s.muzzle(), {.7f, .7f, 1}, accent, 0, 0, s.shotFlash * 7));
  }
  for (const auto& b : s.bullets)
    scene.beams.push_back(lineInstance(b.pos - normalize(b.velocity) * 1.8f, b.pos, accent, .055f));
  appendManualEffects(scene, s);
}

class CommandUi : public GameUi {
  std::vector<Instance> radarMarks;

public:
  float radarHeading = 0;
  static constexpr float COLLECTOR_Y = 440;
  bool tacticalCovered(float px, float py) const {
    float x = px / scale, y = py / scale;
    return y < 80 || y > height - 180 || Rect{26, 94, 310, 360}.contains(x, y) ||
           Rect{width - 354, 94, 328, 113}.contains(x, y) ||
           Rect{width - 238, 220, 212, 204}.contains(x, y) ||
           Rect{width - 354, COLLECTOR_Y, 328, 145}.contains(x, y) ||
           Rect{26, 454, 310, 102}.contains(x, y);
  }
  Rect collectorButton() const { return {width - 336, COLLECTOR_Y + 101, 292, 29}; }
  Rect rewardCard(int i) const {
    float w = std::min(370.f, (width - 120) / 3 - 20);
    return {width / 2 - (3 * w + 40) / 2 + i * (w + 20), height / 2 - 65, w, 285};
  }
  Rect restartButton() const { return {width / 2 - 190, height / 2 + 135, 380, 56}; }
  Rect pauseResumeButton() const { return {width / 2 - 200, height / 2 - 75, 400, 56}; }
  Rect pauseMenuButton() const { return {width / 2 - 200, height / 2, 400, 56}; }
  int rewardClick(float px, float py) const {
    for (int i = 0; i < 3; ++i)
      if (rewardCard(i).contains(px / scale, py / scale))
        return i;
    return -1;
  }
  void drawRadar(Renderer& r, const Session& s) {
    float x = width - 238, y = 220, cx = x + 106, cy = y + 110;
    panel(r, {x, y, 212, 204});
    text(r, x + 12, y + 12, "RADAR / 350 M", 11, mint);
    text(r, x + 12, y + 185, "GÓRA = KIERUNEK PATRZENIA", 8, muted);
    for (int line = -2; line <= 2; ++line) {
      rect(r, {cx - 64, cy + line * 32, 128, 1}, {.12f, .24f, .27f});
      rect(r, {cx + line * 32, cy - 64, 1, 128}, {.12f, .24f, .27f});
    }
    radarMarks.clear();
    auto blip = [&](float px, float pz, float size, Color color) {
      float distance = std::max(350.f, std::hypot(px, pz));
      auto local = radarLocal(px, pz, radarHeading);
      float bx = cx + local.x / distance * 64, by = cy - local.z / distance * 64;
      radarMarks.push_back({bx * scale, by * scale, 0, size * scale, size * scale, 1, 0, color.r,
                            color.g, color.b, 1, 0});
    };
    for (const auto& t : s.world.towers)
      blip(t.x, t.z, 3, {.25f, .85f, .5f});
    for (int id : s.world.active) {
      const auto& foe = s.world.foes[id];
      if (foe.hp > 0)
        blip(foe.x, foe.z, foe.type >= Titan ? 7 : 3,
             foe.type >= Titan ? Color{1, .65f, .2f} : Color{1, .25f, .3f});
    }
    for (int i = 1; i <= 10; ++i)
      blip(std::sin(radarHeading) * i * 10, std::cos(radarHeading) * i * 10, 2, {.4f, .9f, 1});
    for (const auto& rover : s.frontier.rovers)
      blip(rover.position.x, rover.position.z, 2, {.3f, 1, .9f});
    for (const auto& c : s.frontier.crystals)
      if (c.state != CrystalState::Carried)
        blip(c.position.x, c.position.z, 5, c.boss ? Color{1, .7f, .2f} : Color{.6f, .5f, 1});
    blip(0, 0, 7, mint);
    // All blips share one instanced draw, including during large waves.
    r.draw(r.quad, radarMarks, r.white, 5);
  }
  void drawTerminal(Renderer& r, const Session& s) {
    const auto& e = s.expedition;
    rect(r, {0, 0, width, height}, {.01f, .035f, .045f}, .97f);
    text(r, 64, 55, "HELIOS / TERMINAL WYDOBYWCZY", 28, mint);
    text(r, 65, 104, "ROK 2137  |  STACJA URANOWA  |  SYMULACJA WSTRZYMANA", 12, muted);
    text(r, 65, 153,
         "Uran: " + number(e.uranium, 1) + " / 100   |   Wydobycie: " + number(e.rate(), 1) +
             " / s",
         17);
    text(r, 65, 185,
         "Gotowe ładunki: " + std::to_string(e.earned) +
             "   |   Oczekujące wypłaty: " + std::to_string(e.rewards.size()),
         15, mint);
    text(r, 65, 216,
         "Typy bojowe: " + std::to_string(e.chosenTypes(false)) +
             " / 4   |   Wsparcie: " + std::to_string(e.chosenTypes(true)) + " / 2",
         14, muted);
    if (e.rewards.empty()) {
      text(r, 65, height / 2 - 30, "BRAK OCZEKUJĄCEJ WYPŁATY", 27);
      text(r, 65, height / 2 + 22,
           "Każde 100 jednostek uranu natychmiast daje wybór 1 z 3 ulepszeń.", 16, muted);
      text(r, 65, height / 2 + 58, e.flightStatus(), 16, mint);
    } else {
      auto source = e.rewards.front().source;
      std::string label = source == RewardSource::BossCrystal    ? "KRYSZTAŁ BOSSA / NAGRODA #"
                          : source == RewardSource::EliteCrystal ? "KRYSZTAŁ ELITY / NAGRODA #"
                                                                 : "WYPŁATA #";
      text(r, 65, height / 2 - 147, label + std::to_string(e.rewards.front().id), 17, mint);
      text(r, 65, height / 2 - 112, "WYBIERZ JEDNO ULEPSZENIE", 25);
      for (int i = 0; i < 3; ++i) {
        auto card = rewardCard(i);
        auto kind = e.rewards.front().choices[i];
        const auto& info = UPGRADES[size_t(kind)];
        auto rarity = e.rewards.front().rarities[i];
        const auto& tier = RARITIES[int(rarity)];
        panel(r, card);
        rect(r, {card.x, card.y, card.w, 4}, tier.color);
        text(r, card.x + 22, card.y + 19, "[" + std::to_string(i + 1) + "]  " + tier.name, 14,
             tier.color);
        text(r, card.x + 22, card.y + 48, info.category, 11, info.color);
        text(r, card.x + 22, card.y + 80, info.name, std::strlen(info.name) > 24 ? 18 : 22);
        paragraph(r, card.x + 22, card.y + 122, upgradeDescription(kind, rarity),
                  kind == Upgrade::PlasmaMortar ? 43 : 37, kind == Upgrade::PlasmaMortar ? 12 : 14,
                  muted);
        text(r, card.x + 22, card.y + 252, "ZAINSTALUJ / " + std::to_string(i + 1), 14, tier.color);
      }
    }
    text(r, 65, height - 128,
         "Manual: " + number(s.module.spec().damage * s.module.damageMultiplier + s.flatDamage, 1) +
             " DMG / " + number(s.module.rateMultiplier / s.module.interval(), 1) +
             " strz./s   |   Szczęście: +" + std::to_string(e.luck) + "%",
         14, mint);
    text(r, 65, height - 101,
         "Wieżyczki: obrażenia +" + number((s.world.towerDamage - 1) * 100, 1) + "% / tempo +" +
             number((s.world.towerRate - 1) * 100, 1) + "% / zasięg +" +
             number((s.world.towerRange - 1) * 100, 1) + "%",
         14, muted);
    text(r, 65, height - 65, "1 / 2 / 3 lub kliknięcie — wybierz   |   E / Esc — wróć do obrony",
         15, muted);
  }
  void drawServices(Renderer& r, const Session& s) {
    rect(r, {0, 0, width, height}, {.01f, .025f, .05f}, .98f);
    text(r, 65, 55, "HELIOS / JEDNORAZOWE USŁUGI", 28, {.3f, .65f, 1});
    text(r, 65, 110, "Punkty usług: " + std::to_string(s.expedition.servicePoints), 24, mint);
    text(r, 65, 157, "Niebieska karta ulepszenia daje 1 punkt. Każda usługa kosztuje 1 punkt.", 15,
         muted);
    text(r, 65, 188,
         "Anulowanie przed wskazaniem celu zachowuje punkt. Wybór miejsca zatrzymuje czas.", 14,
         muted);
    if (!s.expedition.miningStarted)
      text(r, 65, 240, "Najpierw uruchom wydobycie czerwonym przyciskiem za działem.", 17,
           {1, .5f, .3f});
    for (int i = 0; i < int(Service::Count); ++i) {
      auto card = rewardCard(i);
      panel(r, card);
      const auto& info = SERVICES[i];
      rect(r, {card.x, card.y, card.w, 4}, info.color);
      text(r, card.x + 22, card.y + 22, "[" + std::to_string(i + 1) + "] KOSZT: 1 PUNKT", 14,
           info.color);
      text(r, card.x + 22, card.y + 75, info.name, 23);
      paragraph(r, card.x + 22, card.y + 122, info.description, 36, 14, muted);
      text(r, card.x + 22, card.y + 252,
           s.expedition.servicePoints > 0 ? (i == 1 ? "ZAMÓW FALĘ" : "WYBIERZ CEL")
                                          : "BRAK PUNKTÓW",
           14, info.color);
    }
    text(r, 65, height - 65, "1 / 2 / 3 lub kliknięcie — wybierz   |   E / Esc — wróć do obrony",
         15, muted);
  }
  Rect choice(int i) const { return {60 + (i % 3) * 345.f, 250 + (i / 3) * 145.f, 330, 130}; }
  Rect launch() const { return {60, 735, 1020, 58}; }
  int selectionClick(float px, float py) const {
    float x = px / scale, y = py / scale;
    for (int i = 0; i < GUN_COUNT; ++i)
      if (choice(i).contains(x, y))
        return i;
    return launch().contains(x, y) ? GUN_COUNT : -1;
  }
  void drawCommand(Renderer& r, const Session& s, float fps, bool captured, bool help, bool sound,
                   int hoveredSlot = -1, uint64_t hoveredCrystal = 0) {
    resize(r.width, r.height);
    const auto accent = s.lanceReserveActive ? Color{.25f, 1, .18f} : s.module.spec().color;
    if (s.mode == Mode::Selection) {
      rect(r, {0, 0, 1130, height}, {.012f, .035f, .045f}, .95f);
      text(r, 70, 72, "RDZEŃ", 46, mint);
      text(r, 72, 134,
           s.expedition.enabled ? "ROK 2137 / OBRONA PLATFORMY URANOWEJ"
                                : "CENTRUM DOWODZENIA / PRÓBA UZBROJENIA",
           13, muted);
      text(r, 70, 190, "Wybierz działo pokładowe / wszystkie dostępne", 25);
      for (int i = 0; i < GUN_COUNT; ++i) {
        auto b = choice(i);
        panel(r, b);
        if (int(s.module.kind) == i)
          rect(r, {b.x, b.y, 4, b.h}, GUNS[i].color);
        text(r, b.x + 16, b.y + 16, "[" + std::to_string(i + 1) + "] " + GUNS[i].name, 17,
             GUNS[i].color);
        paragraph(r, b.x + 16, b.y + 47, GUNS[i].detail, 39, 13, muted);
        text(r, b.x + 16, b.y + 106, "ZASIĘG " + number(GUNS[i].range) + " m", 11, GUNS[i].color);
      }
      text(r, 72, 705,
           s.expedition.enabled ? "Wydobywaj uran. Odbieraj wypłaty. Rozwijaj obronę."
                                : "Spokojny napływ bestii. Bez zakupów i budowania.",
           13, muted);
      rect(r, launch(), accent, .85f);
      text(r, 100, 750,
           s.expedition.enabled ? "[ENTER]  ROZPOCZNIJ EKSPEDYCJĘ" : "[ENTER]  ROZPOCZNIJ PRÓBĘ",
           21, {.02f, .06f, .08f});
      text(r, 72, height - 58, "1-9 wybór   |   Enter start   |   Esc wyjście", 13, muted);
      r.flushText();
      return;
    }
    if (s.terminalOpen) {
      drawTerminal(r, s);
      r.flushText();
      return;
    }
    if (s.servicesOpen) {
      drawServices(r, s);
      r.flushText();
      return;
    }
    if (s.expedition.dead()) {
      rect(r, {0, 0, width, height}, {.035f, .012f, .02f}, .95f);
      text(r, width / 2 - 260, height / 2 - 140, "RDZEŃ ZNISZCZONY", 39, {1, .35f, .3f});
      text(r, width / 2 - 260, height / 2 - 66, "Czas obrony: " + number(s.world.time) + " s", 20);
      text(r, width / 2 - 260, height / 2 - 22,
           "Eliminacje: " + std::to_string(s.world.kills) +
               "   Ładunki: " + std::to_string(s.expedition.shipments),
           18, muted);
      text(r, width / 2 - 260, height / 2 + 21,
           "Zainstalowane ulepszenia: " + std::to_string(s.expedition.selected), 18, muted);
      text(r, width / 2 - 260, height / 2 + 62,
           "Bossowie: " + std::to_string(s.world.bossKills) +
               "   Kryształy: " + std::to_string(s.frontier.collected) +
               "   Trafienia jeździków: " + std::to_string(s.frontier.roverHits),
           15, muted);
      panel(r, restartButton());
      text(r, restartButton().x + 25, restartButton().y + 15, "[R] NOWA EKSPEDYCJA", 22, mint);
      r.flushText();
      return;
    }
    rect(r, {0, 0, width, 68}, {.015f, .04f, .052f}, .93f);
    text(r, 26, 14, "RDZEŃ / CENTRUM DOWODZENIA", 22, mint);
    text(r, width - 310, 15,
         "BESTIE " + std::to_string(s.world.active.size()) + "   ELIMINACJE " +
             std::to_string(s.world.kills),
         14);
    text(r, width - 310, 42, number(fps) + " FPS   Dźwięk " + (sound ? "WŁ." : "WYŁ."), 10, muted);
    if (s.expedition.enabled) {
      text(r, width * .43f, 14, Frontier::stageName(s.world.time), 14, mint);
      text(r, width * .43f, 42,
           s.world.time < s.frontier.respiteUntil
               ? "ODDECH PO BOSSIE"
               : "Boss za " + number(std::max(0.f, s.frontier.nextBoss - s.world.time)) + " s",
           11, muted);
    }
    panel(r, {26, 94, 310, s.expedition.enabled ? 214.f : 76.f});
    text(r, 42, 108, s.module.name(), 19, accent);
    if (s.expedition.enabled) {
      const auto& e = s.expedition;
      Color integrity = s.coreWarningLevel() == 0   ? mint
                        : s.coreWarningLevel() == 1 ? Color{1, .75f, .1f}
                                                    : Color{1, .2f, .15f};
      text(r, 42, 147, "RDZEŃ  " + number(e.hp) + " / " + number(e.maxHp) + " HP", 16, integrity);
      rect(r, {42, 177, 275, 7}, {.15f, .23f, .25f});
      rect(r, {42, 177, 275 * e.hp / e.maxHp, 7}, integrity);
      text(r, 42, 186, "REGENERACJA  +" + number(e.regeneration, 1) + " HP/s", 9, mint);
      text(r, 42, 200, "URAN  " + number(e.uranium, 1) + " / 100", 16);
      rect(r, {42, 230, 275, 5}, {.15f, .23f, .25f});
      rect(r, {42, 230, 275 * std::min(1.f, e.uranium / 100), 5}, {.45f, .9f, .3f});
      text(r, 42, 252,
           "Wydobycie: " + number(e.rate(), 1) + "/s   Ulepszenia: " + std::to_string(e.selected),
           12, muted);
      text(r, 42, 279, e.shipping ? "TRANSPORT W TOKU" : "MAGAZYN URANU", 11, mint);
      panel(r, {width - 354, 94, 328, 113});
      text(r, width - 336, 108, "LOGISTYKA / WYPŁATY", 12, mint);
      text(r, width - 336, 138, "Oczekujące: " + std::to_string(e.rewards.size()), 19,
           e.rewards.empty() ? muted : Color{1, .7f, .2f});
      text(r, width - 336, 177,
           !e.miningStarted ? "Czerwony przycisk: uruchom wydobycie"
           : e.shipping     ? "Odbiór za " + number(std::max(0.f, 10 - e.flightAge), 1) + " s"
                            : "Transport po zebraniu 100 uranu",
           12, muted);
      drawRadar(r, s);
      float y = COLLECTOR_Y;
      panel(r, {width - 354, y, 328, 145});
      text(r, width - 336, y + 14, "KRYSZTAŁY / DRON ZBIERAJĄCY", 12, mint);
      const char* droneStatus =
          s.frontier.collectorState == CollectorState::Idle         ? "GOTOWY"
          : s.frontier.collectorState == CollectorState::Outbound   ? "LOT PO KRYSZTAŁ"
          : s.frontier.collectorState == CollectorState::Collecting ? "ZBIERANIE KRYSZTAŁU"
                                                                    : "POWRÓT Z ŁADUNKIEM";
      text(r, width - 336, y + 42, droneStatus, 15, mint);
      text(r, width - 336, y + 73,
           "W terenie: " + std::to_string(s.frontier.crystals.size()) +
               "   Zebrane: " + std::to_string(s.frontier.collected),
           12, muted);
      panel(r, collectorButton());
      text(r, width - 324, y + 106, "[C] WYŚLIJ PO NAJBLIŻSZY", 13,
           s.frontier.nearestCrystal() ? mint : muted);
      if (e.levels[size_t(Upgrade::RoverFactory)] > 0) {
        panel(r, {26, 454, 310, 102});
        text(r, 42, 466, "FABRYKA JEŹDZIKÓW", 15, mint);
        text(r, 42, 497,
             std::to_string(e.roverBatch) + " szt. co " + number(e.roverInterval(), 1) +
                 " s / 100 DMG",
             13);
        text(r, 42, 527,
             "W polu: " + std::to_string(s.frontier.rovers.size()) +
                 " / trafienia: " + std::to_string(s.frontier.roverHits),
             12, muted);
      }
      std::string modules = "USŁUGI: " + std::to_string(e.servicePoints) + "  ";
      if (s.world.frost)
        modules += "KRIO  ";
      if (e.levels[size_t(Upgrade::Focus)] > 0)
        modules += "WSPÓLNY CEL  ";
      if (e.levels[size_t(Upgrade::Purge)] > 0)
        modules += e.purgeUsed ? "OCZYSZCZENIE: ZUŻYTE" : "OCZYSZCZENIE: GOTOWE";
      if (!modules.empty())
        text(r, 26, height - 66, modules, 11, mint);
    } else
      text(r, 42, 141, "RDZEŃ NIEZNISZCZALNY / TRYB PRÓBY", 10, muted);
    if (s.expedition.enabled && (s.tactical || s.sentryStock > 0)) {
      panel(r, {26, 320, 310, 120});
      text(r, 42, 333, "WIEŻYCZKI W MAGAZYNIE: " + std::to_string(s.sentryStock), 15, mint);
      text(r, 42, 365,
           "Heksy: " + std::to_string(s.committedSlots()) + " / " + std::to_string(SLOT_COUNT), 13,
           muted);
      text(r, 42, 390,
           s.sentryStock > 0 ? std::string("Następna: ") + WEAPONS[s.pendingSentry()].name
                             : "[TAB] Obserwacja z góry",
           12, mint);
      text(r, 42, 416,
           "Typy: " + std::to_string(s.expedition.chosenTypes(false)) + "/4 bojowe  " +
               std::to_string(s.expedition.chosenTypes(true)) + "/2 wsparcia",
           11, muted);
    }
    if (s.tactical) {
      panel(r, {width / 2 - 315, height - 150, 630, 88});
      std::string instruction =
          s.serviceTarget != Service::None
              ? (s.serviceTarget == Service::Overload ? "[LPM] Wybierz wieżyczkę / PPM anuluj"
                                                      : "[LPM] Cel Brimstone / PPM anuluj")
          : hoveredCrystal     ? "[LPM] Wyślij drona po wskazany kryształ"
          : s.sentryStock == 0 ? "Brak wieżyczek w magazynie. Tab — wróć."
          : hoveredSlot < 0
              ? "Wskaż jeden z " + std::to_string(SLOT_COUNT) + " heksów wokół rdzenia."
          : !s.slotAvailable(hoveredSlot)
              ? "Ten heks jest zajęty lub zarezerwowany."
              : std::string("[LPM] Dostarcz: ") + WEAPONS[s.pendingSentry()].name;
      text(r, width / 2 - 291, height - 135, instruction, 18, mint);
      text(r, width / 2 - 291, height - 99,
           s.sentryStock > 0 ? "6 s do lądowania / balon odczepi się i odleci"
                             : "Zajęte i zarezerwowane heksy nie przyjmują nowych zrzutów.",
           13, muted);
    } else if (s.mode == Mode::Walking) {
      if (s.nearStartButton()) {
        panel(r, {width / 2 - 250, height * .64f, 500, 70});
        text(r, width / 2 - 226, height * .64f + 12, "[E] URUCHOM WYDOBYCIE", 25, {1, .3f, .15f});
        text(r, width / 2 - 226, height * .64f + 48, "Naciśnij czerwony przycisk", 13, muted);
      } else if (s.nearRadio()) {
        panel(r, {width / 2 - 245, height * .64f, 490, 70});
        text(r, width / 2 - 220, height * .64f + 10, "[E] URANIUM FM / RADIO", 25, mint);
        text(r, width / 2 - 220, height * .64f + 48, "Stacja, utwory i glosnosc", 13, muted);
      } else if (s.nearServices()) {
        panel(r, {width / 2 - 245, height * .64f, 490, 70});
        text(r, width / 2 - 220, height * .64f + 10, "[E] JEDNORAZOWE USŁUGI", 25, {.3f, .65f, 1});
        text(r, width / 2 - 220, height * .64f + 48,
             "Punkty: " + std::to_string(s.expedition.servicePoints), 13, muted);
      } else if (s.nearTerminal()) {
        panel(r, {width / 2 - 245, height * .64f, 490, 70});
        text(r, width / 2 - 220, height * .64f + 10, "[E] KOMPUTER / WYPŁATY", 25, mint);
        text(r, width / 2 - 220, height * .64f + 48, "Odbierz ulepszenie lub sprawdź wydobycie", 12,
             muted);
      } else if (s.nearSeat()) {
        panel(r, {width / 2 - 220, height * .64f, 440, 68});
        text(r, width / 2 - 191, height * .64f + 10, "Press E to control", 27, mint);
        text(r, width / 2 - 191, height * .64f + 46, "Zajmij fotel / winda nad dach", 12, muted);
      } else {
        panel(r, {width / 2 - 260, height - 141, 520, 57});
        text(r, width / 2 - 240, height - 129,
             captured ? (s.expedition.enabled && !s.expedition.miningStarted
                             ? "Czerwony przycisk za działem uruchamia wydobycie [E]."
                         : s.expedition.enabled && !s.expedition.rewards.empty()
                             ? "Wypłata czeka przy bocznym komputerze. Podejdź i naciśnij E."
                             : "Podejdź do fotela na środku platformy")
                      : "Kliknij, aby się rozejrzeć. WASD — podejdź do fotela.",
             15);
      }
    } else {
      const float x = width / 2;
      drawManualSight(*this, r, s);
      panel(r, {x - 190, height - 156, 380, 85});
      std::string status =
          s.mode == Mode::Raising    ? "OTWIERANIE WŁAZU / PODNOSZENIE"
          : s.mode == Mode::Lowering ? "POWRÓT NA POKŁAD"
          : s.lanceReserveActive     ? "REZERWA URANOWA / 5 U/s"
          : s.lanceRemaining > 0     ? "CIĄGŁA WIĄZKA / " + number(s.lanceRemaining, 1) + " s"
          : s.lanceCharge > 0        ? "ŁADOWANIE KONDENSATORÓW"
          : s.cooldown > .05f        ? "ŁADOWANIE  " + number(s.cooldown, 1) + " s"
          : s.module.kind == Gun::Smart &&
                  s.world.valid(s.smartCandidate, s.smartCandidateGeneration) &&
                  !s.world.valid(s.smartTarget, s.smartGeneration)
              ? "NAMIERZANIE " + number(s.smartLock, 2) + " / 0,50 s"
          : s.module.kind == Gun::Smart && !s.world.valid(s.smartTarget, s.smartGeneration)
              ? "BRAK CELU W OKIENKU"
              : "GOTOWOŚĆ / LPM — OGIEŃ";
      text(r, x - 170, height - 143, status, 15, accent);
      float progress = s.mode == Mode::Mounted
                           ? 1 - std::clamp(s.cooldown / s.module.interval(), 0.f, 1.f)
                           : s.lift;
      if (s.mode == Mode::Mounted && s.module.kind == Gun::Lance) {
        if (s.lanceReserveActive)
          progress = 1;
        else if (s.lanceRemaining > 0)
          progress = s.lanceRemaining / 3;
        else if (s.cooldown <= 0)
          progress = s.lanceCharge / 1.8f;
      }
      if (s.mode == Mode::Mounted && s.module.kind == Gun::Smart && s.smartCandidate >= 0 &&
          s.smartTarget < 0)
        progress = s.smartLock / .5f;
      rect(r, {x - 170, height - 111, 340, 3}, {.13f, .24f, .27f});
      rect(r, {x - 170, height - 111, 340 * progress, 3}, accent);
      text(r, x - 170, height - 95, "E — opuść stanowisko   |   Mysz — celuj", 11, muted);
    }
    rect(r, {0, height - 46, width, 46}, {.015f, .04f, .052f}, .94f);
    text(r, 26, height - 33,
         s.tactical ? "WASD kamera   Q/E obrót   Kółko zoom   LPM zrzut/kryształ   C dron   TAB "
                      "powrót   P pauza"
                    : "WASD ruch   E fotel/komputer   TAB heksy   LPM ogień   P pauza   R nowa gra "
                      "  F1 pomoc",
         13, muted);
    if (s.world.paused) {
      rect(r, {0, 0, width, height}, {.005f, .015f, .025f}, .72f);
      panel(r, {width / 2 - 235, height / 2 - 155, 470, 300});
      text(r, width / 2 - 200, height / 2 - 125, "PAUZA", 28, mint);
      panel(r, pauseResumeButton());
      text(r, pauseResumeButton().x + 24, pauseResumeButton().y + 18, "WZNÓW GRĘ [P]", 19, mint);
      panel(r, pauseMenuButton());
      text(r, pauseMenuButton().x + 24, pauseMenuButton().y + 18, "WRÓĆ DO MENU GŁÓWNEGO", 18,
           white);
      text(r, width / 2 - 200, height / 2 + 87, "Powrót kończy obecną ekspedycję.", 12, muted);
    } else if (s.serviceTarget != Service::None) {
      panel(r, {width / 2 - 265, 132, 530, 46});
      text(r, width / 2 - 245, 144, "WSKAŻ CEL USŁUGI / CZAS ZATRZYMANY", 17, {.3f, .65f, 1});
    } else if (s.placingSentry()) {
      panel(r, {width / 2 - 250, 132, 500, 46});
      text(r, width / 2 - 230, 144, "WYBÓR STANOWISKA / CZAS ZATRZYMANY", 17, mint);
    }
    if (s.expedition.enabled && s.world.time < s.world.announcementUntil) {
      panel(r, {width / 2 - 340, 84, 680, 38});
      text(r, width / 2 - 320, 94, s.world.announcement, 12, mint);
    }
    if (help) {
      rect(r, {0, 0, width, height}, {.008f, .025f, .04f}, .93f);
      panel(r, {width / 2 - 340, height / 2 - 260, 680, 520});
      text(r, width / 2 - 305, height / 2 - 230, "STANOWISKO OPERATORA", 27, mint);
      text(r, width / 2 - 305, height / 2 - 172,
           "WASD / Shift   Chodzenie / bieg po platformie\nMysz           Rozglądanie / "
           "celowanie\nE              Działo / komputer / powrót windą\nTAB            Heksy / "
           "powrót na pokład\nLPM            Strzelaj / zamów zrzut "
           "(możesz przytrzymać)\n;              Laboratorium ulepszeń\n'              Bestiariusz "
           "/ przywoływanie\nP              Pauza / wznowienie\nR              Powrót do "
           "wyboru broni i nowa próba\nM / V          Dźwięk / VSync\nF11            Pełny "
           "ekran\nEsc            Uwolnij mysz / pauza\nAlt+F4         Wyjście\nF1             "
           "Zamknij pomoc",
           17);
      text(r, width / 2 - 305, height / 2 + 200,
           s.expedition.enabled ? "100 uranu = transport = wybór 1 z 3 kart przy komputerze."
                                : "Nieograniczona amunicja. Brak zakupów i automatycznej obrony.",
           12, muted);
    }
    r.flushText();
  }
};
} // namespace command
