#pragma once
#include "gameplay.hpp"

namespace command {
using namespace arsenal;
enum class Gun { Pulse, Crank, Arc, Toxin, Howitzer, Smart, Lance, Rail, Flame, Count };
inline constexpr int GUN_COUNT = int(Gun::Count);
struct GunSpec {
  const char* name;
  float damage, interval, range, spread, speed;
  Color color;
  Weapon audio;
  const char* detail;
};
inline constexpr GunSpec GUNS[] = {{"LASER IMPULSOWY",
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
                                    "340 DMG / 3,2 s. Do 10 celów, skoki 28 m."},
                                   {"TOKSYCZNY GRANATNIK",
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
                                    "180 DMG / 0,32 s. Autonaprowadzanie, wybuch 9 m."},
                                   {"LANCA PRÓŻNIOWA",
                                    640,
                                    2,
                                    550,
                                    0,
                                    0,
                                    {1, .3f, .7f},
                                    arsenal::Lance,
                                    "Przytrzymaj: 1,8 s ładowania, 3 s wiązki / 640 DPS."},
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
                                    "240 DPS w stożku 63 m. Podpalenie 30 DPS / 3 s."}};
static_assert(std::size(GUNS) == GUN_COUNT);
struct GunModule {
  Gun kind = Gun::Pulse;
  float damageMultiplier = 1, rateMultiplier = 1, spreadMultiplier = 1;
  const GunSpec& spec() const { return GUNS[int(kind)]; }
};
struct ManualProjectile {
  Gun kind;
  Vec3 start, pos, previous, endpoint, velocity;
  float age = 0, duration = 1, damage = 0;
  int target = -1;
  uint32_t generation = 0;
};
struct ToxicPatch {
  Vec3 position;
  float life = 8, tick = 0, damage = 65;
};
} // namespace command
