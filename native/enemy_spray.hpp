#pragma once
#include "gameplay.hpp"

namespace arsenal {
// The muzzle is a socket in billboard space, not the enemy's collision centre.
// Resolve it using the current render camera, including the sniper CRT camera.
struct SiegeSpray {
  Vec3 centre;
  float width, height, time, phase;
  Vec3 muzzle(Vec3 right, Vec3 up) const {
    // Normalized socket in boss-siege.png: u=.92, v=.20.
    return centre + right * (width * .42f) + up * (height * .30f);
  }
  Vec3 impact(Vec3 right, Vec3 up) const {
    Vec3 mouth = muzzle(right, up);
    float radius = std::max(.001f, std::hypot(mouth.x, mouth.z));
    return {mouth.x * (CORE_RADIUS + .1f) / radius, CORE_HEIGHT * .55f,
            mouth.z * (CORE_RADIUS + .1f) / radius};
  }
  Vec3 point(float t, Vec3 right, Vec3 up) const {
    Vec3 a = muzzle(right, up), d = impact(right, up);
    Vec3 b = a + right * 3 + up * 1.7f;
    Vec3 c = d + (a - d) * .35f + Vec3{0, 5, 0};
    float s = 1 - t;
    return a * (s * s * s) + b * (3 * s * s * t) + c * (3 * s * t * t) + d * (t * t * t);
  }
};
} // namespace arsenal
