#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
void tick(Session& s, float seconds) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60);
}
void quiet(Session& s) {
  s.mode = Mode::Walking;
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
}
void acquire(Session& s, Upgrade u, Rarity rarity = Rarity::Uncommon) {
  s.expedition.rewards.push_front(
      {1, {u, Upgrade::Mining, Upgrade::Hull}, {rarity, Rarity::Common, Rarity::Common}});
  s.terminalOpen = true;
  require(s.chooseUpgrade(0), "Upgrade rejected");
}
int enemy(Session& s, float x, float z) {
  int id = s.world.spawn(Crawler, x, z);
  s.world.foes[id].speed = 0;
  s.world.grid.rebuild(s.world.foes, s.world.active);
  return id;
}
int main() {
  try {
    {
      Session s(true, true);
      quiet(s);
      require(!s.expedition.eligible(Upgrade::RoverBatch) &&
                  !s.expedition.eligible(Upgrade::RoverRate),
              "Factory upgrades offered before unlock");
      tick(s, 13);
      require(s.frontier.rovers.empty(), "Rovers produced without factory");
      for (int tier = 0; tier < 5; ++tier) {
        Session a(true, true);
        quiet(a);
        acquire(a, Upgrade::RoverFactory);
        acquire(a, Upgrade::RoverBatch, Rarity(tier));
        acquire(a, Upgrade::RoverRate, Rarity(tier));
        require(a.expedition.roverBatch == tier + 2 &&
                    std::abs(a.expedition.roverRate - (1.1f + tier * .1f)) < .0001f,
                "Rarity does not scale production quantity / rate");
        require(!a.expedition.eligible(Upgrade::RoverFactory), "Factory unlock repeats");
        tick(a, .05f);
        require(a.frontier.roversMade == uint64_t(tier + 2), "Initial production batch wrong");
        float clock = a.frontier.productionClock;
        a.world.paused = true;
        tick(a, 2);
        require(a.frontier.productionClock == clock, "Production ignores pause");
        a.world.paused = false;
        tick(a, a.expedition.roverInterval() + .1f);
        require(a.frontier.roversMade == uint64_t(2 * (tier + 2)), "Production cadence wrong");
      }
      std::cout << "PASS factory gating, all five batch/rate tiers, actual production and pause\n";
    }
    {
      Session s(true, true);
      quiet(s);
      int a = enemy(s, 20, 0), b = enemy(s, 20, 1);
      s.frontier.rovers.push_back({{18, .8f, 0}});
      tick(s, .05f);
      require(s.world.foes[a].hp <= 0 && s.world.foes[b].hp == 100 && s.frontier.roverHits == 1 &&
                  s.frontier.rovers.empty() && s.frontier.roverDamage == 100 &&
                  s.world.damageDone[Laser] == 0,
              "Rover fails one-hit crawler or applies area damage / weapon stats");
      Session retarget(true, true);
      quiet(retarget);
      int first = enemy(retarget, 40, 0), second = enemy(retarget, 80, 0);
      retarget.frontier.rovers.push_back({{17, .8f, 0}});
      tick(retarget, .05f);
      require(retarget.frontier.rovers.front().target == first,
              "Rover does not choose nearest enemy");
      retarget.world.hurt(first, 100, Laser);
      tick(retarget, .05f);
      int reused = enemy(retarget, 300, 0);
      require(reused == first, "Reuse fixture failed");
      tick(retarget, .3f);
      require(retarget.frontier.rovers.front().target == second,
              "Rover follows a recycled enemy slot instead of retargeting");
      std::cout << "PASS nearest pursuit, 100 single-target damage, no friendly stat pollution and "
                   "safe retarget\n";
    }
    {
      Session s(true, true);
      quiet(s);
      enemy(s, 0, -50);
      s.frontier.rovers.push_back({{0, .8f, 17}});
      for (int i = 0; i < 6 * 60 && !s.frontier.rovers.empty(); ++i) {
        s.step(1.f / 60);
        for (const auto& rover : s.frontier.rovers)
          require(std::hypot(rover.position.x, rover.position.z) >= CORE_RADIUS + 1.9f,
                  "Rover drives through the core");
      }
      require(s.frontier.roverHits == 1, "Rover cannot route around core to opposite target");
      Session crowd(true, true);
      quiet(crowd);
      int id = enemy(crowd, 600, 0);
      crowd.world.foes[id].hp = 10000000;
      crowd.expedition.levels[size_t(Upgrade::RoverFactory)] = 1;
      crowd.expedition.roverBatch = 128;
      crowd.expedition.roverRate = 24;
      tick(crowd, 20);
      require(crowd.frontier.rovers.size() == Frontier::MAX_ROVERS &&
                  crowd.frontier.roversMade == Frontier::MAX_ROVERS,
              "Maximum production exceeds pooled rover cap");
      std::cout << "PASS core avoidance and 2048 live homing rovers under maximum production\n";
    }
    {
      Session s(true, true);
      quiet(s);
      for (int i = 0; i < 10000; ++i) {
        int id = s.world.spawn(Crawler, 100, 0);
        s.world.foes[id].elite = true;
        s.world.hurt(id, 100, Laser);
      }
      s.frontier.processDeaths(s.world);
      size_t eliteDrops = s.frontier.crystals.size();
      require(eliteDrops > 65 && eliteDrops < 135, "Elite crystal probability is not around 1%");
      int boss = s.world.spawn(Titan, 150, 0);
      s.world.hurt(boss, s.world.foes[boss].hp, Laser);
      s.world.hurt(boss, 1, Laser);
      s.frontier.processDeaths(s.world);
      require(s.frontier.crystals.size() == eliteDrops + 1 && s.frontier.crystals.back().boss,
              "Boss does not drop exactly one crystal");
      s.frontier.processDeaths(s.world);
      require(s.frontier.crystals.size() == eliteDrops + 1, "Death event drops twice");
      std::cout
          << "PASS 1% elite drops, guaranteed boss crystal and exactly-once death processing\n";
    }
    {
      Session s(true, true);
      quiet(s);
      s.expedition.miningMultiplier = 0;
      uint64_t boss = s.frontier.dropCrystal({80, .6f, 40}, true);
      uint64_t elite = s.frontier.dropCrystal({-50, .6f, 20}, false);
      tick(s, 2);
      require(s.expedition.rewards.empty() && s.frontier.collectorState == CollectorState::Idle,
              "Crystal collected without dispatch");
      require(s.frontier.requestCollection(boss) && !s.frontier.requestCollection(boss) &&
                  s.frontier.requestCollection(elite),
              "Collector queue accepts duplicates or loses orders");
      tick(s, 1);
      Vec3 pos = s.frontier.collectorPosition;
      s.terminalOpen = true;
      tick(s, 4);
      require(length(pos - s.frontier.collectorPosition) == 0 && s.expedition.rewards.empty(),
              "Collector ignores terminal pause or grants reward before returning");
      s.terminalOpen = false;
      tick(s, 30);
      require(s.frontier.collected == 2 && s.frontier.crystals.empty() &&
                  s.expedition.rewards.size() == 2 && s.expedition.earned == 0 &&
                  s.expedition.shipments == 0,
              "Crystal delivery corrupts uranium accounting");
      require(s.expedition.rewards[0].source == RewardSource::BossCrystal &&
                  s.expedition.rewards[1].source == RewardSource::EliteCrystal,
              "Reward lost crystal provenance");
      for (auto r : s.expedition.rewards[0].rarities)
        require(r >= Rarity::Rare, "Boss crystal rolled below Rare");
      auto before = s.expedition.rewards[0];
      s.terminalOpen = true;
      s.interact();
      s.terminalOpen = true;
      require(s.expedition.rewards[0].choices == before.choices &&
                  s.expedition.rewards[0].rarities == before.rarities,
              "Reopening terminal rerolls a crystal reward");
      s.expedition.addReward();
      require(s.expedition.rewards[0].id != s.expedition.rewards[1].id &&
                  s.expedition.rewards[1].id != s.expedition.rewards[2].id,
              "Crystal and uranium reward IDs collide");
      std::cout << "PASS manual dispatch, queue, pause, return-only reward, rarity floor and "
                   "immutable offers\n";
    }
    {
      Expedition ordinary(true, true), boss(true, true);
      int rareNormal = 0, rareBoss = 0;
      for (int i = 0; i < 10000; ++i) {
        rareNormal += ordinary.rollRarity() >= Rarity::Epic;
        rareBoss += boss.rollRarity(RewardSource::BossCrystal) >= Rarity::Epic;
      }
      require(rareBoss > rareNormal * 3, "Boss reward odds not improved");
      Session s(true, true);
      quiet(s);
      s.frontier.nextBoss = 180;
      s.world.time = 169.9f;
      tick(s, .2f);
      require(s.frontier.bossWarning && s.frontier.bossesSpawned == 0,
              "Missing advance boss warning");
      s.world.time = 179.9f;
      tick(s, .2f);
      require(s.frontier.bossesSpawned == 1 && s.frontier.spawned[Titan] == 1 &&
                  s.frontier.nextBoss == 360,
              "First scheduled boss missing");
      int id = s.world.active.back();
      s.world.hurt(id, s.world.foes[id].hp, Laser);
      tick(s, .05f);
      require(s.frontier.respiteUntil > s.world.time + 14, "Boss death gives no breathing room");
      s.world.time = 359.9f;
      tick(s, .2f);
      require(s.frontier.bossesSpawned == 2 && s.frontier.spawned[Queen] == 1,
              "Boss variants do not alternate");
      for (float time : {65.f, 125.f, 185.f, 245.f}) {
        s.world.time = time;
        std::array<bool, 5> seen{};
        for (int i = 0; i < 1000; ++i)
          seen[s.frontier.waveSpecies(s.world)] = true;
        int count = int(time / 60) + 1;
        for (int i = 0; i < 5; ++i)
          require(seen[i] == (i < count), "Species unlock schedule wrong");
      }
      s.world.time = 900;
      int crawler = s.frontier.spawnFoe(s.world, Crawler, 200, 0);
      require(s.world.foes[crawler].maxHp == Frontier::ROVER_DAMAGE,
              "Basic crawler scales beyond base rover damage");
      std::cout << "PASS boss quality odds, warning, 3-minute schedule, respite and five species "
                   "progression\n";
    }
    {
      Session s(true, true);
      quiet(s);
      enemy(s, 600, 0);
      s.expedition.levels[size_t(Upgrade::RoverFactory)] = 1;
      s.expedition.roverBatch = 128;
      s.frontier.stepRovers(s.world, s.expedition, 0);
      std::array<int, 8> sectors{};
      for (const auto& r : s.frontier.rovers) {
        float a = std::atan2(r.position.x, r.position.z) + PI;
        ++sectors[std::min(7, int(a * 8 / (2 * PI)))];
      }
      for (int n : sectors)
        require(n >= 12, "Factory output is not spread around the whole core");
      s.expedition.levels[size_t(Upgrade::RoverFactory)] = 0;
      s.frontier.rovers.clear();
      for (uint32_t i = 0; i < 40; ++i) {
        Rover r{{50, .4f, 0}};
        r.serial = i;
        s.frontier.rovers.push_back(r);
      }
      for (int i = 0; i < 120; ++i)
        s.frontier.stepRovers(s.world, s.expedition, 1.f / 60);
      float minZ = 1e9f, maxZ = -1e9f;
      for (const auto& r : s.frontier.rovers) {
        minZ = std::min(minZ, r.position.z);
        maxZ = std::max(maxZ, r.position.z);
        require(std::isfinite(r.position.x), "Traffic steering produced invalid coordinates");
      }
      require(maxZ - minZ > 2, "Overlapping cars do not spread into separate tracks");
      s.frontier.rovers.clear();
      for (uint32_t i = 0; i < Frontier::MAX_ROVERS; ++i) {
        Rover r{{30 + float(i % 64) * 2, .4f, -32 + float(i / 64) * 2}};
        r.serial = i;
        s.frontier.rovers.push_back(r);
      }
      auto begin = std::chrono::steady_clock::now();
      for (int i = 0; i < 600; ++i)
        s.frontier.stepRovers(s.world, s.expedition, 1.f / 6000);
      double ms =
          std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin)
              .count() /
          600;
      require(s.frontier.rovers.size() == Frontier::MAX_ROVERS, "Traffic benchmark lost vehicles");
      std::cout << "PASS 360-degree launch, overlap separation; 2048 rovers CPU ms/step=" << ms
                << '\n';
    }
    {
      Session s(true, true);
      quiet(s);
      s.world.time = 599;
      s.frontier.stepWaves(s.world, .016f, s.spawnTimer);
      require(s.frontier.hordeSpawned == 0, "Hordes start before ten minutes");
      s.world.time = 600;
      for (int i = 0; i < 120; ++i)
        s.frontier.stepWaves(s.world, 1.f / 60, s.spawnTimer);
      require(s.frontier.hordeSpawned == 120 && s.frontier.pendingHorde == 0,
              "Ten-minute horde does not release 120 crawlers");
      for (int id : s.world.active)
        require(s.world.foes[id].type == Crawler && s.world.foes[id].hp == 100,
                "Horde contains strengthened or non-basic enemies");
      s.world.time = 660;
      s.frontier.respiteUntil = 670;
      s.frontier.stepWaves(s.world, .1f, s.spawnTimer);
      require(s.frontier.pendingHorde == 150 && s.frontier.hordeSpawned == 120,
              "Hordes do not scale or ignore boss respite");
      s.frontier.respiteUntil = 0;
      for (int i = 0; i < 120; ++i)
        s.frontier.stepWaves(s.world, 1.f / 60, s.spawnTimer);
      require(s.frontier.hordeSpawned == 270, "Later horde not released");
      while (s.world.active.size() < Frontier::MAX_ACTIVE)
        s.world.spawn(Crawler, 300, 0);
      s.world.time = 900;
      s.frontier.stepWaves(s.world, .1f, s.spawnTimer);
      require(s.world.active.size() == Frontier::MAX_ACTIVE, "Horde exceeds active population cap");
      std::cout << "PASS 10-minute hordes, growth, 100 HP, boss respite and population cap\n";
    }
    std::cout << "ALL FRONTIER TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
