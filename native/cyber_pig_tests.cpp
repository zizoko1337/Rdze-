#include "command_game.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool ok, const char* why) {
  if (!ok)
    throw std::runtime_error(why);
}
void tick(Session& s, float seconds) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60);
}
void quiet(Session& s) {
  s.mode = Mode::Walking;
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
  s.expedition.miningMultiplier = 0;
}
int enemy(Game& g, Vec3 point) {
  int id = g.spawn(Crawler, point.x, point.z);
  g.foes[id].hp = g.foes[id].maxHp = 100000;
  g.foes[id].speed = 0;
  g.grid.rebuild(g.foes, g.active);
  return id;
}
int main() {
  try {
    {
      require(cyberPigFacing({1, 0, 0}, {1, 0, 0}) == 1 &&
                  cyberPigFacing({-1, 0, 0}, {1, 0, 0}) == -1 &&
                  cyberPigFacing({1, 0, 0}, {-1, 0, 0}) == -1 &&
                  cyberPigFacing({0, 0, 1}, {0, 0, -1}) == -1,
              "Pig profile does not face movement in rotated cameras");
      CyberPigSystem pool;
      for (auto slot : TURRET_SLOTS) {
        require(pool.release({slot.x, 0, slot.z}), "Valid outer grid cannot receive pigs");
        pool.pigs.clear();
      }
    }
    {
      Session s(true, true);
      quiet(s);
      require(!s.expedition.eligible(Upgrade::ServiceRecharge) &&
                  !s.expedition.eligible(Upgrade::ServiceStorage),
              "Locked terminal rolls service upgrades");
      require(s.debugGrant(Upgrade::ServiceCredit, Rarity::Rare) &&
                  !s.debugGrant(Upgrade::ServiceCredit, Rarity::Rare),
              "Unlock repeats");
      auto& bank = s.expedition.support;
      require(bank.total() == 3 && bank.seconds() == 240,
              "Incorrect initial service stock or recharge");
      require(bank.consume(Service::Brimstone), "Cannot consume charged service");
      s.servicesOpen = true;
      tick(s, 2);
      s.servicesOpen = false;
      require(bank.progress[0] == 0, "Menu recharges services");
      tick(s, 120);
      require(bank.charges[0] == 0 && std::abs(bank.progress[0] - .5) < .0001,
              "Four-minute recharge inaccurate");
      s.world.paused = true;
      tick(s, 2);
      s.world.paused = false;
      require(std::abs(bank.progress[0] - .5) < .0001, "Pause recharges services");
      require(s.debugGrant(Upgrade::ServiceRecharge, Rarity::Common) && bank.seconds() < 240 &&
                  std::abs(bank.progress[0] - .5) < .0001,
              "Rate upgrade resets progress");
      for (int i = 0; i < 4; ++i)
        require(s.debugGrant(Upgrade::ServiceRecharge, Rarity::Legendary), "Rate upgrade missing");
      require(!s.debugGrant(Upgrade::ServiceRecharge, Rarity::Legendary), "Recharge cap missing");
      for (int i = 0; i < 3; ++i)
        require(s.debugGrant(Upgrade::ServiceStorage, Rarity::Rare), "Storage upgrade missing");
      require(bank.capacity() == 4 && !s.debugGrant(Upgrade::ServiceStorage, Rarity::Rare),
              "Storage cap missing");
      bank.step(bank.seconds() * 5);
      require(bank.total() == 12 && bank.progress[0] == 0,
              "Stock fails to accumulate or overflows");
      std::cout << "PASS unique unlock, gated cards, real recharge, pause, upgrades, retained "
                   "progress and four-charge caps\n";
    }
    {
      Session s(true, true);
      quiet(s);
      s.debugGrant(Upgrade::ServiceCredit, Rarity::Rare);
      int slot = nearestTurretSlot(60, 0);
      auto p = TURRET_SLOTS[slot];
      s.world.addTower(Laser, p.x, p.z);
      s.servicesOpen = true;
      require(s.chooseService(2), "Cannot select pig drop");
      require(!s.targetService({}, slot) && s.expedition.support.charges[2] == 1,
              "Occupied hex accepted");
      s.world.removeTower(p.x, p.z);
      s.sentryDrops.push_back({slot, 0, false, Laser});
      require(!s.targetService({}, slot), "Incoming turret ignored");
      s.sentryDrops.clear();
      require(!s.targetService({}, -1) && s.targetService({}, slot), "Empty grid target rejected");
      require(!s.slotAvailable(slot) && s.expedition.support.charges[2] == 0,
              "Drop does not reserve hex and charge");
      s.expedition.support.charges[2] = 1;
      s.servicesOpen = true;
      require(s.chooseService(2), "Cannot select another drop");
      require(!s.targetService({}, slot) && s.expedition.support.charges[2] == 1,
              "Duplicate cage accepted");
      s.cancelService();
      s.servicesOpen = false;
      tick(s, 5.9f);
      require(s.services.cyberPigs.pigs.empty(), "Pigs released in midair");
      tick(s, .2f);
      require(s.services.cyberPigs.pigs.empty(), "Pigs release immediately on landing");
      float age = s.services.casts.front().age;
      s.world.paused = true;
      tick(s, 2);
      s.world.paused = false;
      require(s.services.casts.front().age == age, "Paused cage descends");
      tick(s, 2);
      require(s.services.cyberPigs.pigs.size() == 2 && s.slotAvailable(slot),
              "Release or grid reservation timing wrong");
      for (auto& pig : s.services.cyberPigs.pigs)
        require(std::abs(pig.radius - std::hypot(p.x, p.z)) < .001,
                "Selected grid radius not preserved");
      require(s.world.towers.empty(), "Pig drop creates or sacrifices tower");
      std::cout << "PASS empty grid, incoming tower and cage exclusion, 6s landing, 2s delay, "
                   "pause and two-pig release\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      CyberPigSystem system;
      require(system.release({60, 0, 0}), "Cannot release pigs");
      auto& pig = system.pigs[0];
      system.pigs[1].escaping = true;
      pig.shotClock = 1000;
      int id = enemy(g, pig.position + pig.direction * 1);
      system.step(g, .02f);
      require(g.foes[id].hp == 98500 && system.contactHits == 1,
              "Contact is not exactly 1500 damage");
      for (int n = 0; n < 5; ++n)
        system.step(g, .02f);
      require(g.foes[id].hp == 98500, "Contact damages each simulation tick");
      pig.travelled = 2 * 3.14159265358979323846 - .01;
      system.step(g, .05f);
      require(g.foes[id].hp == 97000, "Next lap cannot contact same enemy");
      g.foes[id].generation++;
      system.step(g, .01f);
      require(g.foes[id].hp == 95500, "Reused foe generation inherits old contact immunity");
      std::cout
          << "PASS exact contact damage, once per lap, generation reuse and swept collision\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      CyberPigSystem system;
      system.release({80, 0, 0});
      auto& pig = system.pigs[0];
      system.pigs[1].escaping = true;
      int id = enemy(g, pig.position + pig.direction * 30);
      g.foes[id].altitude = 5;
      system.step(g, .01f);
      require(g.foes[id].hp == 99800 && system.laserShots == 1 && !g.beams.empty(),
              "Pigs do not fire real lasers");
      require(!g.particles.empty(), "Running pigs leave no dust");
      Vec3 pos = pig.position;
      g.paused = true;
      system.step(g, 1);
      g.paused = false;
      require(length(pig.position - pos) == 0, "Paused pig moves");
      std::cout << "PASS actual laser damage, airborne targets, dust and pause\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      CyberPigSystem system;
      system.release({60, 0, 0});
      int steps = 0;
      while (system.completedTours < 2 && steps++ < 20000) {
        system.step(g, 1.f / 60);
        for (auto& pig : system.pigs)
          if (!pig.escaping)
            require(std::abs(std::hypot(pig.position.x, pig.position.z) - 60) < .001,
                    "Pig leaves its orbital ring early");
      }
      require(system.completedTours == 2, "Five laps never finish");
      float expected = float(CyberPigSystem::FULL_TOUR * 60 / CyberPigSystem::SPEED);
      require(std::abs(steps / 60.f - expected) < .04, "Pigs do not complete exactly five laps");
      system.step(g, 1);
      for (auto& pig : system.pigs)
        require(std::hypot(pig.position.x, pig.position.z) > 60, "Escape does not head outward");
      for (int n = 0; n < 1800; ++n)
        system.step(g, .01f);
      require(system.pigs.empty(), "Escaping pigs never despawn");
      for (int n = 0; n < 32; ++n)
        require(system.release({60, 0, 0}), "Pig pool rejects valid pair");
      require(!system.release({60, 0, 0}), "Unbounded pig pool");
      for (int n = 0; n < 6000; ++n)
        enemy(g, {80 + float(n % 60) * .2f, 0, float(n / 60) * .2f});
      auto start = std::chrono::steady_clock::now();
      system.step(g, 1.f / 60);
      auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                    .count();
      std::cout << "PASS five exact laps, outward escape, cleanup, 64-pig cap; 6000-foe step=" << ms
                << "ms\n";
    }
    std::cout << "ALL CYBER PIG AND SERVICE RECHARGE TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
