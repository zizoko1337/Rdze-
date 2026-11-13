#include "command_game.hpp"
#include "manual_sound.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool ok, const char* message) {
  if (!ok)
    throw std::runtime_error(message);
}
double rms(const std::vector<float>& samples) {
  double energy = 0;
  for (auto v : samples) {
    require(std::isfinite(v) && std::abs(v) < .17f, "Unsafe or non-finite audio sample");
    energy += v * v;
  }
  return std::sqrt(energy / samples.size());
}
std::vector<float> samples(ManualSound& synth, int count) {
  std::vector<float> out(count);
  for (auto& v : out)
    v = synth.sample();
  return out;
}
void mount(Session& s, Gun gun) {
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  s.module.kind = gun;
  s.spawnTimer = 100000;
  s.pitch = s.gunPitch = -.1f;
}
int main() {
  try {
    for (int n = 0; n < GUN_COUNT; ++n) {
      ManualFeedback fx;
      Gun gun = Gun(n);
      fx.step(.01f, gun, true, false, false, false);
      fx.shot(gun);
      fx.step(.05f, gun, true, false, false, false);
      require(std::abs(fx.pose.roll) > 0 && std::abs(fx.pose.roll) < .014f &&
                  length(fx.pose.offset) < .08f,
              "Shot tilt missing or too strong");
      for (int i = 0; i < 120; ++i)
        fx.step(1.f / 60, gun, true, false, false, false);
      require(std::abs(fx.pose.roll) < 1e-6f && length(fx.pose.offset) < 1e-6f &&
                  fx.pose.pitch < 1e-6f,
              "Spring recoil does not settle");
    }
    {
      ManualFeedback normal, reserve;
      double normalTravel = 0, reserveTravel = 0;
      for (int n = 0; n < 240; ++n) {
        normal.step(1.f / 60, Gun::Lance, true, true, true, false);
        reserve.step(1.f / 60, Gun::Lance, true, true, true, true);
        require(length(reserve.aimDelta - normal.aimDelta * 3) < 1e-8f,
                "Uranium reserve drift is not exactly triple normal drift");
        normalTravel += length(normal.aimDelta);
        reserveTravel += length(reserve.aimDelta);
      }
      require(normalTravel > .005 && std::abs(reserveTravel / normalTravel - 3) < 1e-5,
              "Lance drift travel ratio is incorrect");
      normal.step(.01f, Gun::Lance, true, false, true, true);
      require(length(normal.aimDelta) == 0, "Reserve continues dragging after release");
      ManualFeedback sniper;
      sniper.step(.01f, Gun::Rail, true, false, false, false);
      sniper.shot(Gun::Rail);
      sniper.step(.05f, Gun::Rail, true, false, false, false);
      require(sniper.pose.pitch > .2f * PI / 180 && sniper.pose.pitch < .35f * PI / 180 &&
                  std::abs(sniper.pose.roll) > 0,
              "Sniper lacks gentle upward recoil and tilt");
      struct Camera {
        Vec3 eye{0, 20, 0}, target{0, 20, 30};
        float roll = 0;
      } camera;
      applyManualCamera(camera, sniper.pose);
      require(normalize(camera.target - camera.eye).y > .003f,
              "Sniper pitch recoil does not raise camera direction");
      for (int n = 0; n < 120; ++n)
        sniper.step(1.f / 60, Gun::Rail, true, false, false, false);
      require(sniper.pose.pitch < 1e-6f, "Sniper upward kick never returns to neutral");
      std::cout << "PASS exact triple uranium drift and small recovering sniper upward kick\n";
    }
    {
      ManualFeedback low, high;
      low.shot(Gun::Pulse);
      high.shot(Gun::Pulse);
      for (int n = 0; n < 6; ++n)
        low.step(1.f / 30, Gun::Pulse, true, false, false, false);
      for (int n = 0; n < 48; ++n)
        high.step(1.f / 240, Gun::Pulse, true, false, false, false);
      require(std::abs(low.pose.roll - high.pose.roll) < 1e-6f &&
                  length(low.pose.offset - high.pose.offset) < 1e-6f,
              "Frame rate changes single-shot kick");
      struct Camera {
        Vec3 eye{0, 20, 0}, target{0, 19, 30};
        float roll = 0;
      } camera;
      Vec3 direction = normalize(camera.target - camera.eye);
      applyManualCamera(camera, low.pose);
      require(length(normalize(camera.target - camera.eye) - direction) < 1e-6f,
              "Visual recoil moves the centre aiming ray");
      std::cout
          << "PASS all shot profiles, settling, frame-rate stability and preserved aiming ray\n";
    }
    {
      Session s;
      mount(s, Gun::Lance);
      s.lanceActive = true;
      s.lanceRemaining = 3;
      float startYaw = s.yaw, startPitch = s.pitch;
      auto randomBefore = s.random.state, worldBefore = s.world.random.state;
      for (int n = 0; n < 60; ++n)
        s.step(1.f / 60, {0, 0, false, true});
      float dx = s.yaw - startYaw, dy = s.pitch - startPitch;
      require(std::hypot(dx, dy) > 1e-5f && std::hypot(dx, dy) < .015f,
              "Lance does not move the real aim gently");
      require(s.gunYaw == s.yaw && s.gunPitch == s.pitch, "Barrel disagrees with drifting aim");
      require(s.random.state == randomBefore && s.world.random.state == worldBefore,
              "Camera feedback consumes combat RNG");
      s.look(-dx, -dy);
      s.updateAim(1);
      require(std::abs(s.yaw - startYaw) < 1e-6f && std::abs(s.pitch - startPitch) < 1e-6f,
              "Mouse cannot correct lance drift");
      auto pose = s.feedback.pose;
      s.world.paused = true;
      s.step(.05f, {0, 0, false, true});
      require(s.feedback.pose.roll == pose.roll, "Pause advances recoil");
      s.world.paused = false;
      s.step(.01f);
      float stoppedYaw = s.yaw;
      for (int n = 0; n < 30; ++n)
        s.step(.01f);
      require(s.yaw == stoppedYaw, "Released lance continues dragging aim");
      s.pitch = s.gunPitch = .65f;
      s.pendingPitch = .03f;
      s.updateAim(.1f);
      require(s.pitch <= .65f, "Queued mouse correction pushes drift beyond elevation limit");
      s.tactical = true;
      s.step(.01f, {0, 0, false, true});
      require(s.feedback.pose.roll == 0 && length(s.feedback.pose.offset) == 0,
              "Tactical camera keeps weapon vibration");
      std::cout
          << "PASS real controllable lance drift, independent RNG, pause, release and tactical\n";
    }
    {
      ManualFeedback fx;
      fx.step(.01f, Gun::Crank, true, true, false, false);
      for (int n = 0; n < 600; ++n) {
        fx.shot(Gun::Crank);
        fx.step(1.f / 120, Gun::Crank, true, true, false, false);
        require(std::abs(fx.pose.roll) < .014f && length(fx.pose.offset) < .085f,
                "Extreme-cooling minigun causes unbounded camera kick");
      }
      fx.step(.01f, Gun::Pulse, true, false, false, false);
      require(length(fx.pose.offset) == 0 && fx.pose.roll == 0, "Module change retains old recoil");
    }
    {
      ManualSound normal, reserve;
      ManualAudioState c;
      c.active = true;
      c.gun = Gun::Lance;
      c.beam = true;
      normal.set(c);
      c.reserve = true;
      reserve.set(c);
      auto a = samples(normal, 48000), b = samples(reserve, 48000);
      require(rms(a) > .006 && rms(a) < .04 && rms(b) > .005, "Lance hum silent or too loud");
      double delta = 0;
      for (size_t n = 0; n < a.size(); ++n)
        delta += (a[n] - b[n]) * (a[n] - b[n]);
      require(std::sqrt(delta / a.size()) > .005, "Reserve sounds identical to normal beam");
      c.beam = c.reserve = false;
      reserve.set(c);
      samples(reserve, 24000);
      require(rms(samples(reserve, 4800)) < 1e-6, "Beam sound sticks after release");
      c.active = false;
      normal.set(c);
      require(rms(samples(normal, 4800)) == 0, "Pause/mute retains lance audio");
      c.active = true;
      c.beam = true;
      normal.set(c);
      require(std::abs(normal.sample()) < .001f, "Resume creates an audio click");
      double early = 0, late = 0;
      for (int n = 0; n < 2000; ++n) {
        float v = normal.sample();
        if (n < 500)
          early += v * v;
        else if (n >= 1500)
          late += v * v;
      }
      require(late > early, "Hum does not fade in smoothly");
      std::cout << "PASS quiet pulsing audio, different reserve interference, release, mute and "
                   "smooth attack\n";
    }
    for (int n = 0; n < GUN_COUNT; ++n) {
      if (Gun(n) == Gun::Lance)
        continue;
      ManualSound synth;
      ManualAudioState c;
      c.active = true;
      c.gun = Gun(n);
      synth.set(c);
      c.shots = 1;
      synth.set(c);
      require(rms(samples(synth, 2400)) > 1e-5, "Missing manual shot mechanical layer");
    }
    {
      // Reviewable mono WAV: charge -> normal beam -> uranium reserve -> release.
      ManualSound synth;
      std::vector<int16_t> pcm;
      for (int n = 0; n < 48000 * 7; ++n) {
        float t = n / 48000.f;
        ManualAudioState c;
        c.gun = Gun::Lance;
        c.active = true;
        c.charge = t < 1.8f ? t / 1.8f : 0;
        c.beam = t >= 1.8f && t < 6.2f;
        c.reserve = t >= 4.8f;
        if (n % 512 == 0)
          synth.set(c);
        pcm.push_back(int16_t(std::clamp(synth.sample() * 32767, -32767.f, 32767.f)));
      }
      std::filesystem::create_directories("artifacts/manual-feedback");
      std::ofstream wav("artifacts/manual-feedback/lance-charge-beam-reserve.wav",
                        std::ios::binary);
      auto u32 = [&](uint32_t x) { wav.write(reinterpret_cast<const char*>(&x), 4); };
      auto u16 = [&](uint16_t x) { wav.write(reinterpret_cast<const char*>(&x), 2); };
      wav.write("RIFF", 4);
      u32(36 + uint32_t(pcm.size() * 2));
      wav.write("WAVEfmt ", 8);
      u32(16);
      u16(1);
      u16(1);
      u32(48000);
      u32(96000);
      u16(2);
      u16(16);
      wav.write("data", 4);
      u32(uint32_t(pcm.size() * 2));
      wav.write(reinterpret_cast<const char*>(pcm.data()), std::streamsize(pcm.size() * 2));
      require(bool(wav), "Cannot write lance sound preview");
    }
    std::cout << "ALL MANUAL FEEDBACK TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
