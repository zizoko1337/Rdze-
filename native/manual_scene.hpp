#pragma once
#include "command_game.hpp"
#include "game_ui.hpp"

namespace command {
inline void buildExtraManualGun(GameScene& scene, const Session& s) {
  Color steel{.3f, .38f, .42f}, dark{.055f, .09f, .12f}, c = s.module.spec().color;
  float ready = 1 - std::clamp(s.cooldown / s.module.spec().interval, 0.f, 1.f);
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
  case Gun::Arc:
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
  case Gun::Toxin:
  case Gun::Howitzer: {
    bool toxin = s.module.kind == Gun::Toxin;
    box({1.3f, -.3f, .8f}, {1.2f, .65f, 1.8f}, dark);
    rod({1.5f, .12f, .75f}, {1.5f, .12f, 3.2f}, toxin ? .24f : .39f, steel);
    ring({1.5f, .12f, 3.21f}, toxin ? .28f : .43f, c, 0, true);
    box({1.5f, .12f, .5f - (1 - ready) * .65f}, {.75f, .8f, .45f}, dark);
    rod({.8f, -.5f, -.1f}, {1.3f, -.25f, 1.1f - s.recoil * .4f}, .08f, steel);
    for (int i = 0; i < 4; ++i) {
      float a = (1 - ready) * PI + i * PI / 2;
      Vec3 p{2.05f + std::sin(a) * .38f, -.15f + std::cos(a) * .38f, .3f};
      rod(p, p + Vec3{0, 0, .65f}, .13f, toxin ? c : Color{.7f, .5f, .22f}, toxin);
    }
    box({-.45f, -.3f, 1.1f}, {.55f, .3f, .8f}, dark);
    break;
  }
  case Gun::Smart:
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
    float charge = s.lanceRemaining > 0 ? 1 : s.lanceCharge / 1.8f;
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
      rod({side * .85f, -.45f, 1.1f}, {.9f, -.15f, 2.4f}, .08f, steel);
    }
    rod({.9f, -.15f, .8f}, {.9f, -.15f, 3.2f}, .24f, steel);
    ring({.9f, -.15f, 3.2f}, .38f, dark, s.spin);
    for (int i = 0; i < 6; ++i) {
      float a = s.spin + i * PI / 3;
      box({.9f + std::sin(a) * .32f, -.15f + std::cos(a) * .32f, 2.65f}, {.1f, .1f, .65f}, steel);
    }
    rod({.9f, -.4f, 2.8f}, {.9f, -.4f, 3.3f}, .035f, c, true);
    break;
  default:
    break;
  }
}

inline void appendManualEffects(GameScene& scene, const Session& s) {
  for (const auto& p : s.manualProjectiles) {
    Color c = GUNS[int(p.kind)].color;
    float size = p.kind == Gun::Smart ? .65f : .95f;
    scene.hot.push_back(instance(p.pos, {size, size, 1}, c));
    Vec3 tail = p.kind == Gun::Smart ? p.pos - normalize(p.velocity) * 4 : p.previous;
    scene.beams.push_back(lineInstance(tail, p.pos, c, .15f));
  }
  for (const auto& p : s.toxicPatches) {
    float alpha = std::min(1.f, p.life) * .5f;
    scene.shadows.push_back(instance(p.position, {28, 28, 1}, {.18f, .55f, .025f}, 0, 0, alpha));
    for (int i = 0; i < 3; ++i) {
      float a = i * 2.39996f;
      scene.shadows.push_back(instance(p.position + Vec3{std::sin(a) * 5, .01f, std::cos(a) * 5},
                                       {14, 14, 1}, {.3f, .75f, .035f}, 0, 0, alpha * .7f));
    }
    scene.ringData.push_back(instance(p.position, {28, 28, 1}, {.35f, 1, .12f}, 0, 0, alpha));
    // A few translucent billboards suggest vapour without filling the screen with particles.
    for (int i = 0; i < 7; ++i) {
      float a = i * 2.39996f + s.world.time * .12f, radius = 3 + i * 1.3f;
      Vec3 pos = p.position + Vec3{std::sin(a) * radius, 1 + std::sin(s.world.time * 2 + i),
                                   std::cos(a) * radius};
      scene.smoke.push_back(instance(pos, {5, 3, 1}, {.25f, .65f, .08f}, 0, 0, alpha * .4f));
    }
  }
  if (s.mode == Mode::Mounted && !s.tactical &&
      (s.module.kind == Gun::Howitzer || s.module.kind == Gun::Toxin)) {
    Vec3 end = s.groundAim(), start = s.muzzle(), previous = start;
    Color c = s.module.spec().color;
    float radius = s.module.kind == Gun::Howitzer ? 15 : 14;
    scene.ringData.push_back(instance(end, {radius * 2, radius * 2, 1}, c, 0, 0, .65f));
    for (int i = 1; i <= 24; ++i) {
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
  Color c = s.hitFlash > 0 ? Color{1, .75f, .3f} : s.module.spec().color;
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
  case Gun::Smart: {
    float focal = ui.height * .8660254f;
    corners(x, y, focal * Session::SMART_HALF_X, focal * Session::SMART_HALF_Y, 28);
    ui.text(r, x - 125, y + focal * Session::SMART_HALF_Y + 15,
            s.world.valid(s.smartTarget, s.smartGeneration) ? "CEL PRZECHWYCONY" : "SZUKANIE CELU",
            14, c);
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
    ui.text(r, x + 170, y + 12, "+" + number(distance) + "% DMG", 12, c);
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
    float charge = s.lanceRemaining > 0 ? s.lanceRemaining / 3 : s.lanceCharge / 1.8f;
    corners(x, y, 22 + (1 - charge) * 16, 22 + (1 - charge) * 16, 10);
    line(x - 60, y + 55, 120 * charge, 4);
    ui.text(r, x - 65, y + 70, s.lanceRemaining > 0 ? "WIĄZKA AKTYWNA" : "PRZYTRZYMAJ LPM", 12, c);
    break;
  }
  case Gun::Flame:
    corners(x, y, 120, 65, 20);
    line(x - 40, y, 25, 2);
    line(x + 15, y, 25, 2);
    ui.text(r, x - 80, y + 90, "STOŻEK / 63 m / 3 HEKSY", 13, c);
    break;
  case Gun::Arc:
    corners(x, y, 26, 26, 12);
    line(x - 7, y, 14, 2);
    line(x, y - 7, 2, 14);
    ui.text(r, x - 87, y + 48, "ŁAŃCUCH / SKOK 28 m", 12, c);
    break;
  default: {
    float spread = s.module.kind == Gun::Crank ? 11 + s.spinSpeed * .35f : 6;
    line(x - spread - 9, y, 9, 1);
    line(x + spread, y, 9, 1);
    line(x, y - spread - 9, 1, 9);
    line(x, y + spread, 1, 9);
    break;
  }
  }
  line(x - 1, y - 1, 2, 2);
}
} // namespace command
