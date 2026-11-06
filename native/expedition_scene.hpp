#pragma once
#include "command_game.hpp"
#include "game_scene.hpp"

namespace command {
inline void buildExpedition(GameScene& scene, const Session& s) {
  if (!s.expedition.enabled || s.mode == Mode::Selection)
    return;
  Color steel{.48f, .58f, .62f}, dark{.1f, .16f, .19f}, yellow{1, .7f, .13f};
  Vec3 pad = Expedition::DOCK + Vec3{0, -8.5f, 0};
  scene.rod({12, pad.y, -7}, pad, .7f, dark);
  scene.box(pad, {10, .6f, 10}, dark);
  for (float side : {-1.f, 1.f}) {
    scene.box(pad + Vec3{side * 4.5f, .35f, 0}, {.15f, .06f, 9}, yellow, 0, 0, true);
    scene.box(pad + Vec3{0, .35f, side * 4.5f}, {9, .06f, .15f}, yellow, 0, 0, true);
  }
  bool carried = s.expedition.shipping && s.expedition.flightAge >= 6;
  Vec3 ship = s.expedition.shipPosition();
  Vec3 cargo = carried ? ship + Vec3{0, -5.2f, 0} : pad + Vec3{0, 2, 0};
  scene.box(cargo, {5.5f, 3.2f, 5.5f}, steel);
  for (float x : {-2.8f, 2.8f}) {
    scene.box(cargo + Vec3{x, 0, 0}, {.12f, 2.7f, 4.9f}, {.35f, .9f, .28f}, 0, 0, true);
    scene.box(cargo + Vec3{x * .7f, 0, 2.8f}, {.4f, 3.3f, .12f}, yellow);
  }
  if (!s.expedition.shipping)
    return;
  scene.box(ship, {12, 3, 18}, steel);
  scene.box(ship + Vec3{0, 1.8f, -1}, {7, 1.5f, 8}, dark);
  scene.box(ship + Vec3{0, .2f, 9.1f}, {8, .5f, .12f}, {.2f, .9f, 1}, 0, 0, true);
  for (float x : {-10.f, 10.f}) {
    scene.box(ship + Vec3{x * .65f, 0, 0}, {9, 1.3f, 14}, dark);
    for (float z : {-7.f, 7.f}) {
      Vec3 rotor = ship + Vec3{x, 1, z};
      scene.tube(rotor, {3.6f, .35f, 3.6f}, steel);
      scene.tube(rotor + Vec3{0, -.4f, 0}, {2.8f, .12f, 2.8f}, {.12f, .65f, 1}, 0, 0, true);
      scene.box(rotor + Vec3{0, .3f, 0}, {6.4f, .12f, .4f}, dark, s.world.time * 9);
      scene.box(rotor + Vec3{0, .3f, 0}, {.4f, .12f, 6.4f}, dark, s.world.time * 9);
    }
  }
  if (s.expedition.flightAge >= 4) {
    for (float x : {-2.f, 2.f})
      scene.rod(ship + Vec3{x, -1.5f, 0}, cargo + Vec3{x, 1.5f, 0}, .07f, yellow);
  }
}
} // namespace command
