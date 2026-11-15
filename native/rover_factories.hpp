#pragma once
#include "gameplay.hpp"

namespace command {
struct RoverFactories {
  static constexpr int COUNT = 6;
  static constexpr float ARRIVAL = 12, DROP = 1.8f, RETURN = 4, STAGGER = .4f;
  static constexpr float CONNECT_AT = ARRIVAL + DROP + RETURN + STAGGER * (COUNT - 1);
  static constexpr float READY_AT = CONNECT_AT + 1.4f, RELEASE_INTERVAL = .09f;
  struct Route {
    float radius = 160, height = 85, offset = 0, bend = 0, exitOffset = 0;
  };
  std::array<Route, COUNT> routes;
  bool active = false;
  float age = 0, releaseClock = 0;
  int nextFactory = 0;
  std::array<int, COUNT> queued{}, lastBatch{};
  std::array<uint64_t, COUNT> produced{};
  std::array<float, COUNT> doorPulse{};
  static float angle(int index) { return PI / 6 + index * PI / 3; }
  static Vec3 position(int index, float radius = CORE_RADIUS + 3.6f) {
    float a = angle(index);
    return {std::sin(a) * radius, 0, std::cos(a) * radius};
  }
  static Vec3 exit(int index) { return position(index, CORE_RADIUS + 6) + Vec3{0, .4f, 0}; }
  static float ease(float x) {
    x = std::clamp(x, 0.f, 1.f);
    return x * x * (3 - 2 * x);
  }
  void initialize(uint32_t seed = 0x724ed31u) {
    if (active)
      return;
    active = true;
    Random random;
    random.state = seed | 1u;
    for (auto& route : routes) {
      route.radius = 145 + random.unit() * 70;
      route.height = 70 + random.unit() * 35;
      route.offset = (random.unit() - .5f) * .85f;
      route.bend = (random.unit() - .5f) * .65f;
      route.exitOffset = (random.unit() - .5f) * 1.2f;
    }
  }
  float localAge(int index) const { return std::max(0.f, age - index * STAGGER); }
  Vec3 dronePosition(int index) const {
    float t = localAge(index);
    const auto& route = routes[index];
    float radius = CORE_RADIUS + 3.6f, heading = angle(index), height = 10;
    if (t < ARRIVAL) {
      float progress = ease(t / ARRIVAL);
      radius += (route.radius - radius) * (1 - progress);
      heading += route.offset * (1 - progress) + route.bend * std::sin(PI * progress);
      height += (route.height - height) * (1 - progress) + 12 * std::sin(PI * progress);
    } else if (t >= ARRIVAL + DROP) {
      float depart = std::clamp((t - ARRIVAL - DROP) / RETURN, 0.f, 1.f);
      float acceleration = depart * depart;
      radius += 140 * acceleration;
      heading += route.exitOffset * ease(depart);
      height += 130 * acceleration;
    }
    return {std::sin(heading) * radius, height, std::cos(heading) * radius};
  }
  Vec3 cargoPosition(int index) const {
    float t = localAge(index);
    if (t < ARRIVAL)
      return dronePosition(index) - Vec3{0, 4, 0};
    return position(index) + Vec3{0, 6 * (1 - ease((t - ARRIVAL) / DROP)), 0};
  }
  bool droneVisible(int index) const { return active && localAge(index) < ARRIVAL + DROP + RETURN; }
  float connection() const { return ease((age - CONNECT_AT) / (READY_AT - CONNECT_AT)); }
  bool ready() const { return active && age >= READY_AT; }
  int queuedCount() const {
    int total = 0;
    for (int n : queued)
      total += n;
    return total;
  }
  void advance(float dt) {
    if (!active)
      initialize();
    age = std::min(READY_AT, age + std::max(0.f, dt));
    for (auto& pulse : doorPulse)
      pulse = std::max(0.f, pulse - dt);
  }
  void allocate(int requested, int available) {
    lastBatch.fill(0);
    int count = std::clamp(requested, 0, std::max(0, available));
    for (int n = 0; n < count; ++n) {
      int port = (nextFactory + n) % COUNT;
      ++queued[port];
      ++lastBatch[port];
    }
    nextFactory = (nextFactory + count) % COUNT;
  }
};
} // namespace command
