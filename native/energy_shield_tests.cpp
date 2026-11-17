#include "command_game.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool value, const char* message) {
  if (!value)
    throw std::runtime_error(message);
}
void tick(Session& s, float seconds) {
  for (int n = 0; n < int(std::round(seconds * 60)); ++n)
    s.step(1.f / 60);
}
int main() {
  try {
    {
      EnergyShield shield;
      require(shield.absorb(100) == 100 && !shield.online(), "Unpurchased shield absorbs damage");
      require(shield.order() && !shield.order(), "Generator can be ordered twice");
      for (int n = 0; n <= 1200; ++n) {
        shield.deliveryAge = n * .01f;
        auto drone = shield.dronePosition();
        float radius = std::hypot(drone.x, drone.z);
        if (radius < Session::ROOF_RADIUS + .9f)
          require(drone.y + .2f <
                      Session::roofHeight(std::min(Session::ROOF_RADIUS, radius + .9f)) - .18f,
                  "Compact drone clips the canopy underside");
        if (std::abs(radius - Session::RAIL_RADIUS) < .9f)
          require(drone.y - .15f > CORE_HEIGHT + 1.07f, "Drone hits the outer rail");
        for (int column = 0; column < 8; ++column) {
          float angle = column * PI / 4;
          require(std::hypot(drone.x - std::sin(angle) * 11.8f, drone.z - std::cos(angle) * 11.8f) >
                      1.1f,
                  "Delivery route crosses a roof column");
        }
        auto crate = shield.cratePosition();
        if (shield.deliveryAge < EnergyShield::LAND_TIME) {
          float r = std::hypot(crate.x, crate.z);
          if (r < Session::ROOF_RADIUS + .8f)
            require(crate.y >= CORE_HEIGHT + .27f &&
                        crate.y + 4.1f * EnergyShield::CRATE_SCALE <
                            Session::roofHeight(std::min(Session::ROOF_RADIUS, r + .8f)) - .18f,
                    "Carried cabinet intersects floor or canopy");
          if (std::abs(r - Session::RAIL_RADIUS) < .8f)
            require(crate.y > CORE_HEIGHT + 1.07f, "Cabinet catches on railing during entrance");
        }
      }
      shield.deliveryAge = 0;
      shield.step(7.99f);
      require(!shield.installed && shield.absorb(100) == 100, "Shield activates before delivery");
      shield.step(.02f);
      require(shield.installed && shield.starting() && !shield.online() && shield.hp == 0 &&
                  shield.antennaExtension() < .01f && shield.absorb(100) == 100,
              "Shield activates before its antennas and electrical arc");
      shield.step(.7f);
      require(shield.antennaExtension() > .4f && shield.antennaExtension() < 1 &&
                  shield.arcStrength() == 0,
              "Antenna extension or arc sequence wrong");
      shield.step(.8f);
      require(shield.antennaExtension() == 1 && shield.arcStrength() > 0 && !shield.online(),
              "Electrical arc does not start before shield activation");
      shield.step(.71f);
      require(shield.online() && shield.hp == 1000 && shield.arcStrength() == 1,
              "Startup fails to activate full shield after arc ignition");
      require(shield.absorb(400) == 0 && shield.hp == 600, "Shield does not absorb core damage");
      shield.step(10);
      require(shield.hp == 700, "Base regeneration is not 10HP/s");
      require(shield.absorb(900) == 200 && shield.hp == 0 && shield.rebuildRemaining == 60,
              "Break does not pass excess damage through or start one-minute cooldown");
      require(shield.absorb(30) == 30 && shield.rebuildRemaining == 60,
              "Offline hit resets cooldown");
      shield.step(59.9f);
      require(!shield.online() && shield.hp == 0, "Broken shield regenerates before timer ends");
      shield.step(.2f);
      require(shield.online() && shield.hp == 1000 && shield.rebuilds == 1,
              "Shield does not rebuild to full");
      shield.step(100);
      require(shield.hp == 1000, "Shield over-regenerates");
      std::cout << "PASS safe drone route, delivery gate, 1000HP, 10HP/s, overflow and exact "
                   "rebuild state\n";
    }
    {
      auto session = std::make_unique<Session>(true, true);
      auto& s = *session;
      s.mode = Mode::Walking;
      s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = s.frontier.nextHorde =
          s.frontier.mutations.nextMutation = 100000;
      require(fixedRarity(Upgrade::EnergyShield) == int(Rarity::Epic) &&
                  !s.expedition.eligible(Upgrade::EnergyShield, Rarity::Rare) &&
                  !s.expedition.eligible(Upgrade::ShieldCapacity),
              "Shield rarity or prerequisite wrong");
      require(s.debugGrant(Upgrade::EnergyShield, Rarity::Epic) &&
                  !s.debugGrant(Upgrade::EnergyShield, Rarity::Epic),
              "Epic unlock or uniqueness broken");
      tick(s, 11);
      require(s.expedition.shield.online(), "Actual purchase never delivers generator");
      require(std::hypot(EnergyShield::DOCK.x, EnergyShield::DOCK.z) < Session::WALK_RADIUS - 1,
              "Cabinet still lands outside the base");
      s.player = EnergyShield::DOCK + Vec3{0, 1.44f, 0};
      s.move({}, .01f);
      require(std::hypot(s.player.x - EnergyShield::DOCK.x, s.player.z - EnergyShield::DOCK.z) >=
                  1.149f,
              "Operator walks through installed cabinet");
      s.expedition.shield.hp = 500;
      auto age = s.expedition.shield.deliveryAge;
      float phase = s.expedition.shield.mechanismPhase;
      s.world.paused = true;
      tick(s, 2);
      s.world.paused = false;
      s.terminalOpen = true;
      tick(s, 2);
      s.terminalOpen = false;
      require(s.expedition.shield.hp == 500 && s.expedition.shield.deliveryAge == age &&
                  s.expedition.shield.mechanismPhase == phase,
              "Pause or terminal advances delivery/regeneration");
      for (auto upgrade :
           {Upgrade::ShieldCapacity, Upgrade::ShieldRegeneration, Upgrade::ShieldRebuild}) {
        for (int tier = 0; tier < 5; ++tier) {
          for (int rarity = 0; rarity < 5; ++rarity)
            require(s.expedition.eligible(upgrade, Rarity(rarity)) == (rarity == tier),
                    "Shield upgrade skips or repeats rarity tiers");
          require(s.debugGrant(upgrade, Rarity(tier)), "Shield stat upgrade cannot be installed");
        }
        require(!s.expedition.eligible(upgrade), "Shield stat path has no five-choice cap");
      }
      auto& shield = s.expedition.shield;
      require(shield.maxHp == 3000 && shield.regeneration == 26 && shield.rebuildDuration == 20 &&
                  shield.hp == 500,
              "Shield upgrade totals wrong or capacity grants free healing");
      shield.absorb(500);
      tick(s, 19);
      require(!shield.online(), "Upgraded cooldown too short");
      tick(s, 1.1f);
      require(shield.online() && shield.hp == shield.maxHp, "Upgraded cooldown fails to recover");
      std::cout << "PASS Epic unique delivery, active-clock pause, five ordered stat tiers and "
                   "real 3000/26/20 upgrade totals\n";
    }
    {
      Expedition e(true, true);
      e.regeneration = 0;
      e.shield.order();
      e.shield.step(8 + EnergyShield::STARTUP_TIME + .01f);
      e.step(.1f, 600);
      require(e.hp == 1500 && e.shield.hp == 400 && e.lastHullDamage == 0,
              "Shielded damage reaches hull");
      e.step(.1f, 601);
      require(std::abs(e.hp - 1300) < .01f && e.shield.rebuildRemaining == 60,
              "Hull overflow or break ordering wrong");
      e.step(.1f, 1400);
      require(e.dead() && e.hp == 0, "Offline shield prevents lethal core damage");
      e.step(100, 0);
      require(e.hp == 0 && e.shield.rebuildRemaining == 59.9f, "Death permits shield resurrection");
      EnergyShield rebuilt;
      rebuilt.order();
      rebuilt.step(8 + EnergyShield::STARTUP_TIME + .01f);
      rebuilt.absorb(1000);
      rebuilt.step(30);
      float old = rebuilt.rebuildRemaining;
      rebuilt.fasterRebuild(10);
      require(rebuilt.rebuildDuration == 50 && rebuilt.rebuildRemaining == old * 50 / 60,
              "Rebuild upgrade resets elapsed cooldown progress");
      std::cout << "PASS actual core absorption, overflow, lethal priority, death and cooldown "
                   "upgrade progress\n";
    }
    {
      Expedition pool(true, true);
      bool offered = false;
      for (int n = 0; n < 3000; ++n) {
        pool.addReward();
        const auto& offer = pool.rewards.back();
        for (int c = 0; c < 3; ++c)
          if (offer.choices[c] == Upgrade::EnergyShield) {
            require(offer.rarities[c] == Rarity::Epic, "Natural shield unlock has wrong rarity");
            offered = true;
          }
        pool.rewards.clear();
      }
      require(offered, "Shield absent from normal rewards");
      auto session = std::make_unique<Session>(true, true);
      auto& s = *session;
      s.mode = Mode::Walking;
      s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = s.frontier.nextHorde =
          s.frontier.mutations.nextMutation = 100000;
      s.debugGrant(Upgrade::EnergyShield, Rarity::Epic);
      tick(s, 11);
      s.expedition.hp = 1000;
      s.expedition.regeneration = 0;
      s.expedition.levels[size_t(Upgrade::Purge)] = 1;
      int boss = s.world.spawn(Titan, 18, 0);
      s.world.foes[boss].speed = 0;
      tick(s, .1f);
      require(s.expedition.hp == 1000 && s.expedition.shield.hp < 1000 && !s.expedition.purgeUsed,
              "Real boss damage bypasses shield or triggers hull emergency purge");
      s.expedition.shield.absorb(100000);
      s.expedition.hp = 700;
      tick(s, .02f);
      require(s.expedition.purgeUsed, "Shield integration breaks hull emergency threshold");
      auto fresh = std::make_unique<Session>(true, true);
      require(!fresh->expedition.shield.purchased, "Shield survives new run");
      std::cout << "PASS natural Epic rewards, actual boss attack, protected purge threshold and "
                   "new-run reset\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
