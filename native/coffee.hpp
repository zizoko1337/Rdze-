#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace command {
inline constexpr std::array<const char*, 5> COFFEE_NAMES = {
    "TERRAN ARABICA", "FRONTIER ROAST", "LUNA NITRO", "REACTOR BLEND", "PURE CAFFEINE / STAR DUST"};
inline constexpr std::array<int, 5> COFFEE_MINUTES = {5, 6, 7, 8, 10};

// Only advanced by Session's active simulation clock, never wall or render time.
struct CoffeeState {
  double awakeRemaining = 300, sleepyTime = 0, age = 0;
  float duration = 3;
  int servingTier = 1, cups = 0;
  bool active = false;
  static float cycleDuration(int machineTier) { return float(4 - std::clamp(machineTier, 1, 3)); }
  static float smooth(float t) {
    t = std::clamp(t, 0.f, 1.f);
    return t * t * (3 - 2 * t);
  }
  float fatigue() const {
    return awakeRemaining > 0 ? 0 : .25f + .75f * std::min(1.f, float(sleepyTime / 15));
  }
  float walkingMultiplier() const { return 1 - .55f * fatigue(); }
  float turnLimit() const { return 8 - 7.35f * fatigue(); } // radians / second
  float blink() const {
    if (fatigue() <= 0 || active)
      return 0;
    float phase = float(std::fmod(sleepyTime, 4.5));
    if (phase < 3.5f)
      return 0;
    if (phase < 3.7f)
      return smooth((phase - 3.5f) / .2f);
    if (phase < 4.05f)
      return 1;
    return 1 - smooth((phase - 4.05f) / .45f);
  }
  // Drinking always takes 0.5 s. The remaining phases accelerate with the machine.
  float handlingTime() const { return duration - .5f; }
  float alignEnd() const { return handlingTime() * .06f; }
  float pourEnd() const { return handlingTime() * .72f; }
  float sipStart() const { return handlingTime() * .90f; }
  float sipEnd() const { return sipStart() + .5f; }
  bool pouring() const { return active && age >= alignEnd() && age < pourEnd(); }
  bool drinking() const { return active && age >= sipStart() && age < sipEnd(); }
  float cupLift() const {
    if (!active || age < pourEnd())
      return 0;
    if (age < sipStart())
      return smooth(float(age - pourEnd()) / (sipStart() - pourEnd()));
    if (age < sipEnd())
      return 1;
    return 1 - smooth(float(age - sipEnd()) / (duration - sipEnd()));
  }
  float cupFill() const {
    if (!active)
      return 0;
    if (age < pourEnd())
      return std::clamp(float(age - alignEnd()) / (pourEnd() - alignEnd()), 0.f, 1.f);
    return 1 - std::clamp(float(age - sipStart()) / .5f, 0.f, 1.f);
  }
  float cupTilt() const {
    if (!active || age < sipStart())
      return 0;
    if (age < sipEnd())
      return .75f * smooth(float(age - sipStart()) / .5f);
    return .75f * cupLift();
  }
  bool start(int tier, int machineTier) {
    if (active)
      return false;
    active = true;
    age = 0;
    servingTier = std::clamp(tier, 1, 5);
    duration = cycleDuration(machineTier);
    return true;
  }
  void tick(float dt, bool extractionRunning) {
    if (dt <= 0 || !std::isfinite(dt))
      return;
    if (extractionRunning) {
      double elapsed = dt;
      double awake = std::min(awakeRemaining, elapsed);
      awakeRemaining -= awake;
      sleepyTime += elapsed - awake;
    }
    if (!active)
      return;
    double previous = age;
    age = std::min(double(duration), age + dt);
    // Apply only after the last sip; acquiring a card never refills the timer.
    if (previous < sipEnd() && age >= sipEnd()) {
      awakeRemaining = COFFEE_MINUTES[servingTier - 1] * 60;
      sleepyTime = 0;
      ++cups;
    }
    if (age + 1e-6 >= duration) {
      age = duration;
      active = false;
    }
  }
};
} // namespace command
