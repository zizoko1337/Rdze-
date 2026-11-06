#pragma once
#include "command_game.hpp"
#include "game_scene.hpp"

namespace command {
inline constexpr Vec3 RADAR_TV_POSITION{0, CORE_HEIGHT + 3.35f, 7.5f};
inline void appendRadarTelevision(GameScene& scene, const Session& s) {
  if (!s.expedition.enabled || s.expedition.levels[size_t(Upgrade::Radar)] == 0 ||
      s.mode == Mode::Selection)
    return;
  const Vec3 tv = RADAR_TV_POSITION;
  Color frame{.16f, .23f, .26f}, green{.25f, .9f, .6f};
  for (float side : {-1.f, 1.f}) {
    float x = side * 1.8f;
    scene.rod({x, Session::roofHeight(std::hypot(x, tv.z)) - .18f, tv.z}, {x, tv.y + .95f, tv.z},
              .055f, frame);
    scene.box({x, tv.y + .94f, tv.z}, {.28f, .14f, .4f}, frame);
  }
  scene.box(tv, {4.8f, 1.9f, .35f}, frame);
  scene.box(tv + Vec3{0, 0, -.2f}, {4.48f, 1.6f, .035f}, {.015f, .055f, .065f}, 0, 0, true);
  auto pixel = [&](float x, float y, float w, float h, Color color, float depth = -.25f) {
    scene.box(tv + Vec3{x, y, depth}, {w, h, .012f}, color, 0, 0, true);
  };
  constexpr float cx = -.95f, radius = .65f;
  for (int k = -2; k <= 2; ++k) {
    pixel(cx + k * radius / 2, 0, .008f, radius * 2, {.04f, .19f, .16f});
    pixel(cx, k * radius / 2, radius * 2, .008f, {.04f, .19f, .16f});
  }
  float sweep = std::fmod(s.world.time / 3, 1.f) * 2 * radius - radius;
  pixel(cx, sweep, 2 * radius, .014f, {.08f, .35f, .25f}, -.27f);
  auto contact = [&](float x, float z, float size, Color color) {
    float distance = std::max(350.f, std::hypot(x, z));
    pixel(cx + x / distance * radius, z / distance * radius, size, size, color, -.29f);
  };
  for (const auto& t : s.world.towers)
    contact(t.x, t.z, .025f, green);
  for (int id : s.world.active) {
    const auto& foe = s.world.foes[id];
    if (foe.hp > 0)
      contact(foe.x, foe.z, foe.type >= Titan ? .065f : .025f,
              foe.type >= Titan ? Color{1, .6f, .15f} : Color{1, .18f, .2f});
  }
  for (int i = 1; i <= 10; ++i)
    contact(std::sin(s.gunYaw) * i * 10, std::cos(s.gunYaw) * i * 10, .015f, {.2f, .8f, 1});
  contact(0, 0, .06f, green);
  for (const auto& rover : s.frontier.rovers)
    contact(rover.position.x, rover.position.z, .018f, {.3f, 1, .9f});
  for (const auto& c : s.frontier.crystals)
    if (c.state != CrystalState::Carried)
      contact(c.position.x, c.position.z, .045f, c.boss ? Color{1, .7f, .2f} : Color{.6f, .5f, 1});
  // Live extraction, integrity and contact telemetry beside the map.
  for (int i = 0; i < 3; ++i) {
    float y = .45f - i * .35f;
    float value = i == 0   ? s.expedition.hp / s.expedition.maxHp
                  : i == 1 ? s.expedition.uranium / 100
                           : std::min(1.f, s.world.active.size() / 200.f);
    pixel(1, y, 1.6f, .12f, {.04f, .14f, .16f});
    float width = std::max(.02f, value * 1.6f);
    pixel(.2f + width / 2, y, width, .08f, i == 2 ? Color{1, .25f, .2f} : green, -.28f);
  }
  pixel(2.14f, -.67f, .06f, .06f, green);
}
} // namespace command
