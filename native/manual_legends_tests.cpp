#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool b, const char* why) {
  if (!b)
    throw std::runtime_error(why);
}
void setup(Session& s, Gun gun) {
  s.start(gun);
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  for (int id : s.world.active)
    s.world.foes[id].hp = 0;
  s.world.step(.01f);
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
}
int enemy(Session& s, float x, float z, Species type = Crawler) {
  int id = s.world.spawn(type, x, z);
  auto& e = s.world.foes[id];
  e.hp = e.maxHp = 1000000;
  e.speed = 0;
  s.world.grid.rebuild(s.world.foes, s.world.active);
  return id;
}
void tick(Session& s, float seconds, bool firing = false) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60, {0, 0, false, firing});
}
void aim(Session& s, Vec3 point) {
  for (int i = 0; i < 5; ++i) {
    Vec3 d = normalize(point - s.eye());
    s.yaw = s.gunYaw = std::atan2(d.x, d.z);
    s.pitch = s.gunPitch = std::asin(d.y);
  }
}
int main() {
  try {
    constexpr Upgrade upgrades[] = {Upgrade::TeslaAmplifier,   Upgrade::PlasmaGlue,
                                    Upgrade::BouncyMortar,     Upgrade::InstantLock,
                                    Upgrade::DoubleRangeBonus, Upgrade::DualFlame};
    for (int gun = 0; gun < GUN_COUNT; ++gun) {
      Session s(true, true);
      setup(s, Gun(gun));
      for (auto u : upgrades) {
        bool matches = requiredManualGun(u) == Gun(gun);
        require(s.expedition.eligible(u) == matches, "Wrong manual legend in reward pool");
        require(minimumRarity(u) == Rarity::Legendary && uniqueUpgrade(u),
                "Legend rarity or uniqueness wrong");
        require(s.debugGrant(u, Rarity::Common) == matches, "Debug grant bypasses wrong-gun rule");
        if (matches)
          require(!s.debugGrant(u, Rarity::Legendary), "Legend can be acquired twice");
      }
    }
    std::cout
        << "PASS six Legendary cards, starter eligibility, actual installation and uniqueness\n";
    {
      Session s;
      setup(s, Gun::Arc);
      s.debugGrant(Upgrade::TeslaAmplifier, Rarity::Legendary);
      s.world.addTower(Laser, 0, 60);
      s.world.addTower(Minigun, 21, 60);
      s.world.addTower(RateRelay, -21, 60);
      s.world.addTower(Laser, 110, 60);
      float original = s.world.weaponInfo(s.world.towers[0]).cooldown;
      int id = enemy(s, 0, 85);
      require(s.toggleManualMode() && s.teslaSupport, "Tesla switch fails");
      aim(s, {0, 3, 60});
      s.fire();
      require(s.world.towers[0].amplifiedUntil > s.world.time &&
                  s.world.towers[1].amplifiedUntil > s.world.time &&
                  s.world.towers[2].amplifiedUntil > s.world.time &&
                  s.world.towers[3].amplifiedUntil == 0,
              "Friendly chain range or targets wrong");
      require(s.world.foes[id].hp == 1000000 &&
                  s.world.weaponInfo(s.world.towers[0]).cooldown == original * .5f,
              "Support mode damages enemy or fails +100 percent rate");
      s.cooldown = 0;
      s.fire();
      require(s.world.weaponInfo(s.world.towers[0]).cooldown == original * .5f,
              "Buff stacks beyond x2");
      s.world.paused = true;
      float time = s.world.time;
      tick(s, 2);
      require(s.world.time == time, "Pause consumes amplification");
      s.world.paused = false;
      tick(s, Session::TESLA_BUFF_DURATION + .1f);
      require(s.world.weaponInfo(s.world.towers[0]).cooldown == original,
              "Amplification fails to expire");
      require(s.toggleManualMode() && !s.teslaSupport, "Cannot switch back to attack");
      std::cout
          << "PASS friendly Tesla chain, range, support stacking, refresh, pause and expiry\n";
    }
    {
      Session s;
      setup(s, Gun::Toxin);
      int id = enemy(s, 0, 80, Brain);
      s.world.foes[id].speed = 10;
      s.toxicPatches.push_back({{0, .2f, 80}, 8, 0, 65, true});
      tick(s, .2f);
      float z = s.world.foes[id].z;
      tick(s, 1);
      require(std::abs(s.world.foes[id].z - z) < .001f && s.world.foes[id].glued > 0 &&
                  s.world.foes[id].toxicSlow == 0 && s.world.foes[id].hp < 1000000,
              "Glue does not root boss independently of poison");
      s.toxicPatches.clear();
      tick(s, .4f);
      require(s.world.foes[id].z < z - 1, "Root remains after leaving glue");
      std::cout << "PASS glue roots boss, keeps poison, replaces slow and releases outside patch\n";
    }
    {
      Session s;
      setup(s, Gun::Howitzer);
      int middle = enemy(s, 0, 112);
      ManualProjectile p{Gun::Howitzer, {0, 5, 80}, {0, 5, 80}, {0, 5, 80}, {0, .2f, 100}, {}};
      p.damage = 1100;
      p.duration = .1f;
      p.bounces = 2;
      p.bounceDirection = {0, 0, 1};
      s.manualProjectiles.push_back(p);
      for (int n = 0; n < 6; ++n)
        s.stepManualProjectiles(1.f / 60);
      require(s.manualProjectiles.size() == 1 && s.manualProjectiles[0].bounces == 1,
              "First bounce missing");
      for (int n = 0; n < 80; ++n)
        s.stepManualProjectiles(1.f / 60);
      require(s.manualProjectiles.empty() && std::abs(s.world.foes[middle].hp - 996700) < .1f,
              "Mortar does not produce exactly three full-damage impacts");
      std::cout
          << "PASS one projectile pool slot, forward bounce motion and three 1100-damage blasts\n";
    }
    {
      Session s;
      setup(s, Gun::Smart);
      int a = enemy(s, 0, 100), b = enemy(s, 100, 100);
      aim(s, s.world.targetPoint(a));
      s.fire();
      require(s.shots == 0 && s.smartCandidate == a, "Base Smart fires before acquiring");
      tick(s, .45f);
      s.fire();
      require(s.shots == 0, "Smart lock takes less than half a second");
      tick(s, .07f);
      s.fire();
      require(s.shots == 1 && s.smartTarget == a, "Half-second lock never completes");
      aim(s, s.world.targetPoint(b));
      s.updateSmartTarget();
      require(s.smartTarget == -1 && s.smartCandidate == b && s.smartLock == 0,
              "Switching target keeps old lock");
      s.world.paused = true;
      tick(s, 1);
      s.world.paused = false;
      require(s.smartLock == 0, "Pause advances acquisition");
      s.debugGrant(Upgrade::InstantLock, Rarity::Legendary);
      s.cooldown = 0;
      s.fire();
      require(s.shots == 2 && s.smartTarget == b,
              "Instant upgrade does not remove acquisition time");
      std::cout << "PASS default 0.5s lock, candidate reset, pause and actual instant upgrade\n";
    }
    {
      Session s;
      setup(s, Gun::Rail);
      s.debugGrant(Upgrade::DoubleRangeBonus, Rarity::Legendary);
      require(s.sniperDamageAt(100) == 2250 && s.sniperDamageAt(200) == 3750,
              "Doubled distance coefficient wrong");
      int id = enemy(s, 0, 100);
      aim(s, s.world.targetPoint(id));
      float damage = s.sniperDamageAt(length(s.world.targetPoint(id) - s.eye()));
      s.fire();
      require(std::abs(s.world.foes[id].hp - (1000000 - damage)) < .1f,
              "Sniper actual hit disagrees with telemetry");
      std::cout << "PASS 2 percent per metre and actual sniper damage\n";
    }
    {
      Session s;
      setup(s, Gun::Flame);
      s.debugGrant(Upgrade::DualFlame, Rarity::Legendary);
      s.pitch = s.gunPitch = 0;
      int both = enemy(s, 0, 50), wide = enemy(s, 12, 50), focused = enemy(s, 0, 90);
      for (int id : {both, wide, focused})
        s.world.foes[id].altitude = s.eye().y - s.world.foes[id].size * .47f;
      require(!s.toggleManualMode(), "Dual flame still exposes mode switch");
      s.fire();
      require(s.world.foes[both].hp == 999952 && s.world.foes[wide].hp == 999976 &&
                  s.world.foes[focused].hp == 999976,
              "Two simultaneous flame cones do not deal independent damage");
      std::cout << "PASS dual flame wide/focused coverage, overlapping damage and removed switch\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
