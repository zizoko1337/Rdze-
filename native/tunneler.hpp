#pragma once
#include "worm_path.hpp"

namespace rdzen {
struct TunnelerBody : WormTrail<32, 640> {
  static constexpr float FORWARD = 2.6f, SIDEWAYS = 42, WAVELENGTH = 112, AMPLITUDE = 18;
  int head = -1;
  uint32_t generation = 0;
  int direction = 1, reversals = 0, burnPart = -1;
  float travelled = 0, phase = PI * .25f, dustClock = 0;

  static float segmentSize(int part) { return 12.f - std::max(0, part - 24) * .6f; }
  float depth(float along) const {
    return std::sin(phase + along * (2 * PI / WAVELENGTH)) * AMPLITUDE;
  }
  // Seed a complete underground/overground route before the head enters play.
  void seed(float x, float z) {
    float radius = std::max(1.f, std::hypot(x, z)), angle = std::atan2(x, z);
    auto point = [&](float behind) {
      float r = radius - behind * FORWARD / SIDEWAYS;
      float a = angle + direction * behind / radius;
      return Vec3{std::sin(a) * r, depth(behind), std::cos(a) * r};
    };
    reset(point(-300), {0, 0, 1});
    for (int n = -599; n <= 0; ++n)
      advance(point(n * .5f));
  }
};
} // namespace rdzen
