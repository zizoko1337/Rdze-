#pragma once
#include "expedition.hpp"

namespace command {
enum class CrystalState { Ground, Queued, Carried };
struct Crystal {
  uint64_t id;
  Vec3 position;
  bool boss;
  CrystalState state = CrystalState::Ground;
};
struct Rover {
  Vec3 position;
  int target = -1;
  uint32_t generation = 0;
  float searchClock = 0, life = 60;
  uint32_t serial = 0;
};
enum class CollectorState { Idle, Outbound, Collecting, Inbound };
struct Frontier {
  static constexpr int MAX_ROVERS = 2048, MAX_CRYSTALS = 4096, MAX_ACTIVE = 6000;
  static constexpr float ROVER_DAMAGE = SPECIES[Crawler].hp, ROVER_SPEED = 24;
  static constexpr Vec3 COLLECTOR_DOCK{-21, 7, -6};
  std::vector<Rover> rovers;
  // Aggregate nearby traffic instead of checking every pair of cars.
  struct TrafficCell {
    int x = 0, z = 0, count = 0;
    uint32_t epoch = 0;
    Vec3 sum{};
  };
  std::vector<TrafficCell> traffic = std::vector<TrafficCell>(8192);
  uint32_t trafficEpoch = 0;
  TrafficCell& trafficCell(int x, int z) {
    uint32_t slot = (uint32_t(x) * 73856093u ^ uint32_t(z) * 19349663u) & 8191u;
    while (traffic[slot].epoch == trafficEpoch && (traffic[slot].x != x || traffic[slot].z != z))
      slot = (slot + 1) & 8191u;
    auto& cell = traffic[slot];
    if (cell.epoch != trafficEpoch)
      cell = {x, z, 0, trafficEpoch, {}};
    return cell;
  }
  const TrafficCell* findTraffic(int x, int z) const {
    uint32_t slot = (uint32_t(x) * 73856093u ^ uint32_t(z) * 19349663u) & 8191u;
    while (traffic[slot].epoch == trafficEpoch) {
      if (traffic[slot].x == x && traffic[slot].z == z)
        return &traffic[slot];
      slot = (slot + 1) & 8191u;
    }
    return nullptr;
  }
  std::vector<Crystal> crystals;
  std::deque<uint64_t> collectionQueue;
  Random lootRandom;
  std::array<uint64_t, SpeciesCount> spawned{};
  uint64_t crystalSerial = 0, roversMade = 0, roverHits = 0;
  int bossesSpawned = 0, eliteSpawned = 0, collected = 0;
  float nextBoss = 180, nextElite = 90, respiteUntil = 0, productionClock = 0;
  float nextHorde = 600, hordeClock = 0;
  int pendingHorde = 0;
  uint64_t hordeSpawned = 0;
  bool bossWarning = false;
  CollectorState collectorState = CollectorState::Idle;
  uint64_t collectorTarget = 0;
  Vec3 collectorPosition = COLLECTOR_DOCK, collectionPoint{};
  float collectorAge = 0, collectorDuration = 0;
  double roverDamage = 0;
  Frontier() {
    rovers.reserve(MAX_ROVERS);
    crystals.reserve(128);
    lootRandom.state = 51937;
  }
  static const char* stageName(float time) {
    return time < 60    ? "PEŁZACZE"
           : time < 120 ? "ŁOWCY"
           : time < 180 ? "TARANY"
           : time < 240 ? "PLUJCE"
                        : "PEŁNY RÓJ";
  }
  Crystal* crystal(uint64_t id) {
    for (auto& c : crystals)
      if (c.id == id)
        return &c;
    return nullptr;
  }
  uint64_t dropCrystal(Vec3 position, bool boss) {
    if (crystals.size() >= MAX_CRYSTALS) {
      auto expendable = std::find_if(crystals.begin(), crystals.end(), [](const Crystal& c) {
        return !c.boss && c.state == CrystalState::Ground;
      });
      if (expendable == crystals.end())
        return 0;
      crystals.erase(expendable);
    }
    crystals.push_back({++crystalSerial, position, boss});
    return crystalSerial;
  }
  bool requestCollection(uint64_t id) {
    auto* c = crystal(id);
    if (!c || c->state != CrystalState::Ground)
      return false;
    c->state = CrystalState::Queued;
    collectionQueue.push_back(id);
    return true;
  }
  uint64_t nearestCrystal() const {
    uint64_t result = 0;
    float best = 1e30f;
    for (const auto& c : crystals)
      if (c.state == CrystalState::Ground) {
        float distance = length(c.position - COLLECTOR_DOCK);
        if (distance < best) {
          best = distance;
          result = c.id;
        }
      }
    return result;
  }
  void processDeaths(Game& g) {
    for (const auto& death : g.deaths) {
      bool boss = death.type >= Titan;
      if (boss || (death.elite && lootRandom.next() % 100 == 0)) {
        dropCrystal(death.position, boss);
        g.announce(boss ? "Kryształ bossa! Tab: wskaż go i wyślij drona [LPM / C]."
                        : "Elita pozostawiła kryształ. Wyślij drona z widoku taktycznego.");
      }
      if (boss)
        respiteUntil = g.time + 15;
    }
    g.deaths.clear();
  }
  void stepCollector(Game& g, Expedition& e, float dt) {
    if (collectorState == CollectorState::Idle) {
      if (collectionQueue.empty())
        return;
      collectorTarget = collectionQueue.front();
      collectionQueue.pop_front();
      auto* c = crystal(collectorTarget);
      if (!c)
        return;
      collectionPoint = c->position + Vec3{0, 5, 0};
      float radius = std::hypot(collectionPoint.x, collectionPoint.z);
      if (radius < CORE_RADIUS + 7) {
        collectionPoint.x = (radius > .001f ? collectionPoint.x / radius : 1) * (CORE_RADIUS + 7);
        collectionPoint.z = (radius > .001f ? collectionPoint.z / radius : 0) * (CORE_RADIUS + 7);
      }
      collectorDuration = std::max(3.f, length(collectionPoint - COLLECTOR_DOCK) / 42 + 3);
      collectorAge = 0;
      collectorState = CollectorState::Outbound;
    }
    collectorAge += dt;
    if (collectorState == CollectorState::Outbound || collectorState == CollectorState::Inbound) {
      float t = std::clamp(collectorAge / collectorDuration, 0.f, 1.f);
      Vec3 from = collectorState == CollectorState::Outbound ? COLLECTOR_DOCK : collectionPoint;
      Vec3 to = collectorState == CollectorState::Outbound ? collectionPoint : COLLECTOR_DOCK;
      auto ease = [](float a) {
        a = std::clamp(a, 0.f, 1.f);
        return a * a * (3 - 2 * a);
      };
      Vec3 overheadFrom{from.x, CORE_HEIGHT + 15, from.z};
      Vec3 overheadTo{to.x, CORE_HEIGHT + 15, to.z};
      if (t < .2f) {
        float a = ease(t / .2f);
        collectorPosition = from * (1 - a) + overheadFrom * a;
      } else if (t < .8f) {
        float a = ease((t - .2f) / .6f);
        collectorPosition = overheadFrom * (1 - a) + overheadTo * a;
      } else {
        float a = ease((t - .8f) / .2f);
        collectorPosition = overheadTo * (1 - a) + to * a;
      }
      if (t >= 1) {
        collectorAge = 0;
        if (collectorState == CollectorState::Outbound)
          collectorState = CollectorState::Collecting;
        else {
          auto* c = crystal(collectorTarget);
          if (c) {
            e.addReward(c->boss ? RewardSource::BossCrystal : RewardSource::EliteCrystal);
            ++collected;
            g.announce("Dron dostarczył kryształ. Nagroda czeka przy komputerze [E].");
            std::erase_if(crystals,
                          [&](const Crystal& value) { return value.id == collectorTarget; });
          }
          collectorState = CollectorState::Idle;
          collectorTarget = 0;
        }
      }
    } else if (collectorState == CollectorState::Collecting && collectorAge >= 2) {
      if (auto* c = crystal(collectorTarget))
        c->state = CrystalState::Carried;
      collectorAge = 0;
      collectorState = CollectorState::Inbound;
    }
  }
  static float spawnHp(Species type, float time, bool elite = false) {
    float scale = type == Crawler ? 1 : 1 + std::max(0.f, time - 120) / 1200;
    return SPECIES[type].hp * scale * (elite && type != Saboteur ? 4 : 1);
  }
  static float spawnSpeed(Species type, float time, bool elite = false) {
    constexpr float speeds[] = {3.2f, 6.5f, 2.4f, 3.4f, 5.8f, 5, 3, 2.4f, 2.8f, 2.6f};
    static_assert(std::size(speeds) == SpeciesCount);
    return (speeds[type] + (type <= Runner ? std::min(time / 120, 4.f) : 0)) *
           (elite && type != Saboteur ? .9f : 1.f);
  }
  int spawnFoe(Game& g, Species type, float x, float z, bool elite = false) {
    int id = g.spawn(type, x, z);
    if (id < 0)
      return id;
    auto& foe = g.foes[id];
    foe.hp = foe.maxHp = spawnHp(type, g.time, elite);
    foe.speed = spawnSpeed(type, g.time, elite);
    foe.elite = elite || type == Saboteur;
    if (foe.elite) {
      if (type != Saboteur)
        foe.size *= 1.4f;
      ++eliteSpawned;
    }
    ++spawned[type];
    return id;
  }
  Species waveSpecies(Game& g) {
    int roll = int(g.random.next() % 100);
    if (g.time >= 300 && roll >= 94)
      return SiegeSpitter;
    if (g.time >= 240 && roll < 12)
      return Floater;
    if (g.time >= 180 && roll < 24)
      return Spitter;
    if (g.time >= 120 && roll < 40)
      return Brute;
    if (g.time >= 60 && roll < 65)
      return Runner;
    return Crawler;
  }
  void stepWaves(Game& g, float dt, float& spawnTimer) {
    if (!bossWarning && g.time >= nextBoss - 10) {
      bossWarning = true;
      g.announce("ALARM: duża sygnatura. Boss nadejdzie za 10 sekund.");
    }
    bool added = false;
    if (g.time >= nextBoss) {
      int id = g.boss();
      if (id >= 0) {
        auto& boss = g.foes[id];
        boss.hp = boss.maxHp = (boss.type == Titan   ? 7000.f
                                : boss.type == Brain ? 8000.f
                                                     : 6000.f) +
                               bossesSpawned * 1500;
        boss.speed = spawnSpeed(boss.type, g.time);
        ++spawned[boss.type];
        ++bossesSpawned;
        nextBoss += 180;
        bossWarning = false;
        added = true;
      }
    }
    if (g.time >= nextElite && g.active.size() < MAX_ACTIVE) {
      float a = g.random.unit() * 2 * PI;
      Species kind = g.time >= 135 && (eliteSpawned % 2 == 1) ? Saboteur : waveSpecies(g);
      spawnFoe(g, kind, std::sin(a) * 235, std::cos(a) * 235, true);
      nextElite += 45;
      added = true;
    }
    spawnTimer -= dt;
    if (spawnTimer <= 0 && g.active.size() < MAX_ACTIVE) {
      bool respite = g.time < respiteUntil;
      float interval =
          g.time < 60 ? 2.8f - g.time * .006f : std::max(.6f, 2.44f - (g.time - 60) * .0024f);
      spawnTimer = interval * (respite ? 3 : 1);
      int group = respite ? 1 : 1 + std::min(7, int(g.time / 240));
      for (int i = 0; i < group && g.active.size() < MAX_ACTIVE; ++i) {
        float a = g.random.unit() * 2 * PI, r = 160 + g.random.unit() * 100;
        spawnFoe(g, waveSpecies(g), std::sin(a) * r, std::cos(a) * r);
      }
      added = true;
    }
    if (g.time >= nextHorde) {
      // Queue the burst, then release at most 12 foes per frame.
      pendingHorde = std::min(
          2400, pendingHorde + std::min(600, 120 + 30 * int(std::max(0.f, g.time - 600) / 60)));
      nextHorde = g.time + 15;
    }
    hordeClock -= dt;
    if (pendingHorde > 0 && hordeClock <= 0 && g.time >= respiteUntil) {
      hordeClock = .08f;
      int count = std::min({12, pendingHorde, MAX_ACTIVE - int(g.active.size())});
      for (int i = 0; i < count; ++i) {
        float a = float(hordeSpawned % 3) * (2 * PI / 3) + float(int(g.time / 15)) * .71f +
                  (g.random.unit() - .5f) * .8f;
        float r = 240 + g.random.unit() * 60;
        spawnFoe(g, Crawler, std::sin(a) * r, std::cos(a) * r);
        ++hordeSpawned;
      }
      pendingHorde -= count;
      added |= count > 0;
    }
    if (added)
      g.grid.rebuild(g.foes, g.active);
  }
  void stepRovers(Game& g, const Expedition& e, float dt) {
    if (e.levels[size_t(Upgrade::RoverFactory)] > 0) {
      productionClock -= dt;
      if (productionClock <= 0) {
        productionClock = std::max(-dt, productionClock) + e.roverInterval();
        for (int i = 0; i < e.roverBatch && rovers.size() < MAX_ROVERS; ++i) {
          uint32_t serial = uint32_t(roversMade++);
          float a = float(serial % 65536) * 2.399963f;
          float radius = CORE_RADIUS + 3 + float(serial % 3) * 1.1f;
          Rover rover{{std::sin(a) * radius, .4f, std::cos(a) * radius}};
          rover.serial = serial;
          rover.searchClock = float(serial % 12) / 60;
          rovers.push_back(rover);
        }
      }
    }
    if (++trafficEpoch == 0) {
      for (auto& cell : traffic)
        cell.epoch = 0;
      trafficEpoch = 1;
    }
    for (const auto& rover : rovers) {
      auto& cell =
          trafficCell(int(std::floor(rover.position.x / 3)), int(std::floor(rover.position.z / 3)));
      ++cell.count;
      cell.sum = cell.sum + rover.position;
    }
    for (size_t i = 0; i < rovers.size();) {
      auto& rover = rovers[i];
      rover.life -= dt;
      rover.searchClock -= dt;
      if (!g.valid(rover.target, rover.generation) && rover.searchClock <= 0) {
        rover.target = g.grid.nearest(rover.position.x, rover.position.z, 650, g.foes);
        rover.generation = rover.target >= 0 ? g.foes[rover.target].generation : 0;
        rover.searchClock = .2f;
      }
      bool impact = false;
      if (g.valid(rover.target, rover.generation)) {
        auto& foe = g.foes[rover.target];
        Vec3 delta{foe.x - rover.position.x, 0, foe.z - rover.position.z};
        float distance = length(delta), contact = foe.size * .43f + .35f;
        float step = ROVER_SPEED * (.92f + float(rover.serial % 17) * .01f) * dt;
        if (distance <= step + contact) {
          roverDamage += g.hurt(rover.target, ROVER_DAMAGE, Laser, false);
          ++roverHits;
          impact = true;
          g.sparks(g.targetPoint(rover.target), {.3f, 1, .8f}, 10, 8);
          g.ring(rover.position, {.3f, .9f, .8f}, 2.5f, .25f); // Cosmetic only, no area query.
        } else {
          Vec3 direction = delta * (1 / distance);
          Vec3 separation{};
          int cx = int(std::floor(rover.position.x / 3));
          int cz = int(std::floor(rover.position.z / 3));
          for (int z = cz - 1; z <= cz + 1; ++z)
            for (int x = cx - 1; x <= cx + 1; ++x) {
              auto* found = findTraffic(x, z);
              if (!found)
                continue;
              const auto& cell = *found;
              bool own = x == cx && z == cz;
              int count = cell.count - int(own);
              if (count <= 0)
                continue;
              Vec3 away =
                  rover.position - (cell.sum - (own ? rover.position : Vec3{})) * (1.f / count);
              away.y = 0;
              float d = length(away);
              if (d < 2.4f) {
                if (d < .001f) {
                  float angle = float(rover.serial % 1024) * 2.399963f;
                  away = {std::sin(angle), 0, std::cos(angle)};
                  d = 1;
                }
                separation = separation + away * ((2.4f - d) / (2.4f * d));
              }
            }
          // Lateral correction keeps traffic moving towards its target.
          Vec3 lateral = separation - direction * dot(separation, direction);
          float strength = length(lateral);
          if (strength > 1)
            lateral = lateral * (1 / strength);
          direction = normalize(direction + lateral * 1.5f);
          Vec3 ground{rover.position.x, 0, rover.position.z};
          float along = std::clamp(-dot(ground, direction), 0.f, distance);
          Vec3 closest = ground + direction * along;
          if (length(closest) < CORE_RADIUS + 2) {
            Vec3 radial = normalize(ground);
            float sign = radial.x * direction.z - radial.z * direction.x >= 0 ? 1.f : -1.f;
            direction = normalize(Vec3{-radial.z * sign, 0, radial.x * sign} + direction * .15f);
          }
          rover.position = rover.position + direction * step;
          float radius = std::hypot(rover.position.x, rover.position.z);
          if (radius < CORE_RADIUS + 2) {
            rover.position.x *= (CORE_RADIUS + 2) / radius;
            rover.position.z *= (CORE_RADIUS + 2) / radius;
          }
        }
      }
      if (impact || rover.life <= 0) {
        rovers[i] = rovers.back();
        rovers.pop_back();
      } else
        ++i;
    }
  }
  void step(Game& g, Expedition& e, float dt, float& spawnTimer) {
    if (!e.enabled || e.dead() || g.paused || dt <= 0)
      return;
    processDeaths(g);
    stepWaves(g, dt, spawnTimer);
    stepRovers(g, e, dt);
    processDeaths(g);
    stepCollector(g, e, dt);
  }
};
} // namespace command
