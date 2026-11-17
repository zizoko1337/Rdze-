#pragma once
#include "simulation.hpp"

namespace command {
enum class Service { None = -1, Brimstone, StunWave, CyberPigs, Count };
struct ServiceCharges {
  static constexpr int COUNT = int(Service::Count);
  bool unlocked = false;
  int rechargeLevel = 0, storageLevel = 0;
  std::array<int, COUNT> charges{};
  std::array<double, COUNT> progress{};
  int capacity() const { return 1 + storageLevel; }
  float seconds() const { return 240.f / (1 + .3f * rechargeLevel); }
  bool ready(Service type) const {
    return unlocked && type >= Service::Brimstone && type < Service::Count &&
           charges[int(type)] > 0;
  }
  int total() const { return charges[0] + charges[1] + charges[2]; }
  void unlock() {
    if (unlocked)
      return;
    unlocked = true;
    charges.fill(1);
    progress.fill(0);
  }
  bool consume(Service type) {
    if (!ready(type))
      return false;
    --charges[int(type)];
    return true;
  }
  void step(float dt) {
    if (!unlocked || dt <= 0)
      return;
    for (int n = 0; n < COUNT; ++n) {
      if (charges[n] >= capacity()) {
        progress[n] = 0;
        continue;
      }
      progress[n] += dt / seconds();
      while (progress[n] + .000001f >= 1 && charges[n] < capacity()) {
        ++charges[n];
        progress[n] = std::max(0.0, progress[n] - 1);
      }
      if (charges[n] >= capacity())
        progress[n] = 0;
    }
  }
};
} // namespace command
