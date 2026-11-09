#pragma once
#include "renderer.hpp"
#include "jukebox.hpp"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <atomic>
#include <thread>

namespace command {
class Mp3Reader {
  rdzen::Com<IMFSourceReader> reader;

public:
  int channels = 2, frequency = 48000;
  float duration = 0;
  void open(const std::filesystem::path& path) {
    rdzen::check(MFCreateSourceReaderFromURL(path.c_str(), nullptr, reader.put()),
                 "Open radio MP3");
    rdzen::Com<IMFMediaType> native, pcm;
    rdzen::check(reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, native.put()),
                 "MP3 type");
    UINT32 ch = 2, hz = 48000;
    native->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &ch);
    native->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &hz);
    channels = int(ch);
    frequency = int(hz);
    rdzen::check(MFCreateMediaType(pcm.put()), "PCM type");
    pcm->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pcm->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    pcm->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, ch);
    pcm->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, hz);
    pcm->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    pcm->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, ch * 2);
    pcm->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, hz * ch * 2);
    reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    rdzen::check(reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE),
                 "MP3 audio stream");
    rdzen::check(reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pcm),
                 "Decode MP3 to PCM");
    PROPVARIANT length{};
    duration = 0;
    if (SUCCEEDED(reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION,
                                                   &length)) &&
        length.vt == VT_UI8)
      duration = float(length.uhVal.QuadPart / 10000000.0);
    PropVariantClear(&length);
  }
  bool read(rdzen::Com<IMFSample>& sample) {
    DWORD flags = 0;
    rdzen::check(reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags,
                                    nullptr, sample.put()),
                 "Read MP3 sample");
    return !(flags & MF_SOURCE_READERF_ENDOFSTREAM);
  }
};

class MusicPlayer {
  std::filesystem::path assets;
  SDL_AudioStream* stream = nullptr;
  std::thread worker;
  std::atomic<bool> stop{false}, active{false}, paused{false};
  std::atomic<float> gain{.35f};
  std::atomic<int> requestTrack{0}, currentTrack{0};
  std::atomic<uint32_t> command{0};
  std::atomic<float> position{0}, duration{0}, rms{0}, bass{0}, treble{0};
  std::atomic<bool> playing{false}, failed{false};
  std::atomic<uint64_t> decodedFrames{0};
  int64_t lastRevision = -1;
  bool silent;
  void run() {
    HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HRESULT mf = MFStartup(MF_VERSION);
    if (FAILED(com) || FAILED(mf)) {
      failed = true;
      if (SUCCEEDED(mf))
        MFShutdown();
      if (SUCCEEDED(com))
        CoUninitialize();
      return;
    }
    {
      struct Meter {
        uint64_t frame;
        float rms, bass, high;
      };
      std::array<Meter, 64> meters{};
      size_t head = 0, count = 0;
      uint32_t seen = ~uint32_t(0);
      uint64_t written = 0;
      int index = 0, misses = 0;
      bool opened = false;
      float low = 0;
      Mp3Reader decoder;
      while (!stop) {
        if (!active) {
          SDL_PauseAudioStreamDevice(stream);
          SDL_ClearAudioStream(stream);
          playing = false;
          rms = bass = treble = 0;
          opened = false;
          seen = ~uint32_t(0);
          SDL_Delay(10);
          continue;
        }
        uint32_t next = command.load();
        if (next != seen) {
          seen = next;
          index = requestTrack.load();
          SDL_ClearAudioStream(stream);
          opened = false;
          misses = 0;
          rms = bass = treble = position = 0;
        }
        if (!opened) {
          try {
            decoder.open(assets / RADIO_TRACKS[index].file);
            SDL_AudioSpec spec{SDL_AUDIO_S16, decoder.channels, decoder.frequency};
            if (!SDL_SetAudioStreamFormat(stream, &spec, nullptr))
              throw std::runtime_error(SDL_GetError());
            currentTrack = index;
            duration = decoder.duration;
            written = 0;
            head = count = 0;
            low = 0;
            opened = true;
            failed = false;
            misses = 0;
          } catch (...) {
            failed = true;
            index = (index + 1) % int(RADIO_TRACKS.size());
            if (++misses >= int(RADIO_TRACKS.size()))
              SDL_Delay(250);
            else
              SDL_Delay(10);
            continue;
          }
        }
        if (paused) {
          SDL_PauseAudioStreamDevice(stream);
          playing = false;
          rms = bass = treble = 0;
          SDL_Delay(10);
          continue;
        }
        SDL_SetAudioStreamGain(stream, silent ? 0 : gain.load());
        SDL_ResumeAudioStreamDevice(stream);
        playing = true;
        int bytesPerFrame = decoder.channels * int(sizeof(int16_t));
        int queued = std::max(0, SDL_GetAudioStreamQueued(stream));
        uint64_t played = written - std::min(written, uint64_t(queued / bytesPerFrame));
        position = float(played) / decoder.frequency;
        while (count && meters[head].frame <= played) {
          rms = meters[head].rms;
          bass = meters[head].bass;
          treble = meters[head].high;
          head = (head + 1) % meters.size();
          --count;
        }
        if (queued > decoder.frequency * bytesPerFrame / 10) {
          SDL_Delay(5);
          continue;
        }
        try {
          rdzen::Com<IMFSample> sample;
          bool more = decoder.read(sample);
          if (sample.p) {
            rdzen::Com<IMFMediaBuffer> buffer;
            rdzen::check(sample->ConvertToContiguousBuffer(buffer.put()), "MP3 PCM buffer");
            BYTE* data = nullptr;
            DWORD size = 0;
            rdzen::check(buffer->Lock(&data, nullptr, &size), "MP3 PCM lock");
            auto* pcm = reinterpret_cast<const int16_t*>(data);
            size_t frames = size / bytesPerFrame;
            float coefficient = 1 - std::exp(-2 * rdzen::PI * 140 / decoder.frequency);
            for (size_t first = 0; first < frames; first += 1024) {
              size_t block = std::min(size_t(1024), frames - first);
              double energy = 0, lowEnergy = 0, highEnergy = 0;
              for (size_t i = first; i < first + block; ++i) {
                float mono = 0;
                for (int ch = 0; ch < decoder.channels; ++ch)
                  mono += pcm[i * decoder.channels + ch] / 32768.f;
                mono /= decoder.channels;
                low += coefficient * (mono - low);
                energy += mono * mono;
                lowEnergy += low * low;
                highEnergy += (mono - low) * (mono - low);
              }
              if (count == meters.size()) {
                head = (head + 1) % meters.size();
                --count;
              }
              meters[(head + count++) % meters.size()] = {
                  written + first, std::clamp(float(std::sqrt(energy / block) * 3), 0.f, 1.f),
                  std::clamp(float(std::sqrt(lowEnergy / block) * 7), 0.f, 1.f),
                  std::clamp(float(std::sqrt(highEnergy / block) * 3), 0.f, 1.f)};
            }
            SDL_PutAudioStreamData(stream, data, int(size));
            buffer->Unlock();
            written += frames;
            decodedFrames += frames;
          }
          if (!more) {
            // Drain the tail before beginning the next song; commands still interrupt it.
            while (!stop && active && command == seen && SDL_GetAudioStreamQueued(stream) > 0)
              SDL_Delay(5);
            index = (index + 1) % int(RADIO_TRACKS.size());
            opened = false;
          }
        } catch (...) {
          failed = true;
          index = (index + 1) % int(RADIO_TRACKS.size());
          opened = false;
          SDL_ClearAudioStream(stream);
          SDL_Delay(30);
        }
      }
    }
    MFShutdown();
    CoUninitialize();
  }

public:
  explicit MusicPlayer(std::filesystem::path music, bool testSilent = false)
      : assets(std::move(music)), silent(testSilent) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec spec{SDL_AUDIO_S16, 2, 48000};
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (stream)
      worker = std::thread([this] { run(); });
    else
      failed = true;
  }
  ~MusicPlayer() {
    stop = true;
    if (worker.joinable())
      worker.join();
    if (stream)
      SDL_DestroyAudioStream(stream);
  }
  MusicPlayer(const MusicPlayer&) = delete;
  MusicPlayer& operator=(const MusicPlayer&) = delete;
  void reset() {
    lastRevision = -1;
    active = false;
  }
  void sync(const RadioState& state, bool deployed, bool gamePaused, bool soundEnabled) {
    if (lastRevision != state.revision) {
      requestTrack = state.track;
      ++command;
      lastRevision = state.revision;
    }
    gain = soundEnabled ? state.volume : 0;
    paused = gamePaused || !state.playing;
    active = deployed;
  }
  RadioSnapshot snapshot() const {
    float audible = gain.load() > 0 && playing.load() ? 1.f : 0.f;
    return {currentTrack.load(),  position.load(),       duration.load(),
            rms.load() * audible, bass.load() * audible, treble.load() * audible,
            playing.load(),       failed.load(),         decodedFrames.load()};
  }
};
} // namespace command
