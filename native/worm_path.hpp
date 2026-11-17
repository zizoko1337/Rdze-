#pragma once
#include "simulation.hpp"

namespace rdzen {
// Distance-indexed history: each body centre retraces the head's path, not the
// shortest line to its neighbour. Storage/work remain bounded at any speed.
template <int SegmentCount = 8, int HistoryCount = 128> struct WormTrail {
  static constexpr int SEGMENTS = SegmentCount, HISTORY = HistoryCount;
  static constexpr float GAP = 7, SAMPLE = .5f;
  std::array<Vec3, SEGMENTS> segments{};

private:
  struct Point {
    Vec3 position;
    double distance = 0;
  };
  std::array<Point, HISTORY> history{};
  int newest = HISTORY - 1;
  Vec3 lastHead{};
  double distance = 0;

public:
  void reset(Vec3 head, Vec3 backwards) {
    backwards = normalize(backwards);
    lastHead = head;
    distance = 0;
    newest = HISTORY - 1;
    for (int i = 0; i < HISTORY; ++i) {
      float behind = (HISTORY - 1 - i) * SAMPLE;
      history[i] = {head + backwards * behind, -double(behind)};
    }
    for (int i = 0; i < SEGMENTS; ++i)
      segments[i] = head + backwards * (GAP * (i + 1));
  }
  void advance(Vec3 head) {
    Vec3 delta = head - lastHead;
    float travelled = length(delta);
    if (!(travelled > 0) || !std::isfinite(travelled))
      return;
    double before = distance;
    distance += travelled;
    double next = history[newest].distance + SAMPLE;
    // A debug teleport only needs the newest body-length of the straight jump.
    next = std::max(next, std::floor(distance / SAMPLE) * SAMPLE - (HISTORY - 1) * SAMPLE);
    for (int added = 0; next <= distance && added < HISTORY; ++added, next += SAMPLE) {
      float t = float(std::clamp((next - before) / travelled, 0.0, 1.0));
      newest = (newest + 1) % HISTORY;
      history[newest] = {lastHead + delta * t, next};
    }
    lastHead = head;
    Vec3 newer = head;
    double newerDistance = distance;
    int offset = 0;
    for (int segment = 0; segment < SEGMENTS; ++segment) {
      double target = distance - GAP * (segment + 1);
      while (offset < HISTORY) {
        const auto& older = history[(newest - offset + HISTORY) % HISTORY];
        if (older.distance <= target) {
          double span = newerDistance - older.distance;
          float t = span > 0 ? float(std::clamp((target - older.distance) / span, 0.0, 1.0)) : 0;
          segments[segment] = older.position + (newer - older.position) * t;
          break;
        }
        newer = older.position;
        newerDistance = older.distance;
        ++offset;
      }
      if (offset >= HISTORY)
        segments[segment] = newer;
    }
  }
};
using WormPath = WormTrail<>;
} // namespace rdzen
