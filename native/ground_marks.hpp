#pragma once
#include "simulation.hpp"

namespace rdzen {
struct GroundMark {
  Vec3 a, b;
  float radius, life, duration, heat;
};
// Purely cosmetic, bounded storage. No gameplay RNG, damage or collision changes.
class GroundMarks {
  size_t cursor = 0;

public:
  static constexpr size_t capacity = 1024;
  std::vector<GroundMark> marks;
  GroundMarks() { marks.reserve(capacity); }
  void tick(float dt) {
    for (size_t i = 0; i < marks.size();) {
      marks[i].life -= dt;
      if (marks[i].life <= 0) {
        marks[i] = marks.back();
        marks.pop_back();
      } else
        ++i;
    }
  }
  void stroke(Vec3 a, Vec3 b, float radius, float duration, float heat = 0) {
    auto grounded = [](Vec3 p) {
      return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) && std::abs(p.y) <= .3f;
    };
    if (!grounded(a) || !grounded(b) || !std::isfinite(radius) || !std::isfinite(duration) ||
        !std::isfinite(heat) || !(radius > 0) || !(duration > 0))
      return;
    a.y = b.y = .035f;
    // Sustained fire at one point refreshes the mark instead of stacking opacity.
    for (size_t n = marks.size(), stop = n > 24 ? n - 24 : 0; n > stop;) {
      auto& m = marks[--n];
      Vec3 before = m.b - m.a, next = b - a;
      if (std::abs(m.radius - radius) < .01f && std::abs(m.heat - heat) < .01f &&
          m.duration - m.life < .1f && length(m.b - a) < .01f && length(before) > radius &&
          length(next) > .001f && length(b - m.a) <= 3 &&
          dot(normalize(before), normalize(next)) > .985f) {
        m.b = b;
        m.life = m.duration = duration;
        return;
      }
      if (std::abs(m.radius - radius) < .01f && std::abs(m.heat - heat) < .01f &&
          length(m.a - a) < radius * .3f && length(m.b - b) < radius * .3f) {
        m.life = m.duration = duration;
        return;
      }
    }
    GroundMark mark{a, b, radius, duration, duration, heat};
    if (marks.size() < capacity)
      marks.push_back(mark);
    else {
      marks[cursor] = mark;
      cursor = (cursor + 1) % capacity;
    }
  }
  void spot(Vec3 p, float radius, float duration, float heat = 0) {
    stroke(p, p, radius, duration, heat);
  }
};
} // namespace rdzen
