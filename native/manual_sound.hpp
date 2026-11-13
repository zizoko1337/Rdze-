#pragma once
#include "manual_feedback.hpp"

namespace command {
// Sample-clock synthesis, independent of frame rate and combat random numbers.
// This is a quiet mechanical layer under the existing weapon reports.
class ManualSound {
  ManualAudioState controls{};
  Random noise{0x7132a54b};
  std::array<float, GUN_COUNT> shotAge{};
  float charge = 0, beam = 0, crank = 0, flame = 0, reserve = 0, filteredNoise = 0;
  double time = 0, phase = 0;
  uint64_t lastShots = 0;
  Gun lastGun = Gun::Pulse;

public:
  ManualSound() { shotAge.fill(10); }
  void reset() { *this = ManualSound(); }
  void set(ManualAudioState state) {
    if (!state.active) {
      charge = beam = crank = flame = reserve = 0;
      shotAge.fill(10);
    } else if (state.gun == lastGun && state.shots > lastShots && state.gun != Gun::Lance)
      shotAge[int(state.gun)] = 0;
    if (state.gun != lastGun) {
      charge = beam = crank = flame = reserve = 0;
      shotAge.fill(10);
    }
    lastGun = state.gun;
    lastShots = state.shots;
    controls = state;
  }
  float sample() {
    if (!controls.active)
      return 0;
    constexpr float dt = 1.f / 48000, smooth = 1 - 0.998612075f; // 15 ms envelope.
    auto follow = [](float& level, float target) { level += (target - level) * smooth; };
    bool lance = controls.gun == Gun::Lance;
    follow(charge, lance ? std::clamp(controls.charge, 0.f, 1.f) : 0);
    follow(beam, lance && controls.beam ? 1.f : 0);
    follow(crank, controls.gun == Gun::Crank ? controls.minigun : 0);
    follow(flame, controls.gun == Gun::Flame ? controls.flame : 0);
    follow(reserve, lance && controls.beam && controls.reserve ? 1.f : 0);
    time += dt;
    phase += (94 + charge * 32 + reserve * 7) * dt;
    phase -= std::floor(phase);
    float w = noise.unit() * 2 - 1;
    filteredNoise += (w - filteredNoise) * .08f;
    float t = float(std::fmod(time, 60.0)), a = 2 * PI * t;
    float carrier = std::sin(float(phase) * 2 * PI);
    float pulse = .72f + .28f * std::sin(a * 5.2f);
    float interference =
        .76f + .24f * std::tanh(3 * std::sin(a * 17.3f + 1.3f * std::sin(a * 2.1f)));
    float hum = (.72f * carrier + .18f * std::sin(float(phase) * 4 * PI));
    float out = beam * .043f * hum * pulse * (1 - reserve + reserve * interference);
    out += beam * reserve * .009f * (w - filteredNoise) * (.5f + .5f * std::sin(a * 11.7f));
    out += charge * (1 - beam) * .018f * (carrier + .18f * std::sin(float(phase) * 6 * PI));
    out +=
        crank * (.014f * std::sin(a * 47) + .009f * filteredNoise) * (.8f + .2f * std::sin(a * 12));
    out += flame * .012f * filteredNoise;
    constexpr float freq[] = {165, 95, 82, 58, 43, 72, 94, 53, 68};
    constexpr float length[] = {.18f, .045f, .28f, .32f, .4f, .13f, 0, .32f, .07f};
    constexpr float gain[] = {.025f, .014f, .032f, .035f, .045f, .019f, 0, .045f, .006f};
    for (int n = 0; n < GUN_COUNT; ++n) {
      float age = shotAge[n];
      if (age >= length[n])
        continue;
      float envelope = std::min(1.f, age / .0025f) * std::pow(1 - age / length[n], 2.f);
      float body =
          std::sin(age * 2 * PI * freq[n]) * .7f + std::sin(age * 2 * PI * freq[n] * 2.13f) * .18f;
      out += gain[n] * envelope * (body + w * .12f * std::exp(-age * 45));
      shotAge[n] += dt;
    }
    return std::clamp(out, -.16f, .16f);
  }
};
} // namespace command
