#pragma once
#include "gameplay.hpp"

namespace command {
using namespace arsenal;
struct EnergyShield {
  static constexpr float LAND_TIME = 8, DEPART_TIME = 12;
  static constexpr float STARTUP_TIME = 2.2f, ANTENNA_TIME = 1.25f, ARC_BEGIN = 1.3f;
  static constexpr Vec3 DOCK{9.5f, CORE_HEIGHT + .28f, 4};
  static constexpr float CRATE_SCALE = .38f, DRONE_SCALE = .28f, CARGO_OFFSET = 1.85f;
  static constexpr float ENTRY_HEIGHT = CORE_HEIGHT + 2.95f;
  static float yaw() { return std::atan2(DOCK.x, DOCK.z); }
  static Vec3 entry(float radius) {
    Vec3 direction = normalize(Vec3{DOCK.x, 0, DOCK.z});
    return direction * radius + Vec3{0, ENTRY_HEIGHT, 0};
  }
  bool purchased = false, installed = false, activated = false;
  float mechanismPhase = 0;
  float deliveryAge = 0, hp = 0, maxHp = 1000, regeneration = 10;
  float rebuildDuration = 60, rebuildRemaining = 0, hitFlash = 0;
  uint64_t failures = 0, rebuilds = 0;
  bool starting() const { return installed && !activated; }
  float startupAge() const { return installed ? std::max(0.f, deliveryAge - LAND_TIME) : 0; }
  float antennaExtension() const { return smooth(startupAge() / ANTENNA_TIME); }
  float arcStrength() const {
    return online() ? 1.f : starting() ? smooth((startupAge() - ARC_BEGIN) / .6f) : 0.f;
  }
  bool online() const { return activated && rebuildRemaining <= 0 && hp > 0; }
  bool droneVisible() const { return purchased && deliveryAge < DEPART_TIME; }
  static float smooth(float t) {
    t = std::clamp(t, 0.f, 1.f);
    return t * t * (3 - 2 * t);
  }
  Vec3 dronePosition() const {
    Vec3 dock = DOCK + Vec3{0, CARGO_OFFSET, 0};
    auto leg = [&](Vec3 a, Vec3 b, float start, float duration) {
      float t = smooth((deliveryAge - start) / duration);
      return a * (1 - t) + b * t;
    };
    if (deliveryAge < 3)
      return leg({140, 110, 60}, entry(22), 0, 3);
    if (deliveryAge < 4)
      return leg(entry(22), entry(19), 3, 1);
    if (deliveryAge < 6)
      return leg(entry(19), entry(12), 4, 2);
    if (deliveryAge < LAND_TIME) {
      return leg(entry(12), dock, 6, 2);
    }
    if (deliveryAge < 9.2f)
      return leg(dock, entry(12), 8, 1.2f);
    if (deliveryAge < 10.5f)
      return leg(entry(12), entry(22), 9.2f, 1.3f);
    return leg(entry(22), {115, 125, -55}, 10.5f, 1.5f);
  }
  Vec3 cratePosition() const {
    return installed ? DOCK : dronePosition() - Vec3{0, CARGO_OFFSET, 0};
  }
  bool order() {
    if (purchased)
      return false;
    purchased = true;
    return true;
  }
  void step(float dt) {
    if (!purchased || dt <= 0)
      return;
    hitFlash = std::max(0.f, hitFlash - dt);
    deliveryAge = std::min(DEPART_TIME, deliveryAge + dt);
    if (!installed && deliveryAge >= LAND_TIME) {
      installed = true;
    }
    if (!installed)
      return;
    mechanismPhase = std::fmod(mechanismPhase + dt * (online()     ? 2.8f
                                                      : starting() ? 1.7f
                                                                   : .65f),
                               2 * PI);
    if (!activated) {
      if (startupAge() < STARTUP_TIME)
        return;
      activated = true;
      hp = maxHp;
    }
    if (rebuildRemaining > 0) {
      rebuildRemaining = std::max(0.f, rebuildRemaining - dt);
      if (rebuildRemaining <= 0) {
        hp = maxHp;
        ++rebuilds;
      }
    } else {
      hp = std::min(maxHp, hp + regeneration * dt);
    }
  }
  float absorb(float damage) {
    damage = std::max(0.f, damage);
    if (!online() || damage <= 0)
      return damage;
    float absorbed = std::min(hp, damage);
    hp -= absorbed;
    hitFlash = .4f;
    if (hp <= 0) {
      hp = 0;
      rebuildRemaining = rebuildDuration;
      ++failures;
    }
    return std::max(0.f, damage - absorbed);
  }
  void fasterRebuild(float seconds) {
    float old = rebuildDuration;
    rebuildDuration = std::max(20.f, rebuildDuration - seconds);
    if (rebuildRemaining > 0)
      rebuildRemaining *= rebuildDuration / old;
  }
};
} // namespace command
