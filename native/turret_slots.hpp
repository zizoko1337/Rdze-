#pragma once
#include "gameplay.hpp"

namespace command {
using namespace arsenal;
inline constexpr int SLOT_RINGS = 8, SLOT_COUNT = 3 * SLOT_RINGS * (SLOT_RINGS + 1) - 6;
inline constexpr float SLOT_SPACING = SUPPORT_SPACING, SLOT_RADIUS = SLOT_SPACING * .536f;
inline constexpr float TACTICAL_ZOOM = 420;
struct TurretSlot {
  int q, r;
  float x, z;
};
inline constexpr auto TURRET_SLOTS = [] {
  std::array<TurretSlot, SLOT_COUNT> slots{};
  int index = 0;
  auto absInt = [](int v) { return v < 0 ? -v : v; };
  for (int r = -SLOT_RINGS; r <= SLOT_RINGS; ++r)
    for (int q = -SLOT_RINGS; q <= SLOT_RINGS; ++q) {
      int ring = std::max({absInt(q), absInt(r), absInt(q + r)});
      if (ring < 2 || ring > SLOT_RINGS)
        continue;
      slots[index++] = {q, r, SLOT_SPACING * (q + r * .5f), SLOT_SPACING * .866025404f * r};
    }
  return slots;
}();
inline int nearestTurretSlot(float x, float z) {
  if (!std::isfinite(x) || !std::isfinite(z))
    return -1;
  int best = -1;
  float distance = SLOT_SPACING * SLOT_SPACING / 3;
  for (int i = 0; i < SLOT_COUNT; ++i) {
    float dx = x - TURRET_SLOTS[i].x, dz = z - TURRET_SLOTS[i].z;
    float d = dx * dx + dz * dz;
    if (d < distance) {
      distance = d;
      best = i;
    }
  }
  return best;
}
struct SentryDrop {
  static constexpr float LAND_TIME = 6, END_TIME = 10;
  int slot = -1;
  float age = 0;
  bool installed = false;
  Weapon type = Laser;
  float cargoHeight() const {
    float t = std::clamp(age / LAND_TIME, 0.f, 1.f);
    return 100 * (1 - t * t * (3 - 2 * t));
  }
  float balloonHeight() const {
    if (!installed)
      return cargoHeight() + 20;
    float t = std::max(0.f, age - LAND_TIME);
    return 20 + 8 * t + 25 * t * t;
  }
};
} // namespace command
