#pragma once
#include <SDL3/SDL.h>
#include "gameplay.hpp"

namespace arsenal {
class GameAudio {
  struct Voice {
    float phase = 0, age = 0, duration = .15f, frequency = 440, gain = .04f;
    int type = 0;
  };
  SDL_AudioStream* stream = nullptr;
  std::vector<Voice> voices;
  Random noise;
  std::array<uint64_t, WeaponCount> last{};
  std::array<float, WeaponCount> lastTime{};

public:
  bool enabled = true;
  GameAudio() {
    voices.reserve(32);
    if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
      SDL_AudioSpec spec{};
      spec.format = SDL_AUDIO_F32;
      spec.channels = 1;
      spec.freq = 48000;
      stream =
          SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
      if (stream)
        SDL_ResumeAudioStreamDevice(stream);
    }
    if (!stream)
      enabled = false;
  }
  ~GameAudio() {
    if (stream)
      SDL_DestroyAudioStream(stream);
  }
  void reset() {
    last.fill(0);
    lastTime.fill(-1);
    voices.clear();
  }
  void build() {
    if (enabled && voices.size() < 32)
      voices.push_back({0, 0, .2f, 880, .055f, 0});
  }
  void update(const Game& g) {
    if (!stream)
      return;
    for (int i = 0; i < WeaponCount; ++i) {
      if (enabled && !g.paused && g.fired[i] > last[i] && g.time - lastTime[i] > .07f &&
          voices.size() < 32) {
        float frequency[] = {850, 100, 190, 75, 240, 320, 90, 48, 80};
        float duration[] = {.12f, .07f, .4f, .3f, .3f, .22f, .25f, .5f, .1f};
        voices.push_back({0, 0, duration[i], frequency[i], i == Flame ? .015f : .035f, i});
        lastTime[i] = g.time;
      }
      last[i] = g.fired[i];
    }
    if (!enabled || g.paused)
      voices.clear();
    int queued = SDL_GetAudioStreamQueued(stream);
    if (queued < 0)
      return;
    constexpr int block = 512;
    float buffer[block];
    while (queued < 2048 * int(sizeof(float))) {
      for (int s = 0; s < block; ++s) {
        float out = 0;
        for (auto& v : voices) {
          if (v.age >= v.duration)
            continue;
          float f = 1 - v.age / v.duration;
          v.phase += v.frequency * (.4f + .6f * f) / 48000.f;
          v.phase -= std::floor(v.phase);
          float wave = std::sin(v.phase * 2 * PI);
          if (v.type == Minigun || v.type == Tesla || v.type == Rocket || v.type == Mortar ||
              v.type == Flame)
            wave = wave * .3f + (noise.unit() * 2 - 1) * .7f;
          out += wave * v.gain * f * f;
          v.age += 1.f / 48000;
        }
        buffer[s] = std::clamp(out, -.5f, .5f);
      }
      SDL_PutAudioStreamData(stream, buffer, sizeof(buffer));
      queued += sizeof(buffer);
    }
    voices.erase(
        std::remove_if(voices.begin(), voices.end(), [](auto& v) { return v.age >= v.duration; }),
        voices.end());
  }
};
} // namespace arsenal
