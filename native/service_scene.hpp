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
      scene.ringData.push_back(instance({p.x, .23f, p.z}, {80, 80, 1}, color, 0, 0, .8f));
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
        float punch = std::exp(-std::fmod(cast.age - 3, .5f) * 12);
        scene.beams.push_back(
            lineInstance(cast.position + Vec3{0, 1000, 0}, cast.position, color, 10 + punch * 6));
        scene.beams.push_back(lineInstance(cast.position + Vec3{0, 1000, 0}, cast.position,
                                           {1, .95f, .8f}, 3 + punch * 2));
        scene.hot.push_back(instance(cast.position + Vec3{0, 2, 0},
                                     {20 + punch * 15, 20 + punch * 15, 1}, color, 0, 0, .8f));
        for (int i = 0; i < 5; ++i) {
          float a = cast.age * 5 + i * 2 * PI / 5;
          Vec3 offset{std::sin(a) * 5, 0, std::cos(a) * 5};
          scene.beams.push_back(lineInstance(cast.position + offset + Vec3{0, 180, 0},
                                             cast.position - offset, {1, .65f, .15f}, .5f));
        }
      }
    } else if (cast.type == Service::Overload) {
      scene.ringData.push_back(
          instance(cast.position + Vec3{0, .25f, 0}, {80, 80, 1}, color, 0, 0, .8f));
      if (!cast.triggered) {
        serviceHex(scene, cast.position, color, .14f + std::sin(cast.age * 20) * .04f);
        float size = 2 + cast.age;
        scene.hot.push_back(
            instance(cast.position + Vec3{0, 5, 0}, {size, size, 1}, color, 0, 0, .5f));
      } else {
        float t = cast.age - 3, life = std::max(0.f, 1 - t / 2);
        float radius = 4 + std::min(1.f, t / .7f) * 36;
        scene.ringData.push_back(instance(cast.position + Vec3{0, 1, 0},
                                          {radius * 2, radius * 2, 1}, {1, .85f, .4f}, 0, 0, life));
        scene.hot.push_back(instance(cast.position + Vec3{0, 8, 0}, {25 + t * 25, 25 + t * 25, 1},
                                     {1, .5f, .12f}, 0, 0, life * life * .8f));
        scene.smoke.push_back(instance(cast.position + Vec3{0, 8 + t * 10, 0},
                                       {35 + t * 14, 25 + t * 16, 1}, {.12f, .1f, .08f}, 0, 0,
                                       life * .6f));
      }
    } else if (cast.age < 3) {
      float size = 4 + std::fmod(cast.age, 1.f) * 25;
      scene.ringData.push_back(
          instance({0, CORE_HEIGHT + .4f, 0}, {size, size, 1}, color, 0, 0, .8f));
    }
  }
}
} // namespace command
