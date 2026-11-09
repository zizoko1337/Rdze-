#pragma once
#include "expedition.hpp"

namespace command {
enum class Service { None = -1, Brimstone, StunWave, Overload, Count };
struct ServiceInfo {
  const char* name;
  const char* description;
  Color color;
};
inline constexpr ServiceInfo SERVICES[] = {
    {"Brimstone",
     "Po 3 s: sześć uderzeń po 10 000 obrażeń, co 0,5 s, w promieniu 10 m.",
     {1, .32f, .12f}},
    {"Fala ogłuszająca",
     "Po 3 s paraliżuje wszystkich obecnych przeciwników na 5 s, także bossów.",
     {.35f, .65f, 1}},
    {"Przeciążenie",
     "Po 3 s poświęca wieżyczkę: 50 000 obrażeń w promieniu 40 m.",
     {1, .55f, .18f}}};
struct ServiceCast {
  Service type;
  Vec3 position;
  uint32_t towerId = 0;
  float age = 0;
  bool triggered = false;
  int pulses = 0;
};
inline bool insideServiceHex(float x, float z, Vec3 center) {
  float dx = std::abs(x - center.x), dz = std::abs(z - center.z);
  return dx <= SLOT_RADIUS * .866025404f && dz <= SLOT_RADIUS - dx * .577350269f;
}
struct ServiceSystem {
  static constexpr size_t MAX_CASTS = 128;
  std::vector<ServiceCast> casts;
  uint64_t used = 0;
  ServiceSystem() { casts.reserve(MAX_CASTS); }
  bool reserved(uint32_t id) const {
    for (const auto& cast : casts)
      if (cast.type == Service::Overload && cast.towerId == id && !cast.triggered)
        return true;
    return false;
  }
  bool order(Expedition& e, Service type, Vec3 point, uint32_t towerId = 0) {
    if (!e.enabled || !e.miningStarted || e.dead() || e.servicePoints <= 0 ||
        type < Service::Brimstone || type >= Service::Count || casts.size() >= MAX_CASTS ||
        !std::isfinite(point.x) || !std::isfinite(point.z) ||
        std::hypot(point.x, point.z) > WORLD_RADIUS)
      return false;
    if (type == Service::Overload && (!towerId || reserved(towerId)))
      return false;
    casts.push_back({type, point, towerId});
    --e.servicePoints;
    ++used;
    return true;
  }
  void step(Game& g, Expedition& e, float dt) {
    if (g.paused || e.dead() || !e.miningStarted || dt <= 0)
      return;
    for (size_t i = 0; i < casts.size();) {
      auto& cast = casts[i];
      cast.age += dt;
      if (cast.type == Service::Brimstone) {
        while (cast.pulses < 6 && cast.age + .00001f >= 3 + cast.pulses * .5f) {
          ++cast.pulses;
          ++g.brimstonePulses;
          g.blastTargets(cast.position, 10,
                         [&](int id, float) { g.hurt(id, 10000, Laser, false); });
          g.ring(cast.position + Vec3{0, .25f, 0}, {1, .3f, .05f}, 10, .48f);
          g.sparks(cast.position + Vec3{0, 2, 0}, {1, .65f, .2f}, 14, 28);
        }
      } else if (!cast.triggered && cast.age >= 3) {
        cast.triggered = true;
        if (cast.type == Service::StunWave) {
          for (int id : g.active)
            if (g.foes[id].hp > 0)
              g.foes[id].stun = std::max(g.foes[id].stun, 5.f);
          g.ring({0, 1, 0}, {.3f, .65f, 1}, 650, 1.7f);
          g.announce("Fala ogłuszająca: przeciwnicy sparaliżowani na 5 s.");
        } else {
          auto found = std::find_if(g.towers.begin(), g.towers.end(),
                                    [&](const Tower& t) { return t.id == cast.towerId; });
          if (found != g.towers.end()) {
            Weapon type = found->type;
            g.removeTower(found->x, found->z);
            e.releaseTower(type);
            ++g.overloadBlasts;
            g.blastTargets(cast.position, 40,
                           [&](int id, float) { g.hurt(id, 50000, Tesla, false); });
            g.sparks(cast.position + Vec3{0, 4, 0}, {1, .5f, .1f}, 90, 52);
            g.ring(cast.position + Vec3{0, .3f, 0}, {1, .4f, .1f}, 40, 1.5f);
            g.announce("Przeciążenie. Stanowisko jest wolne; technologia pozostaje odblokowana.");
          } else {
            ++e.servicePoints;
            g.announce("Brak wybranej wieżyczki. Punkt usługi zwrócony.");
          }
        }
      }
      float end = cast.type == Service::Brimstone ? 6 : 5;
      if (cast.age >= end) {
        casts[i] = casts.back();
        casts.pop_back();
      } else
        ++i;
    }
  }
};
} // namespace command
