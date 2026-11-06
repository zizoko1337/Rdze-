#pragma once
#include "command_game.hpp"
#include "game_scene.hpp"
namespace command {
inline void serviceHex(GameScene& scene, Vec3 p, Color color, float width) {
  for (int i = 0; i < 6; ++i) {
    float a = PI / 6 + i * PI / 3, b = a + PI / 3;
    scene.rod(p + Vec3{std::cos(a) * SLOT_RADIUS, .3f, std::sin(a) * SLOT_RADIUS},
              p + Vec3{std::cos(b) * SLOT_RADIUS, .3f, std::sin(b) * SLOT_RADIUS}, width, color,
              true);
  }
}
inline void appendServiceEffects(GameScene& scene, const Session& s, Vec3 hover, int slot) {
  if (s.serviceTarget != Service::None) {
    Color color = SERVICES[int(s.serviceTarget)].color;
    if (s.serviceTarget == Service::Overload && slot >= 0) {
      const auto& p = TURRET_SLOTS[slot];
      bool valid = false;
      for (const auto& t : s.world.towers)
        if (std::hypot(t.x - p.x, t.z - p.z) < .5f && !s.services.reserved(t.id))
          valid = true;
      serviceHex(scene, {p.x, 0, p.z}, valid ? color : Color{.4f, .15f, .15f}, .18f);
    } else if (s.serviceTarget == Service::Brimstone)
      scene.ringData.push_back(instance({hover.x, .2f, hover.z}, {20, 20, 1}, color, 0, 0, .85f));
  }
  for (const auto& cast : s.services.casts) {
    Color color = SERVICES[int(cast.type)].color;
    if (cast.type == Service::Brimstone) {
      scene.ringData.push_back(
          instance(cast.position + Vec3{0, .25f, 0}, {20, 20, 1}, color, 0, 0, .9f));
      if (cast.age < 3) {
        float size = 2 + (3 - cast.age) * 3;
        scene.ringData.push_back(
            instance(cast.position + Vec3{0, .3f, 0}, {size, size, 1}, {1, .7f, .3f}, 0, 0, .8f));
      } else {
        scene.beams.push_back(
            lineInstance(cast.position + Vec3{0, 300, 0}, cast.position, color, 3.5f));
        scene.beams.push_back(
            lineInstance(cast.position + Vec3{0, 300, 0}, cast.position, {1, .85f, .5f}, 1));
        scene.hot.push_back(instance(cast.position + Vec3{0, 2, 0}, {9, 9, 1}, color, 0, 0, .8f));
      }
    } else if (cast.type == Service::Overload) {
      serviceHex(scene, cast.position, color, .14f + std::sin(cast.age * 20) * .04f);
      if (!cast.triggered) {
        float size = 2 + cast.age;
        scene.hot.push_back(
            instance(cast.position + Vec3{0, 5, 0}, {size, size, 1}, color, 0, 0, .5f));
      }
    } else if (cast.age < 3) {
      float size = 4 + std::fmod(cast.age, 1.f) * 25;
      scene.ringData.push_back(
          instance({0, CORE_HEIGHT + .4f, 0}, {size, size, 1}, color, 0, 0, .8f));
    }
  }
}
} // namespace command
