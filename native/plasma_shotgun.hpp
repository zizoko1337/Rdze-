#pragma once
#include "simulation.hpp"
namespace command {
using namespace rdzen;
struct PlasmaShotgun {
  static constexpr int PELLETS = 10;
  static constexpr float DAMAGE = 3000, RELOAD = 12, KICK = PI / 18;
  static constexpr Vec3 LOADER_DOCK{0, 2.8f, -2.6f};
  bool reloading = false;
  bool triggerLatched = false;
  int rounds = 2, lastBarrel = 0;
  float age = RELOAD, duration = RELOAD;
  static float ease(float x) {
    x = std::clamp(x, 0.f, 1.f);
    return x * x * (3 - 2 * x);
  }
  static float between(float t, float a, float b) { return ease((t - a) / (b - a)); }
  void begin(float rate) {
    duration = std::clamp(RELOAD / std::max(.1f, rate), 10.f, 15.f);
    age = 0;
    reloading = true;
  }
  void discharge(float rate) {
    lastBarrel = 2 - rounds;
    --rounds;
    triggerLatched = true;
    if (rounds == 0)
      begin(rate);
  }
  void step(float dt) {
    if (reloading) {
      age = std::min(duration, age + dt);
      reloading = age < duration;
      if (!reloading)
        rounds = 2;
    }
  }
  float progress() const { return reloading ? age / duration : 1; }
  float remaining() const { return reloading ? std::max(0.f, duration - age) : 0; }
  float carriageTravel() const {
    float p = progress();
    return between(p, 0, .08f) * (1 - between(p, .96f, 1));
  }
  float elevation() const { return 1.2f * carriageTravel(); }
  float extension() const { return .55f * carriageTravel(); }
  float breakAngle() const {
    float p = progress();
    return .95f * between(p, .085f, .16f) * (1 - between(p, .74f, .82f));
  }
  float lever() const {
    float p = progress();
    return between(p, .83f, .90f) * (1 - between(p, .90f, .955f));
  }
  float extraction() const { return between(progress(), .29f, .36f); }
  float insertion() const { return between(progress(), .56f, .64f); }
  Vec3 loadingAxis() const { return {0, -std::sin(.95f), std::cos(.95f)}; }
  Vec3 chamberPoint(int side) const { return barrelPoint({side == 0 ? -.5f : .5f, 0, .9f}); }
  Vec3 loaderHead() const {
    float p = progress();
    if (!reloading || p < .16f || p >= .74f)
      return LOADER_DOCK;
    Vec3 contact = barrelPoint({0, 0, .9f}) - loadingAxis() * .8f;
    Vec3 overhead = contact + Vec3{0, 1.15f, 0};
    Vec3 extracted = contact - loadingAxis() * .95f;
    auto travel = [](Vec3 a, Vec3 b, float t) { return a + (b - a) * t; };
    if (p < .23f)
      return travel(LOADER_DOCK, overhead, between(p, .16f, .23f));
    if (p < .29f)
      return travel(overhead, contact, between(p, .23f, .27f));
    if (p < .36f)
      return travel(contact, extracted, extraction());
    if (p < .43f)
      return travel(extracted, LOADER_DOCK, between(p, .36f, .43f));
    if (p < .49f)
      return LOADER_DOCK;
    if (p < .56f)
      return travel(LOADER_DOCK, extracted, between(p, .49f, .56f));
    if (p < .67f)
      return travel(extracted, contact, insertion());
    if (p < .70f)
      return travel(contact, overhead, between(p, .67f, .70f));
    return travel(overhead, LOADER_DOCK, between(p, .70f, .74f));
  }
  float loaderGrip() const {
    float p = progress();
    return between(p, .27f, .29f) * (1 - between(p, .64f, .67f));
  }
  float loaderDoors() const {
    float p = progress();
    return std::clamp(between(p, .12f, .16f) * (1 - between(p, .43f, .47f)) +
                          between(p, .47f, .49f) * (1 - between(p, .74f, .78f)),
                      0.f, 1.f);
  }
  Vec3 batteryPoint(int side, bool fresh) const {
    Vec3 chamber = chamberPoint(side);
    if (!reloading)
      return chamber;
    float p = progress();
    if (!fresh && p < .36f)
      return chamber - loadingAxis() * (.95f * extraction());
    if (fresh && p >= .56f)
      return chamber - loadingAxis() * (.95f * (1 - insertion()));
    return loaderHead() + loadingAxis() * .8f + Vec3{side == 0 ? -.5f : .5f, 0, 0};
  }
  bool spentVisible(int side) const { return reloading ? progress() < .49f : side < 2 - rounds; }
  bool freshVisible(int side) const { return reloading ? progress() >= .49f : side >= 2 - rounds; }
  const char* stage() const {
    float p = progress();
    return !reloading  ? (rounds == 2 ? "DWA STRZAŁY GOTOWE" : "DRUGA LUFA GOTOWA")
           : p < .085f ? "PODNOSZENIE DUBELTÓWKI"
           : p < .16f  ? "ŁAMANIE LUF"
           : p < .27f  ? "WYSUWANIE MODUŁU ŁADOWANIA"
           : p < .29f  ? "CHWYTANIE OBU BATERII"
           : p < .43f  ? "ODBIÓR ZUŻYTYCH BATERII"
           : p < .49f  ? "WYMIANA W MAGAZYNIE"
           : p < .64f  ? "WKŁADANIE DWÓCH BATERII"
           : p < .74f  ? "CHOWANIE MODUŁU"
           : p < .83f  ? "ZAMYKANIE LUF"
           : p < .96f  ? "DŹWIGNIA / NAPINANIE MECHANIZMU"
                       : "OPUSZCZANIE DUBELTÓWKI";
  }
  Vec3 barrelPoint(Vec3 p) const {
    Vec3 hinge{0, -.28f, .7f}, d = p - hinge;
    float a = breakAngle();
    return hinge + Vec3{0, elevation(), extension()} +
           Vec3{d.x, d.y * std::cos(a) - d.z * std::sin(a), d.y * std::sin(a) + d.z * std::cos(a)};
  }
};
} // namespace command
