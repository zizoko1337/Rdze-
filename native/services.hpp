#pragma once
#include "expedition.hpp"
#include "cyber_pigs.hpp"

namespace command {
struct ServiceInfo {
  const char* name;
  const char* description;
  Color color;
};
inline constexpr ServiceInfo SERVICES[] = {
    {"Brimstone",
     "Po 3 s: 25 000 obrażeń łącznie w sześciu impulsach. Promień 10 m; wskazany boss jest śledzony.",
     {1, .32f, .12f}},
    {"Fala ogłuszająca",
     "Po 3 s paraliżuje wszystkich obecnych przeciwników na 5 s, także bossów.",
     {.35f, .65f, 1}},
    {"Cyberświnie",
     "4 drony opuszczają klatkę. Po lądowaniu: 2 s do uwolnienia dwóch świń. Lasery, 1500 obrażeń "
     "przy kontakcie, 5 okrążeń rdzenia.",
     {.25f, .9f, 1}}};
struct ServiceCast {
  Service type;
  Vec3 position;
  int slot = -1;
  float age = 0;
  bool triggered = false;
  int pulses = 0;
  int bossTarget = -1;
  uint32_t bossGeneration = 0;
};
inline bool insideServiceHex(float x, float z, Vec3 center) {
  float dx = std::abs(x - center.x), dz = std::abs(z - center.z);
  return dx <= SLOT_RADIUS * .866025404f && dz <= SLOT_RADIUS - dx * .577350269f;
}
struct ServiceSystem {
  static constexpr size_t MAX_CASTS = 128;
  static constexpr float BRIMSTONE_DAMAGE = 25000;
  static constexpr int BRIMSTONE_PULSES = 6;
  static float brimstonePulse(int pulse) {
    float ordinary = BRIMSTONE_DAMAGE/BRIMSTONE_PULSES;
    return pulse == BRIMSTONE_PULSES-1 ? BRIMSTONE_DAMAGE-ordinary*(BRIMSTONE_PULSES-1) : ordinary;
  }
  static constexpr float LAND_TIME = 6, RELEASE_TIME = 8, DROP_END = 12;
  std::vector<ServiceCast> casts;
  CyberPigSystem cyberPigs;
  uint64_t used = 0;
  ServiceSystem() { casts.reserve(MAX_CASTS); }
  bool reserved(int slot) const {
    for (const auto& cast : casts)
      if (cast.type == Service::CyberPigs && cast.slot == slot && !cast.triggered)
        return true;
    return false;
  }
  bool order(Expedition& e, Service type, Vec3 point, int slot = -1) {
    if (!e.enabled || !e.miningStarted || e.dead() || !e.support.ready(type) ||
        type < Service::Brimstone || type >= Service::Count || casts.size() >= MAX_CASTS ||
        !std::isfinite(point.x) || !std::isfinite(point.z) ||
        std::hypot(point.x, point.z) > WORLD_RADIUS)
      return false;
    if (type == Service::CyberPigs) {
      if (slot < 0 || slot >= SLOT_COUNT || reserved(slot) ||
          std::hypot(point.x - TURRET_SLOTS[slot].x, point.z - TURRET_SLOTS[slot].z) > .1f)
        return false;
      size_t reservedPigs = cyberPigs.pigs.size() + 2;
      for (const auto& cast : casts)
        if (cast.type == Service::CyberPigs && !cast.triggered)
          reservedPigs += 2;
      if (reservedPigs > CyberPigSystem::MAX_PIGS)
        return false;
    }
    casts.push_back({type, point, slot});
    e.support.consume(type);
    ++used;
    return true;
  }
  void step(Game& g, Expedition& e, float dt) {
    if (g.paused || e.dead() || !e.miningStarted || dt <= 0)
      return;
    cyberPigs.step(g, dt);
    for (size_t i = 0; i < casts.size();) {
      auto& cast = casts[i];
      if (cast.type == Service::Brimstone && g.valid(cast.bossTarget,cast.bossGeneration))
        { cast.position = g.targetPoint(cast.bossTarget,cast.position); cast.position.y = 0; }
      cast.age += dt;
      if (cast.type == Service::Brimstone) {
        while (cast.pulses < BRIMSTONE_PULSES && cast.age + .00001f >= 3 + cast.pulses * .5f) {
          float damage = brimstonePulse(cast.pulses);
          ++cast.pulses;
          ++g.brimstonePulses;
          g.blastTargets(cast.position, 10,
                         [&](int id, float) { g.hurt(id, damage, Laser, false, DamageClass::Service); });
          g.ring(cast.position + Vec3{0, .25f, 0}, {1, .3f, .05f}, 10, .48f);
          g.sparks(cast.position + Vec3{0, 2, 0}, {1, .65f, .2f}, 14, 28);
        }
      } else if (cast.type == Service::CyberPigs) {
        if (!cast.triggered && cast.age + .00001f >= RELEASE_TIME) {
          cast.triggered = true;
          cyberPigs.release(cast.position);
          g.ring(cast.position + Vec3{0, .25f, 0}, {.2f, .85f, 1}, 7, .6f);
          g.sparks(cast.position + Vec3{0, 2, 0}, {.8f, .55f, .2f}, 22, 16);
          g.announce("Cyberświnie na wolności. Pięć okrążeń; lasery i taranowanie.");
        }
      } else if (!cast.triggered && cast.age >= 3) {
        cast.triggered = true;
        if (cast.type == Service::StunWave) {
          for (int id : g.active)
            if (g.foes[id].hp > 0)
              g.foes[id].stun = std::max(g.foes[id].stun, 5.f);
          g.ring({0, 1, 0}, {.3f, .65f, 1}, 650, 1.7f);
          g.announce("Fala ogłuszająca: przeciwnicy sparaliżowani na 5 s.");
        }
      }
      float end = cast.type == Service::Brimstone   ? 6
                  : cast.type == Service::CyberPigs ? DROP_END
                                                    : 5;
      if (cast.age >= end) {
        casts[i] = casts.back();
        casts.pop_back();
      } else
        ++i;
    }
  }
};
} // namespace command
