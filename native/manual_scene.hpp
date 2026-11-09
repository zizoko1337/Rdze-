#pragma once
#include "command_game.hpp"
#include "game_ui.hpp"

namespace command {
inline void gunLabel(GameScene& scene, const Session& s, Vec3 p, const std::string& value,
                     float size, Color color) {
  Vec3 start = p;
  for (unsigned char ch : value) {
    if (ch == '\n') {
      p.x = start.x;
      p.y -= size * 1.8f;
      continue;
    }
    scene.gunLetters.push_back(
        instance(s.gunPoint(p), {size, size * 1.5f, 1}, color, s.gunYaw, float(ch), -s.gunPitch));
    p.x += size * .56f;
  }
}
inline void buildExtraManualGun(GameScene& scene, const Session& s) {
  Color steel{.3f, .38f, .42f}, dark{.055f, .09f, .12f}, c = s.module.spec().color;
  if (s.module.kind == Gun::Toxin && s.module.plasmaGlue)
    c = {.16f, .4f, .055f};
  float ready =
      1 - std::clamp(s.cooldown / (s.module.interval() / s.module.rateMultiplier), 0.f, 1.f);
  auto box = [&](Vec3 p, Vec3 size, Color color, bool glow = false) {
    p.z -= s.recoil * .23f;
    scene.box(s.gunPoint(p), size, color, s.gunYaw, -s.gunPitch, glow);
  };
  auto rod = [&](Vec3 a, Vec3 b, float radius, Color color, bool glow = false) {
    a.z -= s.recoil * .23f;
    b.z -= s.recoil * .23f;
    scene.rod(s.gunPoint(a), s.gunPoint(b), radius, color, glow);
  };
  auto ring = [&](Vec3 center, float radius, Color color, float phase, bool glow = false) {
    for (int i = 0; i < 12; ++i) {
      float a = phase + i * PI / 6, b = a + PI / 6;
      rod(center + Vec3{std::sin(a) * radius, std::cos(a) * radius, 0},
          center + Vec3{std::sin(b) * radius, std::cos(b) * radius, 0}, .045f, color, glow);
    }
  };
  switch (s.module.kind) {
  case Gun::Pulse:
    if (!s.module.plasma)
      break;
    box({0, -.1f, 1.55f}, {1.95f, .28f, 1.8f}, steel);
    box({0, .07f, 1.55f}, {1.72f, .08f, 1.6f}, dark);
    for (int i = 0; i < 20; ++i) {
      if (!PlasmaSpec::visibleEmitter(i))
        continue;
      float x = float(i % 5 - 2) * .32f, z = .9f + float(i / 5) * .42f;
      rod({x, .12f, z}, {x, .42f, z}, .055f, steel);
      rod({x, .42f, z}, {x, .47f, z}, .085f, c, true);
      box({x, .08f, z}, {.19f, .07f, .2f}, c, true);
    }
    for (float side : {-1.f, 1.f}) {
      rod({side * .65f, -.18f, -.25f}, {side * 1.08f, -.18f, 1.65f}, .09f, steel);
      rod({side * 1.08f, -.18f, 1.65f}, {side * .85f, .04f, 2.25f}, .08f, c, true);
      for (int i = 0; i < 3; ++i) {
        float z = .05f + i * .5f;
        float wave = .7f + .3f * std::sin(s.deckClock * 8 - i * 1.8f);
        rod({side * .91f, -.12f, z}, {side * .91f, .12f + wave * .08f, z}, .13f, c, true);
        box({side * .91f, -.20f, z}, {.35f, .09f, .35f}, dark);
      }
    }
    break;
  case Gun::Arc:
    if (s.module.teslaAmplifier) {
      box({.82f, .4f, .5f}, {.42f, .18f, .7f}, dark);
      rod({.82f, .51f, .45f}, {.82f, .67f, s.teslaSupport ? .7f : .3f}, .065f,
          s.teslaSupport ? Color{.3f, .65f, 1} : c, true);
      gunLabel(scene, s, {.45f, .5f, .1f}, s.teslaSupport ? "BOOST" : "ATTACK", .095f, c);
    }
    for (float side : {-1.f, 1.f}) {
      rod({side * .62f, -.1f, .5f}, {side * .62f, -.1f, 2.8f}, .16f, dark);
      for (int i = 0; i < 6; ++i)
        ring({side * .62f, -.1f, .9f + i * .26f}, .28f, i % 2 ? steel : c,
             s.deckClock * 1.4f * side, i % 2 == 0 && ready > .7f);
      box({side * (.62f + .13f * (1 - ready)), .1f, 3}, {.18f, .55f, .55f}, steel);
      box({side * .55f, .1f, 3.15f}, {.12f, .35f, .12f}, c, true);
    }
    if (ready > .8f) {
      float y = .1f + std::sin(s.deckClock * 20) * .12f;
      scene.beams.push_back(
          lineInstance(s.gunPoint({-.55f, .1f, 3.15f}), s.gunPoint({0, y, 3.3f}), c, .035f));
      scene.beams.push_back(
          lineInstance(s.gunPoint({0, y, 3.3f}), s.gunPoint({.55f, .1f, 3.15f}), c, .035f));
    }
    break;
  case Gun::Toxin: {
    // Open chemical catapult: mixing reservoirs, metering pumps and a filling spoon.
    auto cycle = s.toxinCycle();
    float throwAngle = cycle.throwAngle;
    Vec3 pivot{1.45f, -.35f, .5f};
    Vec3 cup = pivot + Vec3{0, .35f + std::sin(throwAngle) * 1.3f, 1.75f * std::cos(throwAngle)};
    box({1.45f, -.6f, 1.1f}, {1.35f, .25f, 2.5f}, dark);
    for (float side : {-1.f, 1.f}) {
      rod({1.45f + side * .5f, -.5f, .45f}, {1.45f + side * .38f, .05f, .5f}, .08f, steel);
      rod(pivot + Vec3{side * .38f, 0, 0}, cup + Vec3{side * .25f, -.1f, 0}, .09f, steel);
      box({1.45f + side * .48f, -.34f, 1.3f}, {.12f, .15f, 2.1f}, steel);
    }
    rod(pivot - Vec3{.65f, 0, 0}, pivot + Vec3{.65f, 0, 0}, .14f, steel);
    box(cup + Vec3{0, -.06f, 0}, {.74f, .12f, .8f}, steel);
    for (float side : {-1.f, 1.f})
      box(cup + Vec3{side * .34f, .12f, 0}, {.08f, .32f, .8f}, dark);
    box(cup + Vec3{0, .12f, -.36f}, {.74f, .32f, .08f}, dark);
    float fill = cycle.fill;
    if (fill > .01f) {
      box(cup + Vec3{0, .02f + fill * .08f, 0}, {.58f, .03f + fill * .1f, .61f}, c, true);
      scene.glowRocks.push_back(instance(s.gunPoint(cup + Vec3{0, .08f + fill * .14f, 0}),
                                         {.24f * fill, .16f * fill, .24f * fill}, c));
    }
    for (int i = 0; i < 2; ++i) {
      float x = i ? 2.25f : .85f, phase = s.deckClock * 6 + i * PI;
      rod({x, -.5f, -.3f}, {x, .48f, -.3f}, .25f, dark);
      rod({x, -.36f, -.3f}, {x, .3f, -.3f}, .16f, c, true);
      ring({x, -.05f, -.57f}, .24f, steel, 0);
      rod({x + std::sin(phase) * .17f, -.05f + std::cos(phase) * .17f, -.58f}, {x, -.05f, -.58f},
          .045f, steel);
      float pump = .12f * std::sin(phase);
      rod({x, .45f, -.3f}, {x, .7f + pump, -.3f}, .065f, steel);
      box({x, .73f + pump, -.3f}, {.32f, .09f, .3f}, steel);
      rod({x, .4f, -.25f}, {x, .9f, .9f}, .07f, steel);
      rod({x, .9f, .9f}, {1.45f, .9f, .9f}, .07f, steel);
    }
    float angle = cycle.lift * PI * .5f;
    Vec3 hinge{1.45f, .9f, .9f};
    Vec3 elbow = hinge + Vec3{0, std::sin(angle) * 1.35f, std::cos(angle) * 1.35f};
    Vec3 tip = elbow + Vec3{0, -std::cos(angle) * .32f, std::sin(angle) * .32f};
    rod(hinge, elbow, .08f, steel);
    rod(elbow, tip, .11f, dark);
    rod(hinge - Vec3{.15f, 0, 0}, hinge + Vec3{.15f, 0, 0}, .14f, steel);
    if (cycle.pouring) {
      rod(tip, cup + Vec3{0, .1f + fill * .1f, 0}, .04f, c, true);
      scene.hot.push_back(
          instance(s.gunPoint(cup + Vec3{0, .18f, 0}), {.2f, .2f, 1}, c, 0, 0, .35f));
    }
    break;
  }
  case Gun::Howitzer: {
    if (s.module.bouncyMortar) {
      // A caged glass cartridge meters anti-gravity fluid into the breech.
      Color yellow{1, .78f, .06f};
      Vec3 bottom{.82f, .1f, 1.25f};
      box(bottom + Vec3{0, .32f, .17f}, {.3f, .62f, .03f}, dark);
      rod(bottom + Vec3{0, .04f, 0}, bottom + Vec3{0, .05f + ready * .51f, 0},
          .115f, yellow, true);
      for (float side : {-1.f, 1.f})
        rod(bottom + Vec3{side * .16f, 0, 0}, bottom + Vec3{side * .16f, .65f, 0},
            .025f, steel);
      rod(bottom, bottom + Vec3{0, .035f, 0}, .2f, steel);
      rod(bottom + Vec3{0, .62f, 0}, bottom + Vec3{0, .69f, 0}, .2f, steel);
      Vec3 feed{1.5f, .12f, 1.65f};
      rod(bottom, bottom + Vec3{0, -.12f, .15f}, .055f, steel);
      rod(bottom + Vec3{0, -.12f, .15f}, feed, .055f, steel);
      if (s.cooldown > 0 && ready < .14f) {
        float injection = std::clamp(ready / .14f, 0.f, 1.f);
        Vec3 start = bottom + Vec3{0, -.12f, .15f};
        Vec3 pulse = start * (1 - injection) + feed * injection;
        rod(pulse, pulse + (feed - start) * .18f, .075f, yellow, true);
      }
    }
    box({1.3f, -.3f, .8f}, {1.2f, .65f, 1.8f}, dark);
    rod({1.5f, .12f, .75f}, {1.5f, .12f, 3.2f}, .39f, steel);
    ring({1.5f, .12f, 3.21f}, .43f, c, 0, true);
    box({1.5f, .12f, .5f - (1 - ready) * .65f}, {.75f, .8f, .45f}, dark);
    rod({.8f, -.5f, -.1f}, {1.3f, -.25f, 1.1f - s.recoil * .4f}, .08f, steel);
    for (int i = 0; i < 4; ++i) {
      float a = (1 - ready) * PI + i * PI / 2;
      Vec3 p{2.05f + std::sin(a) * .38f, -.15f + std::cos(a) * .38f, .3f};
      rod(p, p + Vec3{0, 0, .65f}, .13f, {.7f, .5f, .22f});
    }
    box({-.45f, -.3f, 1.1f}, {.55f, .3f, .8f}, dark);
    break;
  }
  case Gun::Smart:
    if (s.module.instantLock) {
      box({0, .45f, .55f}, {.6f, .22f, .6f}, dark);
      rod({0, .44f, .8f}, {0, .44f, 1.1f}, .14f, c, true);
    }
    for (float side : {-1.f, 1.f}) {
      box({side * 1.15f, -.12f, 1.1f}, {.9f, 1.05f, 2.5f}, dark);
      for (int i = 0; i < 3; ++i) {
        Vec3 p{side * 1.15f, -.24f + i * .32f, 2.4f};
        rod(p - Vec3{0, 0, 1.3f}, p, .14f, steel);
        ring(p, .16f, c, 0, true);
        float opening = s.shotFlash > 0 && (s.shots % 2 ? -1.f : 1.f) == side ? .18f : .02f;
        box(p + Vec3{side * (.2f + opening), 0, .04f}, {.09f, .25f, .06f}, steel);
      }
      box({side * 1.15f, .55f, .25f}, {.75f, .08f, .4f}, c, true);
    }
    box({0, -.15f, 1.2f}, {.45f, .3f, 1.2f}, steel);
    break;
  case Gun::Lance: {
    float charge = s.lanceActive ? 1 : s.lanceCharge / 1.8f;
    if (s.lanceReserveActive)
      c = {.25f, 1, .18f};
    if (s.module.uraniumReserve) {
      Color feed = s.lanceReserveActive ? Color{.25f, 1, .18f} : Color{.35f, .4f, .42f};
      rod({-.55f, .25f, -.3f}, {-.82f, .52f, .25f}, .12f, feed, s.lanceReserveActive);
      rod({-.82f, .52f, .25f}, {-.82f, .52f, 1.7f}, .12f, feed, s.lanceReserveActive);
      rod({-.82f, .52f, 1.7f}, {-.4f, .05f, 2.4f}, .12f, feed, s.lanceReserveActive);
      for (float z : {0.f, .7f, 1.4f})
        ring({-.82f, .52f, z + .25f}, .17f, steel, 0);
    }
    for (float side : {-1.f, 1.f}) {
      float x = side * (.45f + charge * .18f);
      box({x, -.05f, 1.9f}, {.24f, .38f, 2.7f}, dark);
      box({x - side * .13f, -.05f, 2}, {.035f, .22f, 2.6f}, c, true);
      for (int i = 0; i < 4; ++i)
        box({x + side * .1f, .16f, .9f + i * .5f}, {.5f, .08f, .2f}, steel);
    }
    for (int i = 0; i < 3; ++i)
      ring({0, -.05f, 1 + i * .7f}, .48f + charge * .16f, i == 1 ? c : steel,
           s.deckClock * (1 + charge * 4) * (i % 2 ? -1 : 1), i == 1);
    break;
  }
  case Gun::Rail:
    if (s.module.doubleRangeBonus) {
      // Fixed receiver bracket: the display never follows barrel recoil.
      auto screenBox = [&](Vec3 p, Vec3 size, Color color, bool glow = false) {
        scene.box(s.gunPoint(p), size, color, s.gunYaw, -s.gunPitch, glow);
      };
      screenBox({-.88f, .77f, 1.27f}, {.86f, .56f, .28f}, dark);
      screenBox({-.88f, .77f, 1.12f}, {.75f, .46f, .02f}, {.012f, .055f, .045f}, true);
      scene.rod(s.gunPoint({-.55f, -.3f, 1.4f}), s.gunPoint({-.88f, .48f, 1.4f}),
                .07f, steel);
      auto hit = trace(s.world, s.eye(), s.forward(), s.module.spec().range);
      float meters = hit.id >= 0 ? length(s.world.targetPoint(hit.id) - s.eye()) : hit.distance;
      gunLabel(scene, s, {-1.21f, .87f, 1.10f},
               "RANGE " + number(meters) + "m\nDMG " + number(s.sniperDamageAt(meters)), .07f, c);
      for (int i = 0; i < 9; ++i)
        screenBox({-.88f, .56f + i * .05f, 1.102f}, {.72f, .003f, .003f}, {.02f, .1f, .06f}, true);
      float roll = std::fmod(s.deckClock * .23f, .42f);
      screenBox({-.88f, .56f + roll, 1.101f}, {.72f, .005f, .003f}, {.04f, .2f, .13f}, true);
    }
    for (float side : {-1.f, 1.f}) {
      box({side * .24f, -.08f, 2.1f}, {.18f, .28f, 3.5f}, steel);
      box({side * .14f, -.08f, 2.1f}, {.035f, .14f, 3.4f}, c, true);
      for (int i = 0; i < 5; ++i)
        box({side * .42f, -.08f, .7f + i * .55f - (1 - ready) * .22f}, {.25f, .5f, .1f}, dark);
    }
    // The CRT rests on a bracket below the bezel, behind the glass plane.
    scene.box(s.gunPoint({.60f, -.3f, 1.5f}), {1, .16f, .48f}, steel, s.gunYaw, -s.gunPitch);
    scene.box(s.gunPoint({.95f, -.22f, 1.5f}), {.72f, .18f, .52f}, dark, s.gunYaw, -s.gunPitch);
    scene.box(s.gunPoint({.95f, .02f, 1.5f}), {.3f, .5f, .28f}, steel, s.gunYaw, -s.gunPitch);
    scene.box(s.gunPoint({.95f, .75f, 1.42f}), {1.45f, .96f, .42f}, dark, s.gunYaw, -s.gunPitch);
    scene.box(s.gunPoint({.95f, .75f, 1.195f}), {1.3f, .82f, .01f}, {.015f, .08f, .07f}, s.gunYaw,
              -s.gunPitch, true);
    scene.box(s.gunPoint({1.6f, .35f, 1.19f}), {.035f, .035f, .02f}, c, s.gunYaw, -s.gunPitch,
              true);
    for (float x : {1.22f, 1.42f})
      scene.rod(s.gunPoint({x, .33f, 1.15f}), s.gunPoint({x, .33f, 1.21f}), .035f, steel);
    break;
  case Gun::Flame:
    for (float side : {-1.f, 1.f}) {
      rod({side * .85f, -.45f, -.25f}, {side * .85f, -.45f, 1.15f}, .27f, dark);
      ring({side * .85f, -.45f, .1f}, .29f, steel, 0);
      box({side * .85f, -.15f, .15f}, {.22f, .04f, .6f}, c, true);
      rod({side * .85f, -.45f, 1.1f}, {s.module.dualFlame && side < 0 ? -.9f : .9f, -.15f, 2.4f},
          .08f, steel);
    }
    rod({.9f, -.15f, .8f}, {.9f, -.15f, 3.2f + s.flameMorph * 1.1f}, .24f - .12f * s.flameMorph,
        steel);
    ring({.9f, -.15f, 3.2f + s.flameMorph * 1.1f}, .38f - .23f * s.flameMorph, dark, s.spin);
    if (!s.module.dualFlame) {
      box({.55f, .4f, .65f}, {.4f, .16f, .65f}, dark);
      rod({.55f, .5f, .5f}, {.55f, .58f, .4f + s.flameMorph * .4f}, .055f,
          s.flameFocused ? Color{.3f, .65f, 1} : c, true);
    } else {
      rod({-.9f, -.15f, .8f}, {-.9f, -.15f, 4.3f}, .12f, steel);
      ring({-.9f, -.15f, 4.3f}, .15f, dark, s.spin);
      for (int i = 0; i < 6; ++i) {
        float a = i * PI / 3;
        box({-.9f + std::sin(a) * .13f, -.15f + std::cos(a) * .13f, 3.75f}, {.08f, .08f, .65f},
            steel);
      }
      rod({-.9f, -.35f, 3.8f}, {-.9f, -.35f, 4.4f}, .035f, c, true);
    }
    for (int i = 0; i < 6; ++i) {
      float a = s.spin + i * PI / 3;
      float radius = .32f - .2f * s.flameMorph;
      box({.9f + std::sin(a) * radius, -.15f + std::cos(a) * radius, 2.65f + s.flameMorph * 1.1f},
          {.1f, .1f, .65f}, steel);
    }
    rod({.9f, -.4f, 2.8f + s.flameMorph * 1.1f}, {.9f, -.4f, 3.3f + s.flameMorph * 1.1f}, .035f, c,
        true);
    break;
  default:
    break;
  }
}

inline void appendManualEffects(GameScene& scene, const Session& s) {
  if (s.module.kind == Gun::Lance && s.lanceActive && s.mode == Mode::Mounted) {
    auto hit = trace(s.world, s.eye(), s.forward(), s.module.spec().range);
    Color color = s.lanceReserveActive ? Color{.25f, 1, .18f} : s.module.spec().color;
    scene.beams.push_back(lineInstance(s.muzzle(), hit.point, color, .65f));
    scene.beams.push_back(lineInstance(s.muzzle(), hit.point, {1, 1, 1}, .14f));
    scene.hot.push_back(instance(hit.point, {1.1f, 1.1f, 1}, color));
  }
  for (const auto& p : s.manualProjectiles) {
    Color c = GUNS[int(p.kind)].color;
    if (p.glue)
      c = {.16f, .4f, .055f};
    float size = p.kind == Gun::Smart ? .65f : .95f;
    scene.hot.push_back(instance(p.pos, {size, size, 1}, c));
    Vec3 tail = p.kind == Gun::Smart ? p.pos - normalize(p.velocity) * 4
                : p.plasma           ? p.pos - normalize(p.pos - p.previous) * 4
                                     : p.previous;
    scene.beams.push_back(lineInstance(tail, p.pos, c, p.plasma ? .22f : .15f));
    if (p.plasma)
      scene.beams.push_back(lineInstance(tail, p.pos, {1, 1, 1}, .055f));
  }
  for (const auto& p : s.toxicPatches) {
    float alpha = std::min(1.f, p.life) * .5f;
    scene.shadows.push_back(instance(p.position, {28, 28, 1},
                                     p.glue ? Color{.055f, .16f, .015f} : Color{.18f, .55f, .025f},
                                     0, 0, alpha));
    for (int i = 0; i < 3; ++i) {
      float a = i * 2.39996f;
      scene.shadows.push_back(
          instance(p.position + Vec3{std::sin(a) * 5, .01f, std::cos(a) * 5}, {14, 14, 1},
                   p.glue ? Color{.08f, .2f, .02f} : Color{.3f, .75f, .035f}, 0, 0, alpha * .7f));
    }
    scene.ringData.push_back(instance(p.position, {28, 28, 1},
                                      p.glue ? Color{.12f, .35f, .04f} : Color{.35f, 1, .12f}, 0, 0,
                                      alpha));
    // A few translucent billboards suggest vapour without filling the screen with particles.
    for (int i = 0; i < 7; ++i) {
      float a = i * 2.39996f + s.world.time * .12f, radius = 3 + i * 1.3f;
      Vec3 pos = p.position + Vec3{std::sin(a) * radius, 1 + std::sin(s.world.time * 2 + i),
                                   std::cos(a) * radius};
      scene.smoke.push_back(instance(pos, {5, 3, 1},
                                     p.glue ? Color{.08f, .2f, .035f} : Color{.25f, .65f, .08f}, 0,
                                     0, alpha * .4f));
    }
  }
  if (s.mode == Mode::Mounted && !s.tactical &&
      (s.module.kind == Gun::Howitzer || s.module.kind == Gun::Toxin ||
       (s.module.kind == Gun::Pulse && s.module.plasma))) {
    Vec3 end = s.groundAim(), start = s.muzzle(), previous = start;
    Color c = s.module.spec().color;
    float radius = s.module.plasma                  ? PlasmaSpec::spreadRadius
                   : s.module.kind == Gun::Howitzer ? 15
                                                    : 14;
    scene.ringData.push_back(instance(end, {radius * 2, radius * 2, 1}, c, 0, 0, .65f));
    for (int i = 1; !s.module.plasma && i <= 24; ++i) {
      float t = i / 24.f;
      Vec3 point = start * (1 - t) + end * t;
      point.y += std::sin(t * PI) * (s.module.kind == Gun::Toxin ? 25 : 40);
      if (i % 2)
        scene.beams.push_back(lineInstance(previous, point, c, .055f, .5f));
      previous = point;
    }
  }
}

inline void drawManualSight(GameUi& ui, Renderer& r, const Session& s) {
  float x = ui.width / 2, y = ui.height / 2;
  Color c = s.hitFlash > 0         ? Color{1, .75f, .3f}
            : s.lanceReserveActive ? Color{.25f, 1, .18f}
                                   : s.module.spec().color;
  auto line = [&](float px, float py, float w, float h) { ui.rect(r, {px, py, w, h}, c, .9f); };
  auto corners = [&](float cx, float cy, float w, float h, float n) {
    for (float side : {-1.f, 1.f}) {
      line(cx + side * w - (side > 0 ? n : 0), cy - h, n, 2);
      line(cx + side * w - (side > 0 ? n : 0), cy + h, n, 2);
      line(cx + side * w, cy - h, 2, n);
      line(cx + side * w, cy + h - n, 2, n);
    }
  };
  switch (s.module.kind) {
  case Gun::Pulse:
    if (s.module.plasma) {
      corners(x, y, 35, 24, 13);
      ui.text(r, x + 54, y - 8, "20 IMPULSÓW / 10 m", 14, c);
      ui.text(r, x + 54, y + 16, number(s.manualDamage() * 20) + " DMG / SALWA", 12, c);
    } else {
      line(x - 15, y, 9, 1);
      line(x + 6, y, 9, 1);
      line(x, y - 15, 1, 9);
      line(x, y + 6, 1, 9);
    }
    break;
  case Gun::Smart: {
    float focal = ui.height * .8660254f;
    corners(x, y, focal * Session::SMART_HALF_X, focal * Session::SMART_HALF_Y, 28);
    ui.text(r, x - 125, y + focal * Session::SMART_HALF_Y + 15,
            s.world.valid(s.smartTarget, s.smartGeneration) ? "CEL PRZECHWYCONY"
            : s.smartCandidate >= 0 ? "NAMIERZANIE " + number(s.smartLock, 2) + " / 0,50 s"
                                    : "SZUKANIE CELU",
            14, c);
    if (!s.module.instantLock && s.smartCandidate >= 0)
      line(x - 80, y + focal * Session::SMART_HALF_Y + 42, 160 * s.smartLock / .5f, 3);
    if (s.world.valid(s.smartTarget, s.smartGeneration)) {
      Vec3 f = s.forward(), right = normalize(cross({0, 1, 0}, f)), up = cross(f, right);
      Vec3 d = s.world.targetPoint(s.smartTarget) - s.eye();
      float depth = dot(d, f);
      if (depth > 0) {
        float tx = x + dot(d, right) / depth * focal, ty = y - dot(d, up) / depth * focal;
        corners(tx, ty, 17, 17, 8);
        line(tx - 2, ty - 2, 4, 4);
      }
    }
    break;
  }
  case Gun::Rail: {
    corners(x, y, 150, 90, 25);
    line(x - 110, y, 100, 1);
    line(x + 10, y, 100, 1);
    line(x, y - 70, 1, 60);
    line(x, y + 10, 1, 60);
    for (int i = -3; i <= 3; ++i)
      if (i)
        line(x + i * 28, y - 4, 1, 8);
    auto hit = trace(s.world, s.eye(), s.forward(), s.module.spec().range);
    float distance = hit.id >= 0 ? length(s.world.targetPoint(hit.id) - s.eye()) : hit.distance;
    ui.text(r, x + 170, y - 12, number(distance) + " m", 15, c);
    ui.text(r, x + 170, y + 12,
            "+" + number(distance * (s.module.doubleRangeBonus ? 2 : 1)) + "% DMG", 12, c);
    ui.text(r, x - 85, y + 108, "PRZEBICIE / 8 CELÓW", 12, c);
    break;
  }
  case Gun::Howitzer:
  case Gun::Toxin: {
    corners(x, y, 24, 18, 10);
    for (int i = 1; i <= 4; ++i)
      line(x - i * 3, y + 25 + i * 16, i * 6, 1);
    float duration =
        std::clamp(length(s.groundAim() - s.muzzle()) / s.module.spec().speed, .6f, 5.f);
    ui.text(r, x + 42, y - 8, "LOT " + number(duration, 1) + " s", 14, c);
    ui.text(r, x + 42, y + 15, s.module.kind == Gun::Toxin ? "PLAMA / 14 m" : "WYBUCH / 15 m", 12,
            c);
    break;
  }
  case Gun::Lance: {
    float charge =
        s.lanceActive ? (s.lanceReserveActive ? 1 : s.lanceRemaining / 3) : s.lanceCharge / 1.8f;
    corners(x, y, 22 + (1 - charge) * 16, 22 + (1 - charge) * 16, 10);
    line(x - 60, y + 55, 120 * charge, 4);
    ui.text(r, x - 110, y + 70,
            s.lanceReserveActive ? "REZERWA / 5 U/s / " + number(s.expedition.uranium, 1) + " U"
            : s.lanceActive      ? "WIĄZKA AKTYWNA"
                                 : "PRZYTRZYMAJ LPM",
            12, s.lanceReserveActive ? Color{.25f, 1, .18f} : c);
    break;
  }
  case Gun::Flame:
    corners(x, y, s.flameFocused ? 22 : 120, s.flameFocused ? 22 : 65, 12);
    line(x - 40, y, 25, 2);
    line(x + 15, y, 25, 2);
    if (s.module.dualFlame)
      corners(x, y, 22, 22, 8);
    ui.text(r, x - 140, y + 90,
            s.module.dualFlame ? "DUAL / 63 m + 94,5 m"
            : s.flameFocused   ? "SKUPIONY / 94,5 m / 5 m"
                               : "SZEROKI / 63 m / 3 HEKSY",
            13, c);
    if (!s.module.dualFlame)
      ui.text(r, x - 100, y + 112, "[PPM] ZMIEŃ DYSZĘ", 12, c);
    break;
  case Gun::Arc:
    corners(x, y, 26, 26, 12);
    line(x - 7, y, 14, 2);
    line(x, y - 7, 2, 14);
    ui.text(r, x - 87, y + 48, "ŁAŃCUCH / SKOK 56 m", 12, c);
    if (s.module.teslaAmplifier)
      ui.text(r, x - 140, y + 72,
              s.teslaSupport ? "WZMACNIANIE +100% / PPM: ATAK" : "PPM: TRYB WZMACNIANIA", 12, c);
    break;
  default: {
    float spread = s.module.kind == Gun::Crank
                       ? (11 + s.spinSpeed * .35f) * (s.module.extremeCooling ? 2 : 1)
                       : 6;
    line(x - spread - 9, y, 9, 1);
    line(x + spread, y, 9, 1);
    line(x, y - spread - 9, 1, 9);
    line(x, y + spread, 1, 9);
    break;
  }
  }
  if (s.module.kind == Gun::Crank) {
    ui.text(r, x - 100, y + 64, "TEMP. " + number(s.barrelHeat * 100) + "%", 12,
            s.coolantActive ? Color{.55f, .85f, 1} : Color{1, .45f, .25f});
    if (s.module.extremeCooling)
      ui.text(r, x - 160, y + 88,
              s.coolantActive ? "EXTREME COOLING / CHŁODZENIE" : "TEMPO x5 / ROZRZUT x2", 12, c);
  }
  line(x - 1, y - 1, 2, 2);
}
} // namespace command
