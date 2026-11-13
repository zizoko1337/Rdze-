#include "command_game.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace command;
void require(bool value, const char* error) {
  if (!value)
    throw std::runtime_error(error);
}
void tick(Session& s, float seconds) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60);
}
void quiet(Session& s) {
  s.mode = Mode::Walking;
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
}
int enemy(Session& s, Species type, float x, float z) {
  int id = s.world.spawn(type, x, z);
  s.world.foes[id].hp = s.world.foes[id].maxHp = 10000;
  s.world.foes[id].speed = 0;
  s.world.grid.rebuild(s.world.foes, s.world.active);
  return id;
}
void menu(Session& s, int points) {
  s.terminalOpen = false;
  s.servicesOpen = true;
  s.expedition.servicePoints = points;
}
int main() {
  try {
    {
      Session s(true);
      s.start(Gun::Pulse);
      tick(s, 10);
      require(s.world.time == 0 && s.expedition.uranium == 0 && s.world.active.empty(),
              "Simulation starts before physical button");
      s.player = {0, CORE_HEIGHT + 1.72f, 5.8f};
      require(s.nearStartButton() && s.interact() && s.expedition.miningStarted && s.buttonAge == 0,
              "Button cannot start extraction");
      require(!s.startExtraction() && s.world.active.empty() && s.spawnTimer == .5f,
              "Button starts twice or creates an immediate enemy pack");
      tick(s, 1);
      require(s.world.active.size() == 1, "Opening frontier does not spawn enemies gradually");
      const auto& first = s.world.foes[s.world.active.front()];
      require(std::hypot(first.x, first.z) >= Frontier::SPAWN_MIN - 4 &&
                  std::hypot(first.x, first.z) <= Frontier::SPAWN_MAX,
              "First enemy does not approach from the frontier");
      float age = s.buttonAge;
      s.world.paused = true;
      tick(s, 1);
      require(s.buttonAge == age, "Button animation ignores pause");
      s.world.paused = false;
      tick(s, 1.6f);
      require(s.buttonAge > 2.3f && s.expedition.uranium > 5 && !s.nearStartButton(),
              "Button does not retract / mining stays stopped");
      s.player = {-6.8f, CORE_HEIGHT + 1.72f, -6.2f};
      s.yaw = 0;
      s.pitch = -.08f;
      require(s.nearTerminal() && !s.nearServices() && s.interact() && s.terminalOpen &&
                  !s.servicesOpen,
              "Left console is not upgrades-only");
      s.interact();
      s.player.x = 6.8f;
      require(s.nearServices() && !s.nearTerminal() && s.interact() && s.servicesOpen &&
                  !s.terminalOpen,
              "Right console is not services-only");
      require(!s.chooseUpgrade(0), "Services console consumes upgrade reward");
      std::cout << "PASS manual start, idle preparation, gradual frontier arrivals, animated-button clock "
                   "and separate consoles\n";
    }
    {
      Expedition e(true, true);
      int credits = 0;
      for (int i = 0; i < 5000; ++i) {
        e.addReward();
        for (int c = 0; c < 3; ++c)
          if (e.rewards.back().choices[c] == Upgrade::ServiceCredit) {
            ++credits;
            require(e.rewards.back().rarities[c] == Rarity::Rare,
                    "Service point is not exactly Rare");
          }
        e.rewards.clear();
      }
      require(credits > 0, "Rare service point is unreachable");
      Session s(true, true);
      quiet(s);
      s.expedition.rewards.push_back({1,
                                      {Upgrade::ServiceCredit, Upgrade::Hull, Upgrade::Mining},
                                      {Rarity::Rare, Rarity::Common, Rarity::Common}});
      s.terminalOpen = true;
      require(s.chooseUpgrade(0) && s.expedition.servicePoints == 1 && s.expedition.rewards.empty(),
              "Credit card does not grant one point");
      menu(s, 1);
      require(s.chooseService(0), "Brimstone selection failed");
      float time = s.world.time;
      tick(s, 3);
      require(s.world.time == time && s.expedition.servicePoints == 1,
              "Targeting advances time or spends point early");
      require(!s.targetService({std::numeric_limits<float>::quiet_NaN(), 0, 0}),
              "Invalid target accepted");
      s.cancelService();
      require(s.servicesOpen && s.expedition.servicePoints == 1, "Cancel loses credit");
      menu(s, 0);
      require(!s.chooseService(1), "Free service accepted");
      std::cout << "PASS Rare-only credit, one-point reward, free targeting cancellation and "
                   "invalid inputs\n";
    }
    {
      Session s(true, true);
      quiet(s);
      int center = enemy(s, Titan, 42, 36), near = enemy(s, Crawler, 50, 36),
          outside = enemy(s, Crawler, 53, 36);
      for (int id : {center, near, outside})
        s.world.foes[id].hp = s.world.foes[id].maxHp = 100000;
      menu(s, 1);
      require(s.chooseService(0) && s.targetService({42, 0, 36}), "Cannot order targeted beam");
      require(s.expedition.servicePoints == 0, "Brimstone did not charge one point");
      tick(s, 2.9f);
      require(s.world.foes[center].hp == 100000, "Beam damages before 3-second warning");
      tick(s, .3f);
      float hp = s.world.foes[center].hp, age = s.services.casts.front().age;
      require(hp == 90000 && s.world.brimstonePulses == 1, "Beam first 10000-damage pulse missing");
      s.world.paused = true;
      tick(s, 2);
      require(s.world.foes[center].hp == hp && s.services.casts.front().age == age,
              "Pause advances beam / damage");
      s.world.paused = false;
      tick(s, 3.1f);
      require(std::abs(s.world.foes[center].hp - 40000) < 1 && s.world.brimstonePulses == 6 &&
                  std::abs(s.world.foes[near].hp - 40000) < 1 &&
                  s.world.foes[outside].hp == 100000 && s.services.casts.empty(),
              "Beam does not deal six 10000-damage pulses in 10m");
      std::cout
          << "PASS Brimstone delay, six 10000-damage pulses, 10m radius, pause and completion\n";
    }
    {
      Session s(true, true);
      quiet(s);
      s.expedition.hp = s.expedition.maxHp = 100000;
      int spitter = enemy(s, Spitter, 48, 0), queen = enemy(s, Queen, 0, 150);
      s.world.foes[spitter].ability = s.world.foes[queen].ability = 0;
      menu(s, 1);
      require(s.chooseService(1) && !s.tactical, "Wave unnecessarily requests map target");
      tick(s, 2.9f);
      require(s.world.foes[spitter].stun == 0, "Early stun");
      tick(s, .2f);
      require(s.world.foes[spitter].stun > 4.8f && s.world.foes[queen].stun > 4.8f,
              "Wave misses ranged enemy / boss");
      float damage = s.world.coreDamage, ability = s.world.foes[queen].ability;
      s.world.foes[spitter].poison = 2;
      s.world.foes[spitter].poisonDps = 10;
      float hp = s.world.foes[spitter].hp;
      tick(s, 4.7f);
      require(s.world.coreDamage == damage && s.world.foes[queen].ability == ability &&
                  s.world.foes[spitter].hp < hp,
              "Stunned actors attack/summon or DOT stops");
      tick(s, .5f);
      require(s.world.foes[spitter].stun == 0 && s.world.coreDamage > damage,
              "Actors do not resume after 5-second stun");
      std::cout << "PASS global delayed stun, boss inclusion, disabled attacks/abilities, active "
                   "DOT and expiry\n";
    }
    {
      Session s(true, true);
      quiet(s);
      int slot = nearestTurretSlot(0, 36);
      const auto& p = TURRET_SLOTS[slot];
      int supportSlot = nearestTurretSlot(p.x + SUPPORT_SPACING, p.z);
      const auto& q = TURRET_SLOTS[supportSlot];
      s.world.addTower(Laser, p.x, p.z);
      s.world.towers.back().cooldown = 1000;
      s.world.addTower(RateRelay, q.x, q.z);
      s.expedition.levels[size_t(Upgrade::Sentry)] = 1;
      s.expedition.levels[size_t(Upgrade::RateRelayTower)] = 1;
      require(s.world.towers.front().localRate == 1.2f, "Support fixture failed");
      int victim = enemy(s, Titan, q.x, q.z), outside = enemy(s, Crawler, q.x + 40.5f, q.z);
      s.world.foes[victim].hp = s.world.foes[victim].maxHp = 100000;
      menu(s, 2);
      require(s.chooseService(2), "Cannot select overload");
      require(!s.targetService({0, 0, 0}, 0) && s.expedition.servicePoints == 2,
              "Empty target consumes point");
      require(s.targetService({q.x, 0, q.z}, supportSlot) && s.expedition.servicePoints == 1,
              "Cannot target installed support");
      s.servicesOpen = true;
      require(s.chooseService(2), "Cannot select second overload");
      require(!s.targetService({q.x, 0, q.z}, supportSlot) && s.expedition.servicePoints == 1,
              "Duplicate overload reservation accepted");
      s.cancelService();
      s.servicesOpen = false;
      tick(s, 3.1f);
      require(s.world.towers.size() == 1 && s.world.towers.front().localRate == 1 &&
                  s.slotAvailable(supportSlot),
              "Overload does not remove support and rebuild neighbours");
      require(s.world.foes[victim].hp == 50000 && s.world.foes[outside].hp == 10000 &&
                  s.world.overloadBlasts == 1,
              "Overload 50000 damage / 40m radius wrong");
      require(s.expedition.chosenTypes(true) == 1 && s.expedition.eligible(Upgrade::RateRelayTower),
              "Destroyed type forgotten or cannot be reacquired");
      s.servicesOpen = true;
      require(s.chooseService(2) && s.targetService({p.x, 0, p.z}, slot),
              "Cannot sacrifice final combat tower");
      tick(s, 3.1f);
      require(s.world.towers.empty() && s.expedition.acquiredTowers() == 0 &&
                  s.expedition.chosenTypes(false) == 1 && s.expedition.eligible(Upgrade::Sentry),
              "Final sacrifice does not free slot / retain combat technology");
      std::cout
          << "PASS overload target validity, reservation, 50000 damage / 40m, support removal "
             "and retained technology\n";
    }
    {
      Session s(true, true);
      quiet(s);
      s.expedition.servicePoints = 2;
      for (int index : {0, 1}) {
        auto p = TURRET_SLOTS[index];
        s.world.addTower(Laser, p.x, p.z);
        s.services.order(s.expedition, Service::Overload, {p.x, 0, p.z}, s.world.towers.back().id);
      }
      tick(s, 3.1f);
      require(s.world.towers.empty(), "Concurrent overload uses stale vector index");
      auto p = TURRET_SLOTS[0];
      s.world.addTower(Laser, p.x, p.z);
      s.expedition.servicePoints = 1;
      s.services.order(s.expedition, Service::Overload, {p.x, 0, p.z}, s.world.towers.back().id);
      s.world.removeTower(p.x, p.z);
      s.world.addTower(Laser, p.x, p.z);
      tick(s, 3.1f);
      require(s.world.towers.size() == 1 && s.expedition.servicePoints == 1,
              "Invalidated overload destroys replacement or loses point");
      std::cout
          << "PASS stable tower IDs, simultaneous sacrifices and refund for invalidated target\n";
    }
    std::cout << "ALL SERVICES TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
