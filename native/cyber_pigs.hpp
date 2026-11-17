#pragma once
#include "gameplay.hpp"

namespace command {
using namespace arsenal;
inline float cyberPigFacing(Vec3 direction, Vec3 cameraRight) {
  return dot(direction, cameraRight) < 0 ? -1.f : 1.f;
}
struct CyberPig {
  Vec3 position{}, previous{}, direction{};
  float radius = 0, startAngle = 0, sign = 1, escapeAge = 0;
  float shotClock = 0, dustClock = 0;
  double travelled = 0;
  bool escaping = false;
  // One 1500-damage contact per enemy generation per lap, with constant-time lookup.
  std::vector<uint64_t> contacts = std::vector<uint64_t>(MAX_FOES, 0);
};
struct CyberPigSystem {
  static constexpr size_t MAX_PIGS = 64;
  static constexpr float SPEED = 28, LASER_RANGE = 70, LASER_DAMAGE = 200, LASER_INTERVAL = .4f;
  static constexpr double FULL_TOUR = 5 * 2 * 3.14159265358979323846;
  std::vector<CyberPig> pigs;
  uint64_t contactHits = 0, laserShots = 0, completedTours = 0;
  CyberPigSystem() { pigs.reserve(MAX_PIGS); }
  bool release(Vec3 point) {
    if (pigs.size() + 2 > MAX_PIGS)
      return false;
    float radius = std::hypot(point.x, point.z);
    if (!std::isfinite(radius) || radius < CORE_RADIUS + 5 || radius > BUILD_RADIUS + 1)
      return false;
    for (int n = 0; n < 2; ++n) {
      CyberPig pig;
      pig.radius = radius;
      pig.sign = n ? -1.f : 1.f;
      pig.startAngle = std::atan2(point.x, point.z) + pig.sign * 1.8f / radius;
      pig.position =
          pig.previous = {std::sin(pig.startAngle) * radius, 0, std::cos(pig.startAngle) * radius};
      pig.direction = {std::cos(pig.startAngle) * pig.sign, 0,
                       -std::sin(pig.startAngle) * pig.sign};
      pig.shotClock = n * .2f;
      pigs.push_back(std::move(pig));
    }
    return true;
  }
  void step(Game& g, float dt) {
    if (g.paused || dt <= 0)
      return;
    for (size_t n = 0; n < pigs.size();) {
      auto& pig = pigs[n];
      pig.previous = pig.position;
      if (pig.escaping) {
        pig.escapeAge += dt;
        pig.position = pig.position + pig.direction * (48 * dt);
      } else {
        pig.travelled = std::min(FULL_TOUR, pig.travelled + SPEED * double(dt) / pig.radius);
        float angle = pig.startAngle + pig.sign * float(pig.travelled);
        pig.position = {std::sin(angle) * pig.radius, 0, std::cos(angle) * pig.radius};
        pig.direction = {std::cos(angle) * pig.sign, 0, -std::sin(angle) * pig.sign};
        Vec3 d = pig.position - pig.previous, middle = (pig.position + pig.previous) * .5f;
        float segmentSq = dot(d, d);
        int lap = std::min(4, int(pig.travelled / (2 * 3.14159265358979323846)));
        g.targetsNear(middle.x, middle.z, 40 + length(d) * .5f, [&](int id) {
          const auto& foe = g.foes[id];
          auto point = g.targetPoint(id, pig.position);
          if (point.y - foe.size * .43f > 3)
            return;
          Vec3 delta{point.x - pig.previous.x, 0, point.z - pig.previous.z};
          float t = segmentSq > 1e-8f ? std::clamp(dot(delta, d) / segmentSq, 0.f, 1.f) : 0;
          Vec3 closest = pig.previous + d * t;
          float radius = 1.6f + foe.size * .43f;
          if (std::hypot(point.x - closest.x, point.z - closest.z) > radius)
            return;
          uint64_t key = (uint64_t(foe.generation) << 8) | uint64_t(lap + 1);
          if (pig.contacts[id] == key)
            return;
          pig.contacts[id] = key;
          g.hurt(id, 1500, Laser, false, DamageClass::Collision);
          ++contactHits;
        });
        pig.shotClock -= dt;
        if (pig.shotClock <= 0) {
          pig.shotClock += LASER_INTERVAL;
          int target = -1;
          float best = LASER_RANGE * LASER_RANGE;
          Vec3 muzzle = pig.position + Vec3{0, 3.25f, 0} + pig.direction * 1.5f;
          g.targetsNear(pig.position.x, pig.position.z, LASER_RANGE, [&](int id) {
            Vec3 delta = g.targetPoint(id, muzzle) - muzzle;
            float sq = dot(delta, delta);
            if (sq < best && dot(normalize(delta), pig.direction) > -.25f &&
                coreHitFraction(muzzle, g.targetPoint(id, muzzle)) >= 1) {
              target = id;
              best = sq;
            }
          });
          if (target >= 0) {
            Vec3 end = g.targetPoint(target, muzzle);
            g.beam(muzzle, end, {.15f, .9f, 1}, .18f, .14f);
            g.beam(muzzle, end, {.85f, 1, 1}, .05f, .1f);
            g.hurt(target, LASER_DAMAGE, Laser, false);
            ++laserShots;
          }
        }
        if (pig.travelled >= FULL_TOUR) {
          pig.escaping = true;
          pig.direction = normalize(pig.direction + normalize(pig.position) * .5f);
          ++completedTours;
        }
      }
      pig.dustClock -= dt;
      if (pig.dustClock <= 0) {
        pig.dustClock += .1f;
        g.particle(pig.position - pig.direction * 2 + Vec3{0, .35f, 0},
                   pig.direction * (-2) + Vec3{0, 1.2f, 0}, {.28f, .23f, .17f}, 1.3f, .85f, 1);
      }
      if (pig.escapeAge >= 18) {
        pigs[n] = std::move(pigs.back());
        pigs.pop_back();
      } else
        ++n;
    }
  }
};
} // namespace command
