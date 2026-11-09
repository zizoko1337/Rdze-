#pragma once
#include "simulation.hpp"
#include <functional>
#include <string>

namespace arsenal {
using namespace rdzen;
constexpr float CORE_RADIUS = 14, CORE_HEIGHT = 18, BUILD_RADIUS = 240, WORLD_RADIUS = 3200;
constexpr int MAX_FOES = 20000, MAX_TOWERS = 512, MAX_SHOTS = 100000;
enum Weapon {
  Laser,
  Minigun,
  Tesla,
  Sniper,
  Lance,
  Venom,
  Rocket,
  Mortar,
  Flame,
  WeaponCount, // Number of damaging weapons; audio/statistics and the old arsenal use these nine.
  RateRelay = WeaponCount,
  RangeRelay,
  FrostTower,
  TowerTypeCount
};
inline constexpr bool isSupport(Weapon type) {
  return type >= RateRelay && type < TowerTypeCount;
}
inline constexpr float SUPPORT_SPACING = 21;
inline bool supportNeighbor(float ax, float az, float bx, float bz) {
  float dx = bx - ax, dz = bz - az;
  constexpr float half = SUPPORT_SPACING * .5f, diagonal = SUPPORT_SPACING * .866025404f;
  for (auto offset :
       {Vec3{SUPPORT_SPACING, 0, 0}, Vec3{half, 0, diagonal}, Vec3{-half, 0, diagonal},
        Vec3{-SUPPORT_SPACING, 0, 0}, Vec3{-half, 0, -diagonal}, Vec3{half, 0, -diagonal}})
    if (std::abs(dx - offset.x) < .05f && std::abs(dz - offset.z) < .05f)
      return true;
  return false;
}
enum Species {
  Crawler,
  Runner,
  Brute,
  Spitter,
  Floater,
  Saboteur,
  SiegeSpitter,
  Titan,
  Queen,
  Brain,
  Wyrm,
  Slime,
  Devourer,
  SiegeBoss,
  SpeciesCount
};
constexpr float SABOTAGE_TIME = 3, BRAIN_RADIUS = SUPPORT_SPACING * 2;
constexpr float BRAIN_INTERVAL = 6, BRAIN_DISABLE = 4, SIEGE_RANGE = SUPPORT_SPACING * 3;
constexpr float WYRM_FORWARD_SPEED = 3.2f, WYRM_LATERAL_SPEED = 32, WYRM_SWAY_FREQUENCY = 2.4f;
struct Color {
  float r, g, b;
};
struct WeaponInfo {
  const char* name;
  const char* role;
  const char* description;
  float range, cooldown, damage, height;
  Color color;
};
inline constexpr WeaponInfo WEAPONS[] = {
    {"Laser",
     "PRECYZYJNY IMPULS",
     "Pewne trafienie. Obracana głowica i puls emitera.",
     65,
     .42f,
     30,
     5,
     {1, .13f, .2f}},
    {"Minigun",
     "OGIEŃ ZAPOROWY",
     "Rozkręcany bęben sześciu luf. Krótkie, gęste serie.",
     54,
     .065f,
     8,
     4.8f,
     {1, .69f, .22f}},
    {"Tesla",
     "WYŁADOWANIE OBSZAROWE",
     "Pioruny trafiają wszystkich w zasięgu. Długie odnowienie.",
     55,
     5.5f,
     145,
     7,
     {.4f, .65f, 1}},
    {"Snajper",
     "DALEKI ZASIĘG",
     "Ciężki strzał, odrzut lufy i powolny powrót zamka.",
     180,
     3.5f,
     380,
     6,
     {.65f, .95f, 1}},
    {"Lanca",
     "ŁADOWANY LASER CIĄGŁY",
     "Ładuje 6 s, potem utrzymuje wiązkę przez 3 s.",
     110,
     2,
     185,
     6.5f,
     {1, .3f, .08f}},
    {"Toksyna",
     "TRUCIZNA / OBSZAR",
     "Pociski tworzą chmury. Zatrucie działa przez 6 s.",
     78,
     1.6f,
     18,
     5,
     {.45f, 1, .12f}},
    {"Rakiety",
     "NAPROWADZANIE",
     "Ruchoma wyrzutnia. Rakiety wybuchają w promieniu 10 m.",
     125,
     1.35f,
     140,
     6,
     {1, .48f, .15f}},
    {"Moździerz",
     "OSTRZAŁ BALISTYCZNY",
     "Wysoki łuk lotu. Ciężka eksplozja w promieniu 16 m.",
     170,
     3.7f,
     250,
     4.5f,
     {1, .78f, .4f}},
    {"Palnik",
     "STOŻEK OGNIA",
     "Ciągły płomień podpala wszystkich przed dyszą.",
     38,
     .1f,
     72,
     4.5f,
     {1, .24f, .025f}},
    {"Przekaźnik tempa",
     "WSPARCIE / SĄSIEDNIE HEKSY",
     "+20% szybkostrzelności maksymalnie 6 sąsiednich wieżyczek bojowych.",
     SUPPORT_SPACING,
     3,
     0,
     5.5f,
     {1, .7f, .2f}},
    {"Przekaźnik zasięgu",
     "WSPARCIE / SĄSIEDNIE HEKSY",
     "+20% zasięgu maksymalnie 6 sąsiednich wieżyczek bojowych.",
     SUPPORT_SPACING,
     3,
     0,
     6,
     {.4f, 1, .6f}},
    {"Generator śnieżycy",
     "WSPARCIE / SPOWOLNIENIE",
     "Spowalnia przeciwników o 50% w promieniu 40 m. Śnieżyca co 3 s.",
     40,
     3,
     0,
     5.5f,
     {.45f, .8f, 1}}};
static_assert(std::size(WEAPONS) == TowerTypeCount);
struct SpeciesInfo {
  const char* name;
  const char* texture;
  float hp, speed, size, altitude;
};
inline constexpr SpeciesInfo SPECIES[] = {
    {"Pełzacz", "alien-beast.png", 100, 9, 3.8f, 0},
    {"Łowca", "alien-runner.png", 75, 17, 4.5f, 0},
    {"Taran", "alien-brute.png", 650, 6, 7, 0},
    {"Plujec", "alien-spitter.png", 230, 7.5f, 5.2f, 0},
    {"Widmo", "alien-floater.png", 155, 13, 5.5f, 5},
    {"Sabotażysta", "alien-saboteur.png", 1600, 5, 7.5f, 0},
    {"Plujec oblężniczy", "alien-siege-spitter.png", 900, 3, 8, 0},
    {"TYTAN BAZALTU", "boss-titan.png", 18000, 3.4f, 25, 0},
    {"KRÓLOWA ROJU", "boss-queen.png", 13500, 4.4f, 22, 2},
    {"NADUMYSŁ", "boss-brain.png", 16000, 2.6f, 20, 3},
    {"ŻMIJ SEGMENTOWY", "boss-wyrm.png", 14000, WYRM_FORWARD_SPEED, 13, 0},
    {"ŚLUZOWIEC", "boss-slime.png", 8000, 2.8f, 22, 0},
    {"POŻERACZ", "boss-devourer.png", 12000, 3, 23, 0},
    {"PLUJEC ALFA", "boss-siege.png", 15000, 2.8f, 24, 0}};
static_assert(std::size(SPECIES) == SpeciesCount);
struct Foe {
  float x = 0, z = 0, hp = 0, maxHp = 0, speed = 0, size = 0, altitude = 0, phase = 0;
  float poison = 0, poisonDps = 0, burn = 0, burnDps = 12, flash = 0, ability = 0, stun = 0;
  float toxicSlow = 0, glued = 0;
  float sabotage = 0, towerSearch = 0;
  uint32_t towerTarget = 0;
  Species type = Crawler;
  uint32_t generation = 0;
  bool elite = false;
  uint32_t slimeFamily = 0;
  uint8_t slimeStage = 0, feedUsed = 0, feedPending = 0;
  float absorbClock = 0;
  int summoner = -1;
  uint32_t summonerGeneration = 0;
};
struct FoeDeath {
  Vec3 position;
  Species type;
  bool elite;
  bool reward = true;
};
struct Tower {
  Weapon type = Laser;
  float x = 0, z = 0, angle = 0, cooldown = 0, phase = 0, spin = 0, spinSpeed = 0, recoil = 0,
        pulse = 0, effectClock = 0, searchClock = 0;
  int target = -1, state = 0;
  uint32_t generation = 0, id = 0;
  uint64_t shots = 0;
  float localRate = 1, localRange = 1;
  float disabledUntil = 0;
  float amplifiedUntil = 0;
};
struct SupportLink {
  uint16_t source, target;
};
struct Ordnance {
  Vec3 start, pos, previous, endpoint;
  int target = -1;
  uint32_t generation = 0;
  Weapon type = Rocket;
  float age = 0, duration = 1, speed = 40, damage = 0, radius = 0, trailClock = 0;
};
struct BeamFx {
  Vec3 a, b;
  Color color;
  float width, life, maxLife;
};
struct Particle {
  Vec3 pos, velocity;
  Color color;
  float size, life, maxLife;
  int style = 0;
};
struct GasZone {
  float x, z, radius, life, tick;
  float poisonDps = 22;
};
struct RingFx {
  Vec3 pos;
  Color color;
  float radius, life, maxLife;
};
struct EnemyShot {
  Vec3 start;
  float age, duration, damage;
  Vec3 position(float t) const {
    return start * (1 - t) + Vec3{0, CORE_HEIGHT * .5f, 0} * t + Vec3{0, std::sin(t * PI) * 12, 0};
  }
};

class SpatialGrid {
  static constexpr int SIDE = 128;
  static constexpr float CELL = 16, ORIGIN = -1024;
  std::vector<int> heads = std::vector<int>(SIDE * SIDE, -1), next = std::vector<int>(MAX_FOES, -1);
  static int cell(float p) { return std::clamp(int(std::floor((p - ORIGIN) / CELL)), 0, SIDE - 1); }

public:
  void rebuild(const std::vector<Foe>& foes, const std::vector<int>& active) {
    std::fill(heads.begin(), heads.end(), -1);
    for (int i : active) {
      const auto& e = foes[i];
      if (e.hp <= 0)
        continue;
      int c = cell(e.x) + cell(e.z) * SIDE;
      next[i] = heads[c];
      heads[c] = i;
    }
  }
  template <class Fn>
  void radius(float x, float z, float range, const std::vector<Foe>& foes, Fn&& fn) const {
    float rr = range * range;
    for (int cz = cell(z - range); cz <= cell(z + range); ++cz)
      for (int cx = cell(x - range); cx <= cell(x + range); ++cx) {
        float lx = ORIGIN + cx * CELL, lz = ORIGIN + cz * CELL;
        float dx = std::max({lx - x, 0.f, x - lx - CELL}),
              dz = std::max({lz - z, 0.f, z - lz - CELL});
        if (dx * dx + dz * dz > rr)
          continue;
        for (int i = heads[cx + cz * SIDE]; i >= 0; i = next[i]) {
          const auto& e = foes[i];
          float ex = e.x - x, ez = e.z - z;
          if (e.hp > 0 && ex * ex + ez * ez <= rr)
            fn(i);
        }
      }
  }
  int nearest(float x, float z, float range, const std::vector<Foe>& foes) const {
    int target = -1;
    float best = range * range;
    radius(x, z, range, foes, [&](int i) {
      float dx = foes[i].x - x, dz = foes[i].z - z, d = dx * dx + dz * dz;
      if (d < best || (d == best && (target < 0 || i < target))) {
        best = d;
        target = i;
      }
    });
    return target;
  }
};

class Game {
public:
  std::vector<Foe> foes = std::vector<Foe>(MAX_FOES);
  std::vector<int> active, freeIds;
  std::vector<Tower> towers;
  std::vector<Ordnance> shots;
  std::vector<BeamFx> beams;
  std::vector<Particle> particles;
  std::vector<GasZone> gas;
  std::vector<RingFx> rings;
  std::vector<FoeDeath> deaths;
  struct WormBody {
    int head;
    uint32_t generation;
    std::array<Vec3, 8> segments;
  };
  struct SlimeFamily {
    uint32_t id;
    int alive;
  };
  std::vector<WormBody> worms;
  std::vector<SlimeFamily> slimeFamilies;
  uint32_t nextSlimeFamily = 0;
  bool recordDeaths = false, expeditionRules = false;
  std::vector<SupportLink> supportLinks;
  // Static aura buckets are rebuilt only when installations change, never per enemy/frame.
  std::array<std::vector<uint16_t>, 64 * 64> frostCells;
  bool hasFrostSupports = false;
  SpatialGrid grid;
  Random random, fxRandom;
  float time = 0, nextWave = 25, spawnClock = 0, coreDamage = 0;
  int wave = 1, pending = 0, kills = 0, bossKills = 0, bossSerial = 0;
  bool autoWaves = true, paused = false;
  float towerDamage = 1, towerRate = 1, towerRange = 1;
  bool frost = false;
  int focusTarget = -1;
  uint32_t focusGeneration = 0;
  float focusTime = 0, focusDamageBonus = 0;
  uint32_t nextTower = 1;
  uint64_t brimstonePulses = 0, overloadBlasts = 0;
  std::vector<EnemyShot> enemyShots;
  std::vector<Weapon> destroyedTowers;
  std::array<uint64_t, WeaponCount> fired{}, hits{};
  std::array<double, WeaponCount> damageDone{};
  std::string announcement = "Arsenał gotowy. Budowanie bez kosztów.";
  float announcementUntil = 7;
  Game(bool showcase = true) {
    active.reserve(MAX_FOES);
    freeIds.reserve(MAX_FOES);
    towers.reserve(MAX_TOWERS);
    shots.reserve(MAX_SHOTS);
    beams.reserve(12000);
    particles.reserve(18000);
    gas.reserve(512);
    rings.reserve(1024);
    enemyShots.reserve(512);
    destroyedTowers.reserve(MAX_TOWERS);
    fxRandom.state = 9187;
    for (int i = MAX_FOES - 1; i >= 0; --i)
      freeIds.push_back(i);
    if (showcase) {
      for (int i = 0; i < WeaponCount; ++i) {
        float a = i * 2 * PI / float(WeaponCount);
        addTower(Weapon(i), std::sin(a) * 43, std::cos(a) * 43);
      }
      for (int i = 0; i < 100; ++i) {
        float a = random.unit() * 2 * PI, r = 120 + random.unit() * 120;
        spawn(Species(i % 5), std::sin(a) * r, std::cos(a) * r);
      }
    }
    grid.rebuild(foes, active);
  }
  void announce(const std::string& text) {
    announcement = text;
    announcementUntil = time + 5;
  }
  Vec3 targetPoint(int i) const {
    const auto& e = foes[i];
    return {e.x, e.altitude + e.size * .47f, e.z};
  }
  bool valid(int index, uint32_t generation) const {
    return index >= 0 && index < MAX_FOES && foes[index].hp > 0 &&
           foes[index].generation == generation;
  }
  WeaponInfo weaponInfo(Weapon type) const {
    WeaponInfo info = WEAPONS[type];
    if (isSupport(type))
      return info;
    info.damage *= towerDamage;
    info.cooldown /= towerRate;
    info.range *= towerRange;
    return info;
  }
  WeaponInfo weaponInfo(const Tower& tower) const {
    auto info = weaponInfo(tower.type);
    info.cooldown /= tower.localRate;
    if (tower.amplifiedUntil > time)
      info.cooldown *= .5f;
    info.range *= tower.localRange;
    return info;
  }
  WeaponInfo previewWeaponInfo(Weapon type, float x, float z) const {
    Tower preview;
    preview.type = type;
    if (!isSupport(type))
      for (const auto& source : towers)
        if (source.disabledUntil <= time && supportNeighbor(source.x, source.z, x, z)) {
          if (source.type == RateRelay)
            preview.localRate = 1.2f;
          if (source.type == RangeRelay)
            preview.localRange = 1.2f;
        }
    return weaponInfo(preview);
  }
  void rebuildSupport() {
    supportLinks.clear();
    hasFrostSupports = false;
    for (auto& cell : frostCells)
      cell.clear();
    for (auto& tower : towers)
      tower.localRate = tower.localRange = 1;
    for (size_t i = 0; i < towers.size(); ++i) {
      const auto& source = towers[i];
      if (source.disabledUntil > time)
        continue;
      if (source.type == FrostTower) {
        hasFrostSupports = true;
        int x0 = std::clamp(int(std::floor((source.x - 40 + 512) / 16)), 0, 63);
        int x1 = std::clamp(int(std::floor((source.x + 40 + 512) / 16)), 0, 63);
        int z0 = std::clamp(int(std::floor((source.z - 40 + 512) / 16)), 0, 63);
        int z1 = std::clamp(int(std::floor((source.z + 40 + 512) / 16)), 0, 63);
        for (int z = z0; z <= z1; ++z)
          for (int x = x0; x <= x1; ++x)
            frostCells[x + z * 64].push_back(uint16_t(i));
      } else if (source.type == RateRelay || source.type == RangeRelay) {
        for (size_t j = 0; j < towers.size(); ++j) {
          auto& target = towers[j];
          if (!isSupport(target.type) && supportNeighbor(source.x, source.z, target.x, target.z)) {
            if (source.type == RateRelay)
              target.localRate = 1.2f;
            else
              target.localRange = 1.2f;
            supportLinks.push_back({uint16_t(i), uint16_t(j)});
          }
        }
      }
    }
  }
  float movementSlowAt(float x, float z) const {
    float slow = frost ? 1 - .4f * std::clamp(1 - std::hypot(x, z) / 85, 0.f, 1.f) : 1;
    if (!hasFrostSupports)
      return slow;
    int cx = int(std::floor((x + 512) / 16)), cz = int(std::floor((z + 512) / 16));
    if (cx >= 0 && cx < 64 && cz >= 0 && cz < 64)
      for (uint16_t index : frostCells[cx + cz * 64]) {
        const auto& source = towers[index];
        if (source.disabledUntil > time)
          continue;
        float dx = source.x - x, dz = source.z - z;
        if (dx * dx + dz * dz <= 40 * 40)
          return slow * .5f;
      }
    return slow;
  }
  bool inRange(const Tower& t) const {
    if (!valid(t.target, t.generation))
      return false;
    auto& e = foes[t.target];
    float dx = e.x - t.x, dz = e.z - t.z;
    float range = weaponInfo(t).range;
    return dx * dx + dz * dz <= range * range;
  }
  int spawn(Species species, float x, float z, uint32_t family = 0, uint8_t slimeStage = 0,
            float inheritedHp = 0) {
    if (freeIds.empty())
      return -1;
    int i = freeIds.back();
    freeIds.pop_back();
    uint32_t gen = foes[i].generation + 1;
    const auto& s = SPECIES[species];
    float scale = 1 + std::max(0, wave - 1) * .12f;
    foes[i] = {};
    auto& e = foes[i];
    e.x = x;
    e.z = z;
    e.hp = e.maxHp = s.hp * scale;
    e.speed = s.speed * (1 + std::min(wave, 30) * .012f);
    e.size = s.size;
    e.altitude = s.altitude;
    e.type = species;
    e.elite = species == Saboteur;
    e.generation = gen;
    e.phase = random.unit() * 2 * PI;
    e.ability = 4 + random.unit() * 4;
    if (species == SiegeBoss)
      e.ability = 5;
    if (species == Slime) {
      e.slimeFamily = family ? family : ++nextSlimeFamily;
      e.slimeStage = slimeStage;
      e.hp = e.maxHp = inheritedHp > 0 ? inheritedHp : SPECIES[Slime].hp;
      e.size *= std::pow(.7f, float(slimeStage));
      e.speed *= std::pow(1.3f, float(slimeStage));
      auto f = std::find_if(slimeFamilies.begin(), slimeFamilies.end(),
                            [&](const SlimeFamily& a) { return a.id == e.slimeFamily; });
      if (f == slimeFamilies.end())
        slimeFamilies.push_back({e.slimeFamily, 1});
      else
        ++f->alive;
    }
    if (species == Wyrm) {
      WormBody body{i, gen, {}};
      Vec3 away = normalize(Vec3{x, 0, z});
      if (length(away) < .1f)
        away = {0, 0, 1};
      for (int n = 0; n < 8; ++n)
        body.segments[n] = Vec3{x, 0, z} + away * (7.f * (n + 1));
      worms.push_back(body);
    }
    active.push_back(i);
    return i;
  }
  int boss() {
    constexpr Species bosses[] = {Titan, Queen, Brain, Wyrm, Slime, Devourer, SiegeBoss};
    Species type = bosses[bossSerial++ % std::size(bosses)];
    float a = random.unit() * 2 * PI;
    int id = spawn(type, std::sin(a) * 225, std::cos(a) * 225);
    if (id >= 0)
      announce(std::string("WYKRYTO BOSSA: ") + SPECIES[type].name);
    return id;
  }
  void horde(int count = 250) {
    for (int i = 0; i < count; ++i) {
      float a = random.unit() * 2 * PI, r = 135 + random.unit() * 140;
      spawn(Species(random.next() % 5), std::sin(a) * r, std::cos(a) * r);
    }
    announce("Dodatkowa horda na poligonie.");
  }
  void advanceWave() {
    ++wave;
    pending = std::min(5000, pending + 35 + wave * 20);
    spawnClock = 0;
    nextWave = time + 35;
    announce("Fala " + std::to_string(wave) + " — planeta odpowiada.");
    if (wave % 4 == 0)
      boss();
  }
  const char* canBuild(float x, float z) const {
    if (!std::isfinite(x) || !std::isfinite(z))
      return "Wskaż powierzchnię";
    float r = std::hypot(x, z);
    if (r < CORE_RADIUS + 7)
      return "Zbyt blisko rdzenia";
    if (r > BUILD_RADIUS)
      return "Poza siecią zasilania";
    if (towers.size() >= MAX_TOWERS)
      return "Limit 512 wieżyczek";
    for (auto& t : towers)
      if ((t.x - x) * (t.x - x) + (t.z - z) * (t.z - z) < 36)
        return "Zbyt blisko innej wieżyczki";
    return nullptr;
  }
  bool addTower(Weapon type, float x, float z) {
    if (canBuild(x, z))
      return false;
    Tower t;
    t.type = type;
    t.x = x;
    t.z = z;
    t.id = nextTower++;
    t.angle = std::atan2(x, z);
    towers.push_back(t);
    rebuildSupport();
    return true;
  }
  bool removeTower(float x, float z) {
    for (size_t i = 0; i < towers.size(); ++i)
      if (std::hypot(towers[i].x - x, towers[i].z - z) < 4) {
        towers.erase(towers.begin() + i);
        rebuildSupport();
        return true;
      }
    return false;
  }
  void beam(Vec3 a, Vec3 b, Color color, float width, float duration) {
    if (beams.size() < 12000)
      beams.push_back({a, b, color, width, duration, duration});
  }
  void particle(Vec3 p, Vec3 v, Color color, float size, float duration, int style = 0) {
    if (particles.size() < 18000)
      particles.push_back({p, v, color, size, duration, duration, style});
  }
  void ring(Vec3 p, Color color, float radius, float duration) {
    if (rings.size() < 1024)
      rings.push_back({p, color, radius, duration, duration});
  }
  void sparks(Vec3 p, Color color, int count = 8, float force = 10) {
    for (int i = 0; i < count; ++i) {
      Vec3 v{(fxRandom.unit() - .5f) * force, fxRandom.unit() * force,
             (fxRandom.unit() - .5f) * force};
      particle(p, v, color, .25f + fxRandom.unit() * .7f, .25f + fxRandom.unit() * .5f);
    }
  }
  float hurt(int i, float amount, Weapon weapon, bool trackWeapon = true) {
    if (i < 0 || foes[i].hp <= 0)
      return 0;
    auto& e = foes[i];
    if (focusTime > 0 && i == focusTarget && e.generation == focusGeneration)
      amount *= 1 + focusDamageBonus;
    float actual = std::min(e.hp, amount);
    e.hp -= amount;
    if (e.type == Devourer && e.hp > 0) {
      for (int phase = 0; phase < 3; ++phase) {
        uint8_t bit = uint8_t(1u << phase);
        if (e.hp <= e.maxHp * (.75f - phase * .25f) && !(e.feedUsed & bit)) {
          e.feedUsed |= bit;
          e.feedPending |= bit;
        }
      }
    }
    e.flash = .09f;
    if (trackWeapon) {
      damageDone[weapon] += actual;
      ++hits[weapon];
    }
    if (e.hp <= 0) {
      ++kills;
      bool reward = true;
      if (e.type == Slime) {
        if (e.slimeStage < 2) {
          for (float side : {-1.f, 1.f})
            spawn(Slime, e.x + side * e.size * .35f, e.z + side * 2.f, e.slimeFamily,
                  uint8_t(e.slimeStage + 1), e.maxHp * .5f);
        }
        auto family = std::find_if(slimeFamilies.begin(), slimeFamilies.end(),
                                  [&](const SlimeFamily& a) { return a.id == e.slimeFamily; });
        if (family != slimeFamilies.end()) {
          reward = --family->alive == 0;
          if (reward)
            slimeFamilies.erase(family);
        }
      }
      if (recordDeaths && deaths.size() < MAX_FOES)
        deaths.push_back({{e.x, .6f, e.z}, e.type, e.elite, reward});
      bool isBoss = e.type >= Titan;
      if (isBoss && reward) {
        ++bossKills;
        announce(std::string("Pokonano: ") + SPECIES[e.type].name);
        ring({e.x, .15f, e.z}, {1, .6f, .2f}, 35, 1.4f);
      }
      sparks(targetPoint(i), isBoss ? Color{1, .5f, .1f} : Color{.9f, .16f, .27f}, isBoss ? 55 : 5,
             isBoss ? 28 : 8);
    }
    return actual;
  }
  template <class Fn> void blastTargets(Vec3 p, float radius, Fn&& fn) {
    grid.radius(p.x, p.z, radius, foes, [&](int i) {
      fn(i, std::hypot(foes[i].x - p.x, foes[i].z - p.z));
    });
    for (const auto& body : worms) {
      if (!valid(body.head, body.generation))
        continue;
      const auto& head = foes[body.head];
      if (std::hypot(head.x - p.x, head.z - p.z) <= radius)
        continue; // Head was already visited; one hit per shared health pool.
      float nearest = 1e30f;
      for (const auto& segment : body.segments)
        nearest = std::min(nearest, std::hypot(segment.x - p.x, segment.z - p.z));
      if (nearest <= radius)
        fn(body.head, nearest);
    }
  }
  void blast(Vec3 p, float radius, float damage, Weapon type) {
    blastTargets(p, radius, [&](int i, float d) {
      hurt(i, damage * (1 - .45f * d / radius), type);
    });
    auto color = WEAPONS[type].color;
    ring({p.x, .2f, p.z}, color, radius, .7f);
    sparks(p, color, type == Mortar ? 30 : 16, 22);
    particle(p, {0, 2, 0}, color, radius * .7f, .4f);
  }
  void launch(Tower& t) {
    if (shots.size() >= MAX_SHOTS)
      return;
    auto w = weaponInfo(t);
    Vec3 origin{t.x + std::sin(t.angle) * 2.5f, w.height, t.z + std::cos(t.angle) * 2.5f};
    Ordnance p;
    p.type = t.type;
    p.start = p.pos = p.previous = origin;
    p.target = t.target;
    p.generation = t.generation;
    p.endpoint = targetPoint(t.target);
    p.damage = w.damage;
    p.radius = t.type == Mortar ? 16 : t.type == Rocket ? 10 : 9;
    p.speed = t.type == Rocket ? 55 : 30;
    p.duration = t.type == Mortar ? 1.3f + length(p.endpoint - origin) / 65 : 1;
    shots.push_back(p);
  }
  void lightning(Vec3 from, Vec3 to, int seed) {
    Vec3 last = from;
    for (int s = 1; s <= 6; ++s) {
      float f = s / 6.f;
      Vec3 p = from + (to - from) * f;
      if (s < 6) {
        float k = std::sin(seed * 5.7f + s * 17.1f + time * 7);
        p = p + Vec3{k * 2.5f, std::cos(seed + s * 8.1f) * 2.5f, -k * 1.5f};
      }
      beam(last, p, {.36f, .6f, 1}, .2f, .27f);
      last = p;
    }
  }
  void updateTower(Tower& t, float dt) {
    if (t.disabledUntil > time) {
      t.pulse = 0;
      t.spinSpeed *= std::max(0.f, 1 - dt * 5);
      t.recoil = std::max(0.f, t.recoil - dt * 2.8f);
      return;
    }
    if (isSupport(t.type)) {
      t.spin = std::fmod(t.spin + dt * (t.type == FrostTower ? 2.8f : 1.2f), 2 * PI);
      t.pulse = std::max(0.f, t.pulse - dt);
      t.phase += dt;
      if (t.phase >= 3) {
        t.phase -= 3;
        t.pulse = 1;
        if (t.type == FrostTower) {
          ring({t.x, .3f, t.z}, WEAPONS[FrostTower].color, 40, 2.8f);
          for (int i = 0; i < 64; ++i) {
            float a = fxRandom.unit() * 2 * PI, r = std::sqrt(fxRandom.unit()) * 40;
            particle({t.x + std::cos(a) * r, 2 + fxRandom.unit() * 7, t.z + std::sin(a) * r},
                     {-std::sin(a) * 5, -.6f, std::cos(a) * 5}, {.7f, .9f, 1},
                     .3f + fxRandom.unit() * .5f, 2.7f, 2);
          }
        }
      }
      return;
    }
    auto w = weaponInfo(t);
    // Carry a small residual between shots so upgrades survive fixed-step quantisation.
    t.cooldown = std::max(-dt, t.cooldown - dt);
    t.recoil = std::max(0.f, t.recoil - dt * 2.8f);
    t.pulse = std::max(0.f, t.pulse - dt * 2);
    t.effectClock -= dt;
    t.searchClock -= dt;
    if (focusTime > 0 && valid(focusTarget, focusGeneration)) {
      const auto& e = foes[focusTarget];
      if (std::hypot(e.x - t.x, e.z - t.z) <= w.range) {
        t.target = focusTarget;
        t.generation = focusGeneration;
      }
    }
    if (!inRange(t)) {
      t.target = -1;
      if (t.searchClock <= 0) {
        t.target = grid.nearest(t.x, t.z, w.range, foes);
        t.generation = t.target >= 0 ? foes[t.target].generation : 0;
        t.searchClock = .08f;
      }
    }
    bool ready = inRange(t);
    if (ready) {
      float a = std::atan2(foes[t.target].x - t.x, foes[t.target].z - t.z);
      float delta = std::atan2(std::sin(a - t.angle), std::cos(a - t.angle));
      t.angle += delta * std::min(1.f, dt * (t.type == Sniper ? 4.f : 10.f));
    }
    float desired = ready ? (t.type == Minigun ? 32.f : 4.f) : .35f;
    t.spinSpeed += (desired - t.spinSpeed) * std::min(1.f, dt * 3);
    t.spin = std::fmod(t.spin + t.spinSpeed * dt, 2 * PI);
    Vec3 muzzle{t.x + std::sin(t.angle) * 2.5f, w.height, t.z + std::cos(t.angle) * 2.5f};
    if (t.type == Lance) {
      if (t.state == 0) {
        if (ready)
          t.phase = std::min(6.f, t.phase + dt * towerRate * t.localRate *
                                                (t.amplifiedUntil > time ? 2.f : 1.f));
        if (t.phase >= 6 && ready) {
          t.state = 1;
          t.phase = 3;
          ++fired[Lance];
          ++t.shots;
        }
      } else if (t.state == 1) {
        t.phase -= dt;
        if (ready) {
          Vec3 end = targetPoint(t.target);
          hurt(t.target, w.damage * dt, Lance);
          t.pulse = 1;
          if (t.effectClock <= 0) {
            beam(muzzle, end, w.color, .65f, .075f);
            beam(muzzle, end, {1, .86f, .55f}, .18f, .075f);
            sparks(end, w.color, 2, 8);
            t.effectClock = .04f;
          }
        }
        if (t.phase <= 0) {
          t.state = 2;
          t.phase = w.cooldown;
        }
      } else {
        t.phase -= dt;
        if (t.phase <= 0) {
          t.phase = 0;
          t.state = 0;
        }
      }
      return;
    }
    if (t.type == Flame) {
      if (!ready)
        return;
      if (t.cooldown <= 0) {
        float dx = std::sin(t.angle), dz = std::cos(t.angle);
        grid.radius(t.x, t.z, w.range, foes, [&](int i) {
          auto& e = foes[i];
          float ex = e.x - t.x, ez = e.z - t.z, d = std::hypot(ex, ez);
          if (d < .001f || (ex * dx + ez * dz) / d > .78f) {
            e.burn = 3;
            e.burnDps = 12 * towerDamage;
            hurt(i, w.damage * .1f, Flame);
          }
        });
        t.cooldown += w.cooldown;
        ++t.shots;
        ++fired[Flame];
      }
      t.pulse = 1;
      if (t.effectClock <= 0) {
        for (int i = 0; i < 4; ++i) {
          float spread = (fxRandom.unit() - .5f) * .45f;
          Vec3 v{std::sin(t.angle + spread) * 38, (fxRandom.unit() - .7f) * 3,
                 std::cos(t.angle + spread) * 38};
          particle(muzzle, v, {1, .24f + fxRandom.unit() * .3f, .025f}, 1 + fxRandom.unit(),
                   .65f + fxRandom.unit() * .25f);
        }
        t.effectClock = .035f;
      }
      return;
    }
    if (!ready || t.cooldown > 0)
      return;
    if (t.type == Minigun && t.spinSpeed < 13)
      return;
    t.cooldown += w.cooldown;
    t.recoil = 1;
    t.pulse = 1;
    ++t.shots;
    ++fired[t.type];
    if (t.type == Tesla) {
      grid.radius(t.x, t.z, w.range, foes, [&](int i) {
        lightning({t.x, w.height + 1, t.z}, targetPoint(i), i);
        hurt(i, w.damage, Tesla);
      });
      ring({t.x, .15f, t.z}, w.color, w.range, .55f);
      return;
    }
    if (t.type == Venom || t.type == Rocket || t.type == Mortar) {
      launch(t);
      return;
    }
    Vec3 end = targetPoint(t.target);
    Color c = w.color;
    float width = t.type == Sniper ? .2f : t.type == Minigun ? .07f : .13f;
    beam(muzzle, end, c, width, t.type == Sniper ? .22f : .075f);
    sparks(end, c, 2, 8);
    hurt(t.target, w.damage, t.type);
  }
  void updateShots(float dt) {
    for (size_t i = 0; i < shots.size();) {
      auto& p = shots[i];
      p.previous = p.pos;
      p.age += dt;
      bool targetAlive = valid(p.target, p.generation);
      if (targetAlive)
        p.endpoint = targetPoint(p.target);
      bool impact = false;
      if (p.type == Mortar) {
        float t = std::min(1.f, p.age / p.duration);
        p.pos = p.start + (p.endpoint - p.start) * t;
        p.pos.y += std::sin(t * PI) * 40;
        impact = t >= 1;
      } else {
        Vec3 delta = p.endpoint - p.pos;
        float d = length(delta), step = p.speed * dt;
        p.pos = p.pos + delta * (std::min(step, d) / std::max(d, .00001f));
        impact = d <= step + .3f;
      }
      p.trailClock -= dt;
      if (p.trailClock <= 0) {
        particle(p.pos, {0, .7f, 0},
                 p.type == Venom ? Color{.3f, .8f, .1f} : Color{.35f, .4f, .43f},
                 p.type == Rocket ? .6f : .4f, .6f, 1);
        p.trailClock = .04f;
      }
      if (impact || p.age > 12) {
        if (p.type == Venom) {
          if (gas.size() < 512)
            gas.push_back({p.pos.x, p.pos.z, 9, 7, 0, 22 * p.damage / WEAPONS[Venom].damage});
          grid.radius(p.pos.x, p.pos.z, 9, foes, [&](int j) {
            foes[j].poison = 6;
            foes[j].poisonDps = std::max(foes[j].poisonDps, 22 * p.damage / WEAPONS[Venom].damage);
            hurt(j, p.damage, Venom);
          });
          ring({p.pos.x, .1f, p.pos.z}, WEAPONS[Venom].color, 9, .7f);
        } else
          blast(p.pos, p.radius, p.damage, p.type);
        shots[i] = shots.back();
        shots.pop_back();
      } else
        ++i;
    }
  }
  // Only elite saboteurs search the (at most 210) occupied hexes, twice per second.
  // Targets are stable IDs: vector compaction cannot redirect an ongoing attack.
  bool sabotageTower(Foe& e, float dt) {
    e.towerSearch -= dt;
    auto target = towers.end();
    if (e.towerTarget)
      target = std::find_if(towers.begin(), towers.end(),
                            [&](const Tower& t) { return t.id == e.towerTarget; });
    if (target == towers.end() && e.towerSearch <= 0) {
      e.towerSearch = .5f;
      e.sabotage = 0;
      e.towerTarget = 0;
      float best = 1e30f;
      for (auto it = towers.begin(); it != towers.end(); ++it) {
        float dx = it->x - e.x, dz = it->z - e.z, distance = dx * dx + dz * dz;
        if (distance < best) {
          best = distance;
          target = it;
        }
      }
      if (target != towers.end())
        e.towerTarget = target->id;
    }
    if (target == towers.end()) {
      e.sabotage = 0;
      return false;
    }
    float dx = target->x - e.x, dz = target->z - e.z, distance = std::hypot(dx, dz);
    if (distance > 4.05f) {
      e.sabotage = 0;
      float slow = movementSlowAt(e.x, e.z) * (e.toxicSlow > 0 ? .5f : 1.f);
      float amount = e.glued > 0 ? 0 : std::min(distance - 4, e.speed * slow * dt);
      Vec3 next{e.x + dx / distance * amount, 0, e.z + dz / distance * amount};
      // Slide around the core instead of walking through its cylinder.
      float r = std::hypot(next.x, next.z), safe = CORE_RADIUS + 3;
      if (r < safe) {
        float angle = std::atan2(e.x, e.z);
        float turn = e.z * dx - e.x * dz;
        angle += (turn >= 0 ? 1 : -1) * amount / safe;
        next = {std::sin(angle) * safe, 0, std::cos(angle) * safe};
      }
      e.x = next.x;
      e.z = next.z;
    } else {
      e.sabotage += dt;
      if (e.sabotage + .00001f >= SABOTAGE_TIME) {
        Vec3 p{target->x, 2, target->z};
        if (destroyedTowers.size() < MAX_TOWERS)
          destroyedTowers.push_back(target->type);
        towers.erase(target);
        rebuildSupport();
        ring({p.x, .2f, p.z}, {1, .3f, .05f}, 7, .8f);
        sparks(p, {1, .45f, .1f}, 24, 18);
        announce("Sabotażysta zniszczył wieżyczkę!");
        e.towerTarget = 0;
        e.sabotage = 0;
        e.towerSearch = 0;
      }
    }
    return true;
  }
  void beginFeeding(int id) {
    auto& boss = foes[id];
    uint8_t bit = uint8_t(boss.feedPending & uint8_t(-int(boss.feedPending)));
    boss.feedPending &= uint8_t(~bit);
    boss.absorbClock = 8;
    // Threshold events are rare. Scan live positions once, without stale grid cells,
    // new entities, HP changes or allocations. Other bosses cannot be consumed.
    for (int child : active) {
      auto& minion = foes[child];
      float dx = minion.x - boss.x, dz = minion.z - boss.z;
      if (minion.hp <= 0 || minion.type >= Titan || dx * dx + dz * dz > 50 * 50)
        continue;
      if (valid(minion.summoner, minion.summonerGeneration) &&
          foes[minion.summoner].absorbClock > 0)
        continue;
      minion.summoner = id;
      minion.summonerGeneration = boss.generation;
      minion.sabotage = 0;
    }
    ring({boss.x, .2f, boss.z}, {.8f, .12f, 1}, 50, 2);
    sparks(targetPoint(id), {.7f, .1f, 1}, 22, 15);
  }
  bool absorbMinion(Foe& minion, float dt) {
    if (!valid(minion.summoner, minion.summonerGeneration)) {
      minion.summoner = -1;
      return false;
    }
    auto& boss = foes[minion.summoner];
    if (boss.absorbClock <= 0) {
      minion.summoner = -1;
      return false;
    }
    // Two seconds to kill the offerings before the pull begins.
    if (boss.absorbClock > 6 || boss.stun > 0 || minion.glued > 0)
      return true;
    float dx = boss.x - minion.x, dz = boss.z - minion.z;
    float distance = std::hypot(dx, dz), mouth = boss.size * .28f;
    if (distance <= mouth + .05f) {
      boss.hp = std::min(boss.maxHp, boss.hp + minion.hp);
      beam({minion.x, minion.size * .4f, minion.z}, targetPoint(minion.summoner),
           {.65f, .2f, 1}, .5f, .35f);
      minion.hp = 0; // Absorption is not a kill and cannot create loot.
    } else {
      float amount = std::min(distance - mouth, 14.f * dt *
                              movementSlowAt(minion.x, minion.z) *
                              (minion.toxicSlow > 0 ? .5f : 1.f));
      minion.x += dx / distance * amount;
      minion.z += dz / distance * amount;
    }
    return true;
  }
  void updateWorms() {
    for (size_t n = 0; n < worms.size();) {
      auto& body = worms[n];
      if (!valid(body.head, body.generation)) {
        body = worms.back();
        worms.pop_back();
        continue;
      }
      Vec3 previous{foes[body.head].x, 0, foes[body.head].z};
      for (auto& segment : body.segments) {
        Vec3 delta = segment - previous;
        float distance = length(delta);
        if (distance > 7)
          segment = previous + delta * (7 / distance);
        previous = segment;
      }
      ++n;
    }
  }
  void step(float dt) {
    if (paused || dt <= 0)
      return;
    dt = std::min(dt, .1f);
    time += dt;
    bool supportDirty = false;
    for (auto& tower : towers)
      if (tower.disabledUntil > 0 && tower.disabledUntil <= time) {
        tower.disabledUntil = 0;
        supportDirty = true;
      }
    focusTime = std::max(0.f, focusTime - dt);
    for (size_t i = 0; i < active.size();) {
      int id = active[i];
      if (foes[id].hp <= 0) {
        freeIds.push_back(id);
        active[i] = active.back();
        active.pop_back();
      } else
        ++i;
    }
    if (autoWaves && time >= nextWave)
      advanceWave();
    spawnClock -= dt;
    if (pending > 0 && spawnClock <= 0) {
      int group = std::min(pending, 4);
      for (int i = 0; i < group; ++i) {
        float a = random.unit() * 2 * PI, r = 430 + random.unit() * 200;
        int maxType = std::min(5, 1 + wave);
        Species kind = Species(random.next() % maxType);
        spawn(kind, std::sin(a) * r, std::cos(a) * r);
      }
      pending -= group;
      spawnClock = .1f;
    }
    size_t count = active.size();
    for (size_t k = 0; k < count; ++k) {
      int i = active[k];
      auto& e = foes[i];
      e.flash = std::max(0.f, e.flash - dt);
      bool stunned = e.stun > 0;
      e.toxicSlow = std::max(0.f, e.toxicSlow - dt);
      e.glued = std::max(0.f, e.glued - dt);
      e.stun = std::max(0.f, e.stun - dt);
      if (!stunned)
        e.ability -= dt;
      if (e.poison > 0) {
        float tick = std::min(e.poison, dt);
        e.poison -= tick;
        hurt(i, (e.poisonDps + (e.type >= Titan ? 100.f : 20.f)) * tick, Venom);
      }
      if (e.burn > 0) {
        float tick = std::min(e.burn, dt);
        e.burn -= tick;
        hurt(i, e.burnDps * tick, Flame);
      }
      if (stunned)
        e.sabotage = 0;
      if (e.hp <= 0 || stunned)
        continue;
      if (e.summoner >= 0 && absorbMinion(e, dt))
        continue;
      if (e.type == Devourer) {
        e.absorbClock = std::max(0.f, e.absorbClock - dt);
        if (e.absorbClock <= 0 && e.feedPending)
          beginFeeding(i);
      }
      if (e.type == Saboteur && sabotageTower(e, dt))
        continue;
      float distance = std::hypot(e.x, e.z);
      float stop = CORE_RADIUS + e.size * .25f + 1;
      if (e.type == Spitter)
        stop = 48;
      if (e.type == SiegeSpitter || e.type == SiegeBoss)
        stop = SIEGE_RANGE;
      if (distance > stop) {
        float slow = movementSlowAt(e.x, e.z) * (e.toxicSlow > 0 ? .5f : 1.f);
        float amount = e.glued > 0 || e.absorbClock > 0
                           ? 0 : std::min(distance - stop, e.speed * slow * dt);
        if (e.type == Wyrm) {
          float lateral = amount > 0 ? std::sin(time * WYRM_SWAY_FREQUENCY + e.phase) *
              WYRM_LATERAL_SPEED * slow * dt * (e.speed / WYRM_FORWARD_SPEED) : 0;
          // Rotate around the core independently of inward movement. A wide sway
          // must neither speed up the advance nor push the boss away from the core.
          float angle = std::atan2(e.x, e.z) + lateral / distance;
          e.x = std::sin(angle) * (distance - amount);
          e.z = std::cos(angle) * (distance - amount);
        } else {
          e.x -= e.x / distance * amount;
          e.z -= e.z / distance * amount;
        }
      } else if (e.type != Spitter && e.type != SiegeSpitter && e.type != SiegeBoss)
        coreDamage += dt * (e.type >= Titan ? 80 : e.elite ? 8 : 4);
      if (e.type == Brain && e.ability <= 0) {
        e.ability = BRAIN_INTERVAL;
        bool hit = false;
        for (auto& tower : towers) {
          float dx = tower.x - e.x, dz = tower.z - e.z;
          if (dx * dx + dz * dz <= BRAIN_RADIUS * BRAIN_RADIUS) {
            tower.disabledUntil = std::max(tower.disabledUntil, time + BRAIN_DISABLE);
            hit = true;
          }
        }
        supportDirty |= hit;
        ring({e.x, .25f, e.z}, {.8f, .25f, 1}, BRAIN_RADIUS, 1.4f);
        sparks(targetPoint(i), {.7f, .3f, 1}, 18, 12);
      }
      if (e.type == Queen && distance < 240 && e.ability <= 0) {
        e.ability = 8;
        for (int n = 0; n < 5; ++n) {
          float a = n * 2 * PI / 5;
          int child = spawn(Runner, e.x + std::sin(a) * 10, e.z + std::cos(a) * 10);
          if (child >= 0 && expeditionRules)
            foes[child].speed = 7;
        }
        ring({e.x, .2f, e.z}, {.8f, .2f, 1}, 18, 1);
      }
      if (e.type == Titan && distance < 80 && e.ability <= 0) {
        e.ability = 6;
        ring({e.x, .1f, e.z}, {1, .4f, .1f}, 35, 1.2f);
        sparks(targetPoint(i), {1, .4f, .1f}, 15, 14);
        if (expeditionRules)
          coreDamage += 30;
      }
      if (e.type == Spitter && distance < 55 && e.ability <= 0) {
        e.ability = 3;
        beam(targetPoint(i), {0, CORE_HEIGHT * .5f, 0}, {.35f, 1, .1f}, .3f, .2f);
        coreDamage += e.elite ? 50 : 25;
      }
      if ((e.type == SiegeSpitter || e.type == SiegeBoss) && distance <= SIEGE_RANGE + .1f &&
          e.ability <= 0) {
        e.ability = 5;
        // A bounded pool of ballistic acid projectiles, rather than hits every frame.
        if (enemyShots.size() < 512)
          enemyShots.push_back(
              {targetPoint(i), 0, std::max(.8f, distance / 35),
               e.type == SiegeBoss ? 200.f : e.elite ? 90.f : 45.f});
        sparks(targetPoint(i), {.4f, 1, .08f}, 4, 3);
      }
    }
    updateWorms();
    if (supportDirty)
      rebuildSupport();
    for (size_t n = 0; n < enemyShots.size();) {
      auto& shot = enemyShots[n];
      shot.age += dt;
      if (shot.age >= shot.duration) {
        coreDamage += shot.damage;
        sparks({0, CORE_HEIGHT * .5f, 0}, {.35f, 1, .1f}, 7, 8);
        enemyShots[n] = enemyShots.back();
        enemyShots.pop_back();
      } else
        ++n;
    }
    grid.rebuild(foes, active);
    for (auto& t : towers)
      updateTower(t, dt);
    updateShots(dt);
    for (size_t i = 0; i < gas.size();) {
      auto& g = gas[i];
      g.life -= dt;
      g.tick -= dt;
      if (g.tick <= 0) {
        grid.radius(g.x, g.z, g.radius, foes, [&](int j) {
          foes[j].poison = 6;
          foes[j].poisonDps = std::max(foes[j].poisonDps, g.poisonDps);
        });
        g.tick = .25f;
      }
      if (g.life <= 0) {
        gas[i] = gas.back();
        gas.pop_back();
      } else
        ++i;
    }
    for (size_t i = 0; i < beams.size();) {
      beams[i].life -= dt;
      if (beams[i].life <= 0) {
        beams[i] = beams.back();
        beams.pop_back();
      } else
        ++i;
    }
    for (size_t i = 0; i < particles.size();) {
      auto& p = particles[i];
      p.life -= dt;
      p.pos = p.pos + p.velocity * dt;
      if (p.style == 0)
        p.velocity.y -= dt * 3;
      if (p.life <= 0) {
        particles[i] = particles.back();
        particles.pop_back();
      } else
        ++i;
    }
    for (size_t i = 0; i < rings.size();) {
      rings[i].life -= dt;
      if (rings[i].life <= 0) {
        rings[i] = rings.back();
        rings.pop_back();
      } else
        ++i;
    }
  }
};
} // namespace arsenal
