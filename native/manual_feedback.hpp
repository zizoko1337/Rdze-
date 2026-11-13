#pragma once
#include "manual_arsenal.hpp"

namespace command {
struct ManualAudioState {
  Gun gun = Gun::Pulse;
  uint64_t shots = 0;
  float charge = 0, minigun = 0, flame = 0;
  bool active = false, beam = false, reserve = false;
};
struct ManualCameraPose {
  Vec3 offset{}; // Local right, up, forward, in metres.
  float roll = 0, pitch = 0;
};
template <class Camera>
inline void applyManualCamera(Camera& camera, const ManualCameraPose& pose) {
  Vec3 forward = normalize(camera.target - camera.eye);
  Vec3 right = normalize(cross({0, 1, 0}, forward)), up = cross(forward, right);
  Vec3 offset = right * pose.offset.x + up * pose.offset.y + forward * pose.offset.z;
  camera.eye = camera.eye + offset;
  camera.target = camera.target + offset;
  if (pose.pitch != 0) {
    float distance = length(camera.target - camera.eye);
    camera.target =
        camera.eye + (forward * std::cos(pose.pitch) + up * std::sin(pose.pitch)) * distance;
  }
  camera.roll = pose.roll;
}
class ManualFeedback {
  Random random{0x2ac87e13}; // Never advance combat/reward RNG for camera effects.
  Vec3 spring{}, velocity{}, drift{}, driftTarget{};
  float clock = 0, driftClock = 0, vibration = 0, reserveMix = 0;
  float pitchSpring = 0, pitchVelocity = 0;
  Gun previousGun = Gun::Pulse;

public:
  bool triggerHeld = false;
  ManualCameraPose pose{};
  Vec3 aimDelta{};
  void clear() {
    spring = velocity = drift = driftTarget = {};
    pose = {};
    aimDelta = {};
    driftClock = vibration = reserveMix = 0;
    pitchSpring = pitchVelocity = 0;
    triggerHeld = false;
  }
  void shot(Gun gun, bool plasma = false) {
    // Peak roll in degrees, backward movement and upward movement. All remain subtle.
    constexpr Vec3 kick[] = {{.22f, .012f, .004f}, {.025f, .002f, .0007f}, {.32f, .018f, .006f},
                             {.36f, .022f, .007f}, {.48f, .032f, .010f},   {.085f, .006f, .002f},
                             {.18f, .012f, .004f}, {.42f, .027f, .008f},   {.012f, .001f, .0003f}};
    Vec3 p = kick[int(gun)];
    if (plasma)
      p = {.38f, .025f, .008f};
    float side = random.unit() < .5f ? -1.f : 1.f;
    constexpr float impulse = 18 * 2.718281828f;
    if (gun == Gun::Rail)
      pitchVelocity += .28f * PI / 180 * impulse;
    velocity = velocity + Vec3{p.x * PI / 180 * side, p.y, p.z} * impulse;
    velocity.x = std::clamp(velocity.x, -.5f, .5f);
    velocity.y = std::min(velocity.y, 2.f);
    velocity.z = std::min(velocity.z, .7f);
  }
  void step(float dt, Gun gun, bool mounted, bool held, bool beam, bool reserve) {
    aimDelta = {};
    if (!mounted || gun != previousGun) {
      clear();
      previousGun = gun;
      if (!mounted)
        return;
    }
    if (!(dt > 0))
      return;
    triggerHeld = held;
    clock += dt;
    // Exact critically damped spring, stable across simulation rates.
    float decay = std::exp(-18 * dt);
    Vec3 c = velocity + spring * 18;
    spring = (spring + c * dt) * decay;
    velocity = (velocity - c * (18 * dt)) * decay;
    float pitchC = pitchVelocity + pitchSpring * 18;
    pitchSpring = (pitchSpring + pitchC * dt) * decay;
    pitchVelocity = (pitchVelocity - pitchC * (18 * dt)) * decay;
    bool lance = gun == Gun::Lance && held && beam;
    float target = held && gun == Gun::Crank   ? 1.f
                   : lance                     ? .7f
                   : held && gun == Gun::Flame ? .2f
                                               : 0;
    vibration += (target - vibration) * (-std::expm1(-dt * 16));
    reserveMix += (float(lance && reserve) - reserveMix) * (-std::expm1(-dt * 10));
    if (lance) {
      driftClock -= dt;
      if (driftClock <= 0) {
        float angle = random.unit() * 2 * PI;
        float speed = (.275f + random.unit() * .20f) * PI / 180;
        driftTarget = {std::cos(angle) * speed, std::sin(angle) * speed, 0};
        driftClock = .35f + random.unit() * .4f;
      }
      float blend = -std::expm1(-dt * 7);
      drift = drift + (driftTarget - drift) * blend;
      aimDelta = drift * (dt * (reserve ? 3.f : 1.f));
    } else {
      drift = driftTarget = {};
      driftClock = 0;
    }
    float roughness = vibration * (1 + .35f * reserveMix);
    float a = clock * 2 * PI;
    pose.roll = std::clamp(spring.x, -.012f, .012f) +
                (.00038f * std::sin(a * 13.7f) + .00022f * std::sin(a * 19.1f)) * roughness;
    pose.pitch = std::clamp(pitchSpring, 0.f, .008f);
    pose.offset = {(.0025f * std::sin(a * 17.3f) + .0015f * std::sin(a * 23.7f)) * roughness,
                   std::clamp(spring.z, 0.f, .025f) + .003f * std::sin(a * 21.1f) * roughness,
                   -std::clamp(spring.y, 0.f, .075f) + .002f * std::sin(a * 15.7f) * roughness};
  }
};
} // namespace command
