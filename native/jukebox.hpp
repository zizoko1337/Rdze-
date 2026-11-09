#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace command {
struct RadioTrack {
  const char* file;
  const char* title;
};
inline constexpr std::array<RadioTrack, 4> RADIO_TRACKS{
    {{"i-mine-uraium.mp3", "I Mine Uraium"},
     {"my-laser-go-brrr.mp3", "My Laser Go Brrr"},
     {"aint-gonna-take-my-uranum-again.mp3", "Ain't Gonna Take My Uranum Again"},
     {"this-is-private-property.mp3", "This Is Private Property"}}};
inline constexpr const char* RADIO_STATION = "URANIUM FM 2137";
struct RadioState {
  float volume = .35f;
  int track = 0;
  uint32_t revision = 0;
  bool playing = true;
  void selectTrack(int index) {
    track =
        (index % int(RADIO_TRACKS.size()) + int(RADIO_TRACKS.size())) % int(RADIO_TRACKS.size());
    ++revision;
  }
  void changeVolume(float amount) { volume = std::clamp(volume + amount, 0.f, 1.f); }
  static float deployment(float age) {
    float t = std::clamp((age - 1.65f) / 1.8f, 0.f, 1.f);
    return t * t * (3 - 2 * t);
  }
};
struct RadioSnapshot {
  int track = 0;
  float position = 0, duration = 0, rms = 0, bass = 0, treble = 0;
  bool playing = false, failed = false;
  uint64_t decodedFrames = 0;
};
} // namespace command
