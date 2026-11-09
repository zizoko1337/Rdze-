#include "upgrade_lab.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool yes, const char* error) {
  if (!yes)
    throw std::runtime_error(error);
}
void setup(Session& s) {
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  s.module.kind = Gun::Crank;
  s.expedition.manualLaser = false;
  s.expedition.manualMinigun = true;
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
}
void tick(Session& s, float seconds, bool fire = false, float dt = 1.f / 60) {
  for (int i = 0; i < int(std::round(seconds / dt)); ++i)
    s.step(dt, {0, 0, false, fire});
}
int main() {
  try {
    require(minimumRarity(Upgrade::ExtremeCooling) == Rarity::Legendary &&
                uniqueUpgrade(Upgrade::ExtremeCooling),
            "Cooling is not a unique Legendary upgrade");
    Session normal(true, true), cooled(true, true);
    setup(normal);
    setup(cooled);
    require(cooled.debugGrant(Upgrade::ExtremeCooling, Rarity::Legendary) &&
                cooled.module.extremeCooling,
            "Cooling does not install for minigun");
    require(!cooled.debugGrant(Upgrade::ExtremeCooling, Rarity::Legendary),
            "Cooling upgrade stacks");
    Session laser(true, true);
    require(!laser.expedition.eligible(Upgrade::ExtremeCooling) &&
                !laser.debugGrant(Upgrade::ExtremeCooling, Rarity::Legendary),
            "Laser receives minigun cooling");
    for (int i = 0; i < 500; ++i) {
      laser.expedition.addReward(RewardSource::BossCrystal);
      for (auto u : laser.expedition.rewards.back().choices)
        require(u != Upgrade::ExtremeCooling, "Laser rolls minigun conversion");
      laser.expedition.rewards.clear();
    }
    normal.fire();
    cooled.fire();
    Vec3 aim =
        normalize(trace(normal.world, normal.eye(), normal.forward(), 420).point - normal.muzzle());
    auto spread = [&](Vec3 velocity) {
      auto direction = normalize(velocity);
      float forward = dot(direction, aim);
      return length(direction - aim * forward) / forward;
    };
    require(
        std::abs(spread(cooled.bullets.front().velocity) / spread(normal.bullets.front().velocity) -
                 2) < .001f,
        "Actual minigun bullet spread is not twice as wide");
    require(normal.bullets.front().damage == 26 && cooled.bullets.front().damage == 26,
            "Cooling changes per-bullet damage");
    tick(normal, 1, true);
    tick(cooled, 1, true);
    auto beforeNormal = normal.shots, beforeCooled = cooled.shots;
    tick(normal, 2, true);
    tick(cooled, 2, true);
    require(normal.shots - beforeNormal == 24 && cooled.shots - beforeCooled == 120,
            "Cooling cadence is not 60/s against baseline 12/s");
    require(normal.barrelHeat > .25f && !normal.coolantActive,
            "Ordinary minigun does not gradually heat up");
    float hot = normal.barrelHeat;
    tick(normal, 2);
    require(normal.barrelHeat < hot, "Released minigun does not cool down");
    cooled.module.rateMultiplier = 1.5f;
    tick(cooled, .2f, true); // Let the shot already in cooldown finish at its old rate.
    beforeCooled = cooled.shots;
    tick(cooled, 2, true);
    require(cooled.shots - beforeCooled == 180,
            "Existing fire-rate bonuses do not multiply new base cadence");
    require(cooled.bullets.size() < 1024, "Minigun projectile pool is not bounded");
    Session spray(true, true);
    setup(spray);
    spray.debugGrant(Upgrade::ExtremeCooling, Rarity::Legendary);
    bool mist = false, coldAtStart = spray.coolantActive;
    for (int i = 0; i < 240; ++i) {
      spray.step(1.f / 60, {0, 0, false, true});
      mist |= spray.coolantActive &&
              std::any_of(spray.world.particles.begin(), spray.world.particles.end(),
                          [](auto& p) { return p.style == 1; });
    }
    require(!coldAtStart && mist, "Cryogenic mist does not switch on when barrels get hot");
    float frozen = spray.barrelHeat;
    spray.upgradeLabOpen = true;
    tick(spray, 1, true);
    require(spray.barrelHeat == frozen, "Heat ignores upgrade menu pause");
    spray.upgradeLabOpen = false;
    tick(spray, 3);
    require(!spray.coolantActive && spray.barrelHeat < .1f, "Cold idle barrels keep spraying");
    UpgradeLab lab;
    lab.text("cooling");
    require(lab.results().size() == 1 && lab.results()[0] == Upgrade::ExtremeCooling,
            "Cooling not searchable in catalogue");
    std::cout << "PASS Legendary eligibility, actual x2 spread, 12/60/90 rounds per second, heat, "
                 "cold mist, pause and bounded projectiles\n";
    std::cout << "ALL COOLING TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
