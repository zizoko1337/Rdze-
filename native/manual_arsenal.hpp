#pragma once
#include "gameplay.hpp"

namespace command {
using namespace arsenal;
enum class Gun { Pulse, Crank, Arc, Toxin, Howitzer, Smart, Lance, Rail, Flame, Count };
inline constexpr int GUN_COUNT = int(Gun::Count);
struct PlasmaSpec {
  static constexpr float spreadRadius = 10, hitRadius = 1, minSpeed = 60, maxSpeed = 85;
  static constexpr bool visibleEmitter(int i) { return i < 1 || i > 3; }
  static constexpr int emitterForImpulse(int i) { return visibleEmitter(i) ? i : i + 5; }
};
struct GunSpec {
  const char* name;
  float damage, interval, range, spread, speed;
  Color color;
  Weapon audio;
  const char* detail;
};
inline constexpr GunSpec GUNS[] = {
    {"LASER IMPULSOWY",
     150,
     1,
     500,
     0,
     0,
     {.25f, .9f, 1},
     Laser,
     "150 DMG / 1 s. Precyzyjna wiązka."},
    {"MINIGUN KORBOWY",
     26,
     1.f / 12,
     420,
     .025f,
     210,
     {1, .65f, .18f},
     Minigun,
     "26 DMG / 12 na s. Pociski z rozrzutem."},
    {"TESLA / ŁAŃCUCH",
     340,
     3.2f,
     440,
     0,
     0,
     {.55f, .65f, 1},
     Tesla,
     "340 DMG / 3,2 s. Do 10 celów, skoki 56 m; także od ziemi."},
    {"TOKSYCZNA KATAPULTA",
     65,
     2.8f,
     360,
     0,
     75,
     {.45f, 1, .18f},
     Venom,
     "Plama 14 m / 8 s: 65 DPS i -50% ruchu."},
    {"MOŹDZIERZ BOCZNY",
     1100,
     3.5f,
     460,
     0,
     90,
     {1, .65f, .3f},
     Mortar,
     "1100 DMG / 3,5 s. Wybuch w promieniu 15 m."},
    {"RAKIETY / SMART",
     180,
     .32f,
     500,
     0,
     120,
     {1, .35f, .35f},
     Rocket,
     "Namierzanie 0,5 s. 180 DMG / 0,32 s, wybuch 9 m."},
    {"LANCA PRÓŻNIOWA",
     640,
     2,
     550,
     0,
     0,
     {1, .3f, .7f},
     arsenal::Lance,
     "1,8 s ładowania, 3 s wiązki / 640 DPS; kontakt +200 DMG."},
    {"SNAJPER / RAILGUN",
     750,
     2.4f,
     750,
     0,
     0,
     {.3f, 1, .8f},
     Sniper,
     "750 bazowo / +1% DMG za metr. Przebicie 8 celów."},
    {"MIOTACZ / PIEKŁO",
     240,
     .1f,
     63,
     .5f,
     0,
     {1, .35f, .08f},
     arsenal::Flame,
     "PPM: szeroki 63 m / skupiony 94,5 m, szerokość 5 m. 240 DPS."}};
static_assert(std::size(GUNS) == GUN_COUNT);
struct GunModule {
  Gun kind = Gun::Pulse;
  bool plasma = false;
  bool extremeCooling = false;
  bool uraniumReserve = false;
  bool teslaAmplifier = false, plasmaGlue = false, bouncyMortar = false;
  bool instantLock = false, doubleRangeBonus = false, dualFlame = false;
  float damageMultiplier = 1, rateMultiplier = 1, spreadMultiplier = 1;
  const GunSpec& spec() const { return GUNS[int(kind)]; }
  float interval() const {
    return spec().interval / (kind == Gun::Crank && extremeCooling ? 5.f : 1.f);
  }
  float spread() const {
    return spec().spread * spreadMultiplier * (kind == Gun::Crank && extremeCooling ? 2.f : 1.f);
  }
  const char* name() const {
    return kind == Gun::Pulse && plasma           ? "LASEROWY MOŹDZIERZ"
           : kind == Gun::Crank && extremeCooling ? "MINIGUN / EXTREME COOLING"
                                                  : spec().name;
  }
};
struct ManualProjectile {
  Gun kind;
  Vec3 start, pos, previous, endpoint, velocity;
  float age = 0, duration = 1, damage = 0;
  int target = -1;
  uint32_t generation = 0;
  bool plasma = false;
  int emitter = 0;
  bool glue = false;
  int bounces = 0;
  float arcHeight = 40;
  Vec3 bounceDirection{};
};
struct ToxicPatch {
  Vec3 position;
  float life = 8, tick = 0, damage = 65;
  bool glue = false;
};
struct ToxinCycle {
  float progress, lift, fill, throwAngle;
  bool pouring;
};
} // namespace command
