#pragma once
#include "command_game.hpp"
#include "game_scene.hpp"

namespace command {
inline void appendRoverFactories(GameScene& scene, const Session& s, std::vector<Instance>& logos) {
  const auto& factory = s.frontier.factories;
  if (!factory.active)
    return;
  Color steel{.3f, .4f, .43f}, dark{.035f, .085f, .10f}, yellow{1, .69f, .05f}, mint{.25f, 1, .65f};
  for (int i = 0; i < RoverFactories::COUNT; ++i) {
    float a = RoverFactories::angle(i), cs = std::cos(a), sn = std::sin(a);
    Vec3 base = factory.cargoPosition(i);
    auto point = [&](Vec3 p) {
      return base + Vec3{p.x * cs + p.z * sn, p.y, -p.x * sn + p.z * cs};
    };
    auto box = [&](Vec3 p, Vec3 scale, Color c, bool glow = false) {
      scene.box(point(p), scale, c, a, 0, glow);
    };
    box({0, .15f, 0}, {3.9f, .3f, 3.7f}, steel);
    box({0, 1.5f, 0}, {3.6f, 2.7f, 3.3f}, dark);
    box({0, 2.95f, 0}, {3.9f, .2f, 3.6f}, yellow);
    float gate = factory.ready() && (factory.queued[i] > 0 || factory.doorPulse[i] > 0) ? 1.f : 0;
    box({0, 1.1f, 1.68f}, {2.4f, 1.9f, .06f}, {.009f, .018f, .023f});
    for (float side : {-1.f, 1.f}) {
      box({side * (.58f + gate * .63f), 1.15f, 1.73f}, {1.1f, 1.85f, .12f}, steel);
      box({side * 1.5f, 1.1f, 1.77f}, {.13f, 2, .13f}, yellow);
    }
    box({0, 2.25f, 1.75f}, {2.7f, .15f, .12f}, factory.ready() ? mint : yellow, true);
    scene.tube(point({1.25f, 3.55f, -.7f}), {.5f, 1.2f, .5f}, steel);
    box({1.25f, 4.18f, -.7f}, {.75f, .1f, .75f}, dark);
    // One camera-facing decal per installation, shared texture and shared draw call.
    for (float side : {-1.f, 1.f})
      scene.rod(point({side * 1.3f, 3.05f, .4f}), point({side * 1.3f, 3.7f, .4f}), .045f, steel);
    logos.push_back(instance(point({0, 3.8f, .3f}), {3.8f, 1.28f, 1}, {1, 1, 1}));
    if (factory.connection() > 0) {
      Vec3 from = RoverFactories::position(i, CORE_RADIUS + .05f) + Vec3{0, .7f, 0};
      Vec3 to = RoverFactories::position(i, CORE_RADIUS + 2.1f) + Vec3{0, .7f, 0};
      Vec3 tip = from + (to - from) * factory.connection();
      scene.rod(from, tip, .3f, steel);
      scene.rod(from + Vec3{0, .28f, 0}, tip + Vec3{0, .28f, 0}, .08f, mint, factory.ready());
      float phase = std::fmod(s.world.time * 1.7f + i * .17f, 1.f);
      Vec3 signal = from * (1 - phase) + tip * phase + Vec3{0, .28f, 0};
      scene.glowRocks.push_back(instance(signal, {.16f, .16f, .16f}, mint));
    }
    if (factory.droneVisible(i)) {
      Vec3 drone = factory.dronePosition(i);
      scene.box(drone, {3.4f, 1.2f, 3}, dark, a);
      scene.box(drone + Vec3{0, .7f, 0}, {2.7f, .16f, 2.5f}, yellow, a);
      logos.push_back(instance(drone + Vec3{0, 1.6f, 0}, {4.6f, 1.55f, 1}, {1, 1, 1}));
      for (float x : {-2.4f, 2.4f})
        for (float z : {-2.4f, 2.4f}) {
          Vec3 rotor = drone + Vec3{x, .25f, z};
          scene.rod(drone, rotor, .13f, steel);
          scene.tube(rotor, {1.2f, .25f, 1.2f}, dark);
          scene.box(rotor + Vec3{0, .2f, 0}, {2.5f, .07f, .18f}, steel, s.deckClock * 22 + i);
          if (factory.localAge(i) < RoverFactories::ARRIVAL)
            scene.rod(drone + Vec3{x * .5f, -.6f, z * .5f}, base + Vec3{x * .5f, 2.95f, z * .5f},
                      .045f, yellow);
        }
      if (factory.localAge(i) > RoverFactories::ARRIVAL + RoverFactories::DROP) {
        scene.hot.push_back(
            instance(drone + Vec3{0, -1.5f, 0}, {2, 3.5f, 1}, {.3f, .8f, 1}, 0, 0, .75f));
      }
    }
  }
}
} // namespace command
