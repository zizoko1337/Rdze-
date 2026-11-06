#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
void tick(Game& g, float seconds) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    g.step(1.f / 60);
}
void tick(Session& s, float seconds) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60);
}
bool acquire(Session& s, Upgrade u) {
  s.expedition.rewards.push_front({1,
                                   {u, Upgrade::Hull, Upgrade::Mining},
                                   {Rarity::Legendary, Rarity::Common, Rarity::Common}});
  s.terminalOpen = true;
  return s.chooseUpgrade(0);
}
int target(Game& g, float x, float z) {
  int id = g.spawn(Crawler, x, z);
  g.foes[id].hp = g.foes[id].maxHp = 1000000;
  g.foes[id].speed = 0;
  g.grid.rebuild(g.foes, g.active);
  return id;
}
int main() {
  try {
    {
      Session s(true, true);
      s.mode = Mode::Walking;
      for (auto u :
           {Upgrade::Sentry, Upgrade::MinigunTower, Upgrade::TeslaTower, Upgrade::SniperTower})
        require(acquire(s, u), "Combat type rejected before limit");
      require(s.expedition.chosenTypes(false) == 4 && !acquire(s, Upgrade::LanceTower),
              "Fifth combat type accepted");
      require(acquire(s, Upgrade::Sentry), "Existing combat type cannot repeat");
      require(acquire(s, Upgrade::RateRelayTower) && acquire(s, Upgrade::RangeRelayTower),
              "Combat limit blocks separate support pool");
      require(!acquire(s, Upgrade::FrostSupportTower) && s.expedition.chosenTypes(true) == 2,
              "Third support type accepted");
      require(acquire(s, Upgrade::RateRelayTower), "Existing support type cannot repeat");
      s.expedition.rewards.clear();
      for (int n = 0; n < 200; ++n) {
        s.expedition.addReward();
        for (auto u : s.expedition.rewards.back().choices)
          require(s.expedition.eligible(u), "Unselected type leaks through a capped pool");
      }
      s.expedition.rewards.front().choices = {Upgrade::LanceTower, Upgrade::FrostSupportTower,
                                              Upgrade::Sentry};
      s.expedition.validateFront();
      const auto& o = s.expedition.rewards.front().choices;
      require(!isTower(o[0]) && !isTower(o[1]) && o[2] == Upgrade::Sentry && o[0] != o[1],
              "Queued type-limited choices not repaired");
      std::cout << "PASS four combat / two support types, repeat copies and queued pool repair\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      require(g.addTower(RateRelay, 63, 0), "Cannot build relay");
      for (int i = 0; i < 6; ++i) {
        float a = i * PI / 3;
        require(
            g.addTower(Laser, 63 + std::cos(a) * SUPPORT_SPACING, std::sin(a) * SUPPORT_SPACING),
            "Cannot build neighbour");
      }
      require(g.supportLinks.size() == 6, "Relay does not link exactly six neighbours");
      for (int i = 1; i <= 6; ++i)
        require(g.towers[i].localRate == 1.2f && g.towers[i].localRange == 1,
                "Neighbour is missing rate bonus");
      require(g.addTower(Laser, 105, 0) && g.towers.back().localRate == 1,
              "Relay buffs a second-ring tower");
      // The tower at (84, 0) now has two rate relays and one range relay.
      require(g.addTower(RateRelay, 94.5f, 18.186533f), "Cannot build second relay");
      require(g.addTower(RangeRelay, 94.5f, -18.186533f), "Cannot build range relay");
      require(g.towers[1].localRate == 1.2f && g.towers[1].localRange == 1.2f,
              "Support types do not combine, or same type stacks");
      g.towerRange = 1.5f;
      require(std::abs(g.weaponInfo(g.towers[1]).range - 65 * 1.5f * 1.2f) < .001f,
              "Local range does not multiply existing upgrades");
      for (const auto& t : g.towers)
        if (isSupport(t.type))
          require(t.localRate == 1 && t.localRange == 1, "Supports buff each other");
      require(g.removeTower(63, 0) && g.towers[0].localRate == 1.2f,
              "Removing one relay discards the other relay's bonus");
      require(g.removeTower(94.5f, 18.186533f) && g.towers[0].localRate == 1,
              "Removing final rate relay leaves stale bonus");
      require(g.removeTower(94.5f, -18.186533f) && g.towers[0].localRange == 1,
              "Removing range relay leaves stale bonus");
      std::cout
          << "PASS exactly six hex neighbours, multiplicative buffs, no duplicates and removal\n";
    }
    {
      Game base(false), boosted(false), ranged(false);
      for (Game* g : {&base, &boosted, &ranged}) {
        g->autoWaves = false;
        g->addTower(Laser, 0, 42);
      }
      boosted.addTower(RateRelay, 21, 42);
      ranged.addTower(RangeRelay, 21, 42);
      target(base, 0, 92);
      target(boosted, 0, 92);
      int far = target(ranged, 0, 116); // 74 m, beyond base 65 m.
      tick(base, 20);
      tick(boosted, 20);
      tick(ranged, 1);
      require(boosted.fired[Laser] > base.fired[Laser] * 1.17 &&
                  boosted.fired[Laser] < base.fired[Laser] * 1.23,
              "20% rate support does not improve actual cadence");
      require(ranged.foes[far].hp < 1000000, "Range support does not extend actual firing range");
      Game lance(false);
      lance.autoWaves = false;
      lance.addTower(RateRelay, 21, 42);
      lance.addTower(Lance, 0, 42);
      target(lance, 0, 80);
      tick(lance, 5.2f);
      require(lance.fired[Lance] == 1, "Rate relay does not accelerate lance charging");
      std::cout << "PASS actual supported cadence, range and lance charge\n";
    }
    {
      Game one(false), two(false), combined(false), outside(false);
      for (Game* g : {&one, &two, &combined, &outside}) {
        g->autoWaves = false;
        g->addTower(FrostTower, 42, 0);
      }
      two.addTower(FrostTower, 63, 0);
      combined.addTower(FrostTower, 63, 0);
      combined.frost = true;
      int a = target(one, 60, 0), b = target(two, 60, 0), c = target(combined, 60, 0);
      int d = target(outside, 120, 0);
      one.foes[a].speed = two.foes[b].speed = combined.foes[c].speed = outside.foes[d].speed = 10;
      tick(one, 1);
      tick(two, 1);
      tick(combined, 1);
      tick(outside, 1);
      require(std::abs(one.foes[a].x - 55) < .002f &&
                  std::abs(one.foes[a].x - two.foes[b].x) < .002f,
              "Frost aura is not 50%, or duplicate frost auras stack");
      require(combined.foes[c].x > two.foes[b].x + .4f,
              "Core frost does not compound with support frost");
      require(std::abs(outside.foes[d].x - 110) < .002f, "Frost slows outside 40 m");
      require(one.removeTower(42, 0) && one.movementSlowAt(60, 0) == 1,
              "Deleted frost tower leaves cached aura");
      Game pulses(false);
      pulses.autoWaves = false;
      pulses.addTower(FrostTower, 42, 0);
      tick(pulses, 2.9f);
      require(pulses.particles.empty() && pulses.rings.empty(), "Early blizzard pulse");
      tick(pulses, .2f);
      require(pulses.particles.size() == 64 && !pulses.rings.empty(),
              "Missing 3-second snow pulse");
      pulses.paused = true;
      float phase = pulses.towers[0].phase;
      tick(pulses, 2);
      require(pulses.towers[0].phase == phase && pulses.particles.size() == 64,
              "Pause advances the blizzard");
      pulses.paused = false;
      tick(pulses, 3);
      require(pulses.particles.size() == 64 && pulses.fired[Laser] == 0,
              "Second pulse missing or support deals attack damage");
      std::cout << "PASS continuous 40 m / 50% slow, duplicate suppression, core stacking and snow "
                   "pulses\n";
    }
    {
      Session s(true, true);
      s.mode = Mode::Walking;
      s.spawnTimer = 100000;
      int slot = nearestTurretSlot(0, 36);
      const auto& p = TURRET_SLOTS[slot];
      s.world.addTower(Laser, p.x + SUPPORT_SPACING, p.z);
      require(acquire(s, Upgrade::RateRelayTower) && s.orderSentry(slot), "Cannot deliver support");
      tick(s, 5.8f);
      require(s.world.towers.size() == 1 && s.world.towers[0].localRate == 1,
              "Airborne relay already buffs neighbours");
      tick(s, .4f);
      require(s.world.towers.size() == 2 && s.world.towers[1].type == RateRelay &&
                  s.world.towers[0].localRate == 1.2f,
              "Relay landing does not activate support");
      std::cout
          << "PASS support inventory, typed balloon delivery and activation only after landing\n";
    }
    std::cout << "ALL SUPPORT TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
