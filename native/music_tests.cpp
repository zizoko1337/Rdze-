#include "music_player.hpp"
#include "command_game.hpp"
#include <iostream>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
int main(int, char**) {
  try {
    rdzen::check(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "COM");
    rdzen::check(MFStartup(MF_VERSION), "Media Foundation");
    auto assets = std::filesystem::current_path() / "public/music";
    for (const auto& track : RADIO_TRACKS) {
      Mp3Reader reader;
      reader.open(assets / track.file);
      require(reader.duration > 10 && reader.frequency >= 22050 && reader.channels >= 1,
              "Invalid MP3 metadata");
      uint64_t frames = 0;
      double energy = 0;
      for (int i = 0; i < 100; ++i) {
        rdzen::Com<IMFSample> sample;
        if (!reader.read(sample))
          break;
        if (!sample.p)
          continue;
        rdzen::Com<IMFMediaBuffer> buffer;
        rdzen::check(sample->ConvertToContiguousBuffer(buffer.put()), "PCM buffer");
        BYTE* bytes = nullptr;
        DWORD size = 0;
        rdzen::check(buffer->Lock(&bytes, nullptr, &size), "PCM lock");
        auto* pcm = reinterpret_cast<const int16_t*>(bytes);
        for (DWORD j = 0; j < size / 2; ++j) {
          double f = pcm[j] / 32768.;
          energy += f * f;
        }
        frames += size / (reader.channels * 2);
        buffer->Unlock();
      }
      require(frames > 1000 && energy > .01, "MP3 decoder returned silence / no PCM");
      std::cout << "PASS MP3 " << track.title << " duration=" << reader.duration
                << " rate=" << reader.frequency << " decoded=" << frames << '\n';
    }
    {
      Session s(true, true);
      s.mode = Mode::Walking;
      s.buttonAge = 0;
      require(!s.nearRadio() && RadioState::deployment(1.65f) == 0,
              "Radio appears before button retracts");
      s.buttonAge = 4;
      s.player = {0, CORE_HEIGHT + 1.72f, 5.8f};
      require(RadioState::deployment(s.buttonAge) == 1 && s.nearRadio() && s.interact() &&
                  s.radioOpen,
              "Deployed radio cannot be opened");
      float clock = s.world.time;
      s.step(.05f);
      require(s.world.time == clock && s.interact() && !s.radioOpen,
              "Radio menu does not pause combat / close correctly");
      s.radio.changeVolume(-100);
      require(s.radio.volume == 0, "Volume below zero");
      s.radio.changeVolume(100);
      require(s.radio.volume == 1, "Volume above one");
      s.radio.selectTrack(-1);
      require(s.radio.track == 3, "Previous track does not wrap");
      s.radio.selectTrack(4);
      require(s.radio.track == 0, "Next track does not wrap");
      std::cout << "PASS deployment timing, radio interaction, combat pause, volume bounds and "
                   "playlist wrap\n";
    }
    {
      SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
      require(SDL_Init(SDL_INIT_AUDIO), "SDL audio init failed");
      MusicPlayer music(assets, true);
      RadioState radio;
      music.sync(radio, false, false, true);
      SDL_Delay(100);
      require(music.snapshot().decodedFrames == 0, "Music starts before radio deployment");
      uint64_t previous = 0;
      for (int i = 0; i < 4; ++i) {
        radio.selectTrack(i);
        music.sync(radio, true, false, true);
        bool ok = false;
        for (int j = 0; j < 400; ++j) {
          SDL_Delay(10);
          auto status = music.snapshot();
          if (status.track == i && status.decodedFrames > previous + 3000 && status.duration > 10 &&
              status.playing) {
            ok = true;
            break;
          }
        }
        require(ok && !music.snapshot().failed, "Streaming / track switching failed");
        previous = music.snapshot().decodedFrames;
      }
      float peak = 0, minimum = 1;
      for (int i = 0; i < 100; ++i) {
        SDL_Delay(20);
        float value = music.snapshot().bass;
        peak = std::max(peak, value);
        minimum = std::min(minimum, value);
      }
      require(peak > .01f && peak - minimum > .001f,
              "Visualizer does not respond to changing decoded audio");
      music.sync(radio, true, false, false);
      require(music.snapshot().bass == 0, "Muted audio leaves visible speaker pulse");
      music.sync(radio, true, false, true);
      radio.playing = false;
      music.sync(radio, true, false, true);
      SDL_Delay(100);
      float position = music.snapshot().position;
      SDL_Delay(150);
      require(!music.snapshot().playing && std::abs(position - music.snapshot().position) < .03f,
              "Radio pause advances playback");
      radio.playing = true;
      music.sync(radio, true, false, true);
      SDL_Delay(200);
      require(music.snapshot().playing && music.snapshot().position > position,
              "Radio resume failed");
      music.reset();
      music.sync(RadioState{}, false, false, true);
      SDL_Delay(100);
      require(!music.snapshot().playing && music.snapshot().bass == 0,
              "Radio reset leaves audio / visualizer active");
      std::cout << "PASS bounded real-time MP3 streaming, four tracks, pause/resume and reset\n";
    }
    SDL_Quit();
    MFShutdown();
    CoUninitialize();
    std::cout << "ALL RADIO TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
