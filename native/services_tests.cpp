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
  s.expedition.support.unlock();
  s.expedition.support.charges.fill(points);
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
      require(!s.nearServices() && !s.interact(), "Unpowered service terminal opens");
      require(s.debugGrant(Upgrade::ServiceCredit, Rarity::Rare), "Cannot unlock service terminal");
      require(s.nearServices() && !s.nearTerminal() && s.interact() && s.servicesOpen &&
                  !s.terminalOpen,
              "Right console is not services-only");
      require(!s.chooseUpgrade(0), "Services console consumes upgrade reward");
      std::cout << "PASS manual start, idle preparation, gradual frontier arrivals, "
                   "animated-button clock "
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
      require(s.chooseUpgrade(0) && s.expedition.support.total() == 3 &&
                  s.expedition.rewards.empty(),
              "Unlock does not ready each service independently");
      menu(s, 1);
      require(s.chooseService(0), "Brimstone selection failed");
      float time = s.world.time;
      tick(s, 3);
      require(s.world.time == time && s.expedition.support.charges[0] == 1,
              "Targeting advances time or spends point early");
      require(!s.targetService({std::numeric_limits<float>::quiet_NaN(), 0, 0}),
              "Invalid target accepted");
      s.cancelService();
      require(s.servicesOpen && s.expedition.support.charges[0] == 1, "Cancel loses charge");
      menu(s, 0);
      require(!s.chooseService(1), "Free service accepted");
      std::cout << "PASS Rare-only terminal unlock, independent stocks, free targeting cancellation and "
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
      require(s.expedition.support.charges[0] == 0 && s.expedition.support.charges[1] == 1,
              "Brimstone consumes the wrong service inventory");
      tick(s, 2.9f);
      require(s.world.foes[center].hp == 100000, "Beam damages before 3-second warning");
      tick(s, .3f);
      float hp = s.world.foes[center].hp, age = s.services.casts.front().age;
      require(std::abs(hp-(100000-ServiceSystem::brimstonePulse(0))) < .02f &&
                  s.world.brimstonePulses == 1, "First Brimstone pulse incorrect");
      s.world.paused = true;
      tick(s, 2);
      require(s.world.foes[center].hp == hp && s.services.casts.front().age == age,
              "Pause advances beam / damage");
      s.world.paused = false;
      tick(s, 3.1f);
      require(std::abs(s.world.foes[center].hp - 75000) < 1 && s.world.brimstonePulses == 6 &&
                  std::abs(s.world.foes[near].hp - 75000) < 1 &&
                  s.world.foes[outside].hp == 100000 && s.services.casts.empty(),
              "Brimstone does not deal 25000 total damage in 10m");
      std::cout
          << "PASS Brimstone delay, 25000 total damage, 10m radius, pause and completion\n";
    }
    {
      Session s(true,true);quiet(s);
      int boss=enemy(s,Titan,100,0);
      s.world.foes[boss].hp=s.world.foes[boss].maxHp=100000;
      s.world.foes[boss].speed=8;
      menu(s,1);
      require(s.chooseService(0) && s.targetService({100,0,0}),"Cannot lock Brimstone onto boss");
      tick(s,6.1f);
      require(std::abs(s.world.foes[boss].hp-75000)<1,"Moving boss escapes tracked Brimstone");
      require(s.world.foes[boss].x<60,"Tracking fixture does not move far enough");
      std::cout << "PASS Brimstone follows a moving boss for the entire 25000-damage volley\n";
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
    std::cout << "ALL SERVICES TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
