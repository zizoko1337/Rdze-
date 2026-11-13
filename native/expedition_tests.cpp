#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
void tick(Session& s, float seconds, Input input = {}) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60, input);
}
void offer(Session& s, Upgrade type) {
  s.expedition.rewards.push_back({1, {type, Upgrade::Hull, Upgrade::Mining}});
  s.expedition.rewards.back().rarities[0] = s.expedition.effectiveRarity(type, Rarity::Uncommon);
  s.terminalOpen = true;
}
int main() {
  try {
    {
      Expedition e(true, true);
      e.step(49, 0);
      require(!e.shipping && e.rewards.empty(), "Premature pickup");
      e.step(1, 0);
      require(e.shipping && e.uranium == 0 && e.earned == 1 && e.shipments == 0 &&
                  e.rewards.size() == 1,
              "Full bar does not grant an immediate reward");
      e.step(9, 0);
      require(e.rewards.size() == 1 && e.shipping && e.shipments == 0,
              "Early payout or transport accounting wrong");
      e.step(1, 0);
      require(!e.shipping && e.shipments == 1 && e.rewards.size() == 1 && e.uranium == 20,
              "Shipment/reward accounting wrong");
      auto choices = e.rewards.front().choices;
      require(choices[0] != choices[1] && choices[0] != choices[2] && choices[1] != choices[2],
              "Duplicate cards");
      e.step(1, 0);
      require(e.rewards.size() == 1, "Duplicate payout");
      e.step(39, 0);
      require(e.shipping, "Mining does not continue during shipping");
      e.step(10, 0);
      require(e.shipments == 2 && e.rewards.size() == 2, "Pending rewards not queued");
      require(e.rewards.front().id == 1 && e.rewards.back().id == 2,
              "Reward IDs depend on departing drones");
      Expedition fast(true, true);
      fast.miningMultiplier = 100;
      fast.step(1, 0);
      fast.step(1, 0);
      require(fast.earned == 4 && fast.rewards.size() == 4 && fast.shipments == 0 &&
                  fast.awaitingPickup == 3 && fast.uranium == 0,
              "Transport throttles full-bar rewards");
      std::cout << "PASS uranium threshold, transport completion, background extraction, unique "
                   "queued cards\n";
    }
    {
      Session s(true, true);
      s.mode = Mode::Walking;
      s.spawnTimer = 100000;
      int id = s.world.spawn(Crawler, 0, 15.9f);
      s.world.foes[id].speed = 0;
      tick(s, 1);
      require(std::abs(s.expedition.hp - 1499) < .1f,
              "Enemy damage / base regeneration not applied to core");
      s.world.paused = true;
      float hp = s.expedition.hp, uranium = s.expedition.uranium;
      tick(s, 1);
      require(s.expedition.hp == hp && s.expedition.uranium == uranium,
              "Pause does not freeze extraction/damage");
      s.world.paused = false;
      s.terminalOpen = true;
      tick(s, 1);
      require(s.expedition.hp == hp && s.expedition.uranium == uranium,
              "Terminal does not freeze combat/extraction");
      s.terminalOpen = false;
      s.expedition.hp = .01f;
      tick(s, .1f);
      require(s.expedition.dead() && s.expedition.hp == 0, "No defeat at zero HP");
      float time = s.world.time;
      uranium = s.expedition.uranium;
      tick(s, 5, {0, 1, false, true});
      require(s.world.time == time && s.expedition.uranium == uranium && s.shots == 0,
              "Dead run continues");
      std::cout << "PASS real HP, pause, terminal freeze, defeat and stopped simulation\n";
    }
    {
      Session s(true, true);
      s.mode = Mode::Walking;
      s.player = {-6.8f, CORE_HEIGHT + 1.72f, -6.2f};
      s.expedition.shipments = 1;
      s.expedition.addReward();
      auto before = s.expedition.rewards.front().choices;
      require(s.nearTerminal() && s.interact() && s.terminalOpen, "Terminal inaccessible");
      s.interact();
      require(!s.terminalOpen, "Cannot leave terminal");
      s.interact();
      require(s.expedition.rewards.front().choices == before, "Reopening rerolls cards");
      require(!s.chooseUpgrade(-1) && !s.chooseUpgrade(3), "Invalid choice accepted");
      require(s.chooseUpgrade(0) && s.expedition.selected == 1 && s.expedition.rewards.empty(),
              "Choice not consumed");
      require(!s.chooseUpgrade(0), "Same payout consumed twice");
      s.player = {0, CORE_HEIGHT + 1.72f, -9};
      require(!s.nearTerminal(), "Terminal accessible across deck");
      std::cout << "PASS proximity, immutable offers, valid choice and one reward consumption\n";
    }
    {
      Session s(true, true);
      s.mode = Mode::Mounted;
      s.lift = s.hatch = 1;
      s.spawnTimer = 100000;
      offer(s, Upgrade::Damage);
      require(s.chooseUpgrade(0), "Damage card rejected");
      int id = s.world.spawn(Crawler, 0, 95);
      s.world.foes[id].hp = 10000;
      s.world.foes[id].speed = 0;
      s.world.grid.rebuild(s.world.foes, s.world.active);
      Vec3 d = normalize(s.world.targetPoint(id) - s.eye());
      s.pitch = s.gunPitch = std::asin(d.y);
      s.fire();
      require(std::abs(s.world.damageDone[Laser] - 187.5) < .01,
              "Damage card does not affect shots");
      offer(s, Upgrade::Mining);
      s.chooseUpgrade(0);
      require(s.expedition.rate() == 2.5f, "Mining card has no effect");
      s.expedition.hp = 500;
      offer(s, Upgrade::Hull);
      s.chooseUpgrade(0);
      require(s.expedition.maxHp == 1750 && s.expedition.hp == 750, "Hull upgrade/heal incorrect");
      s.world.foes[id].hp = 0;
      offer(s, Upgrade::Sentry);
      s.chooseUpgrade(0);
      require(s.world.towers.empty() && s.sentryStock == 1 && s.tactical,
              "Sentry card bypasses manual placement");
      int slot = nearestTurretSlot(0, 31);
      require(s.orderSentry(slot), "Cannot order acquired sentry");
      tick(s, 6.1f);
      require(s.world.towers.size() == 1 && s.world.towers[0].type == Laser,
              "Sentry not installed after landing");
      int victim = s.world.spawn(Crawler, TURRET_SLOTS[slot].x, TURRET_SLOTS[slot].z + 18);
      s.world.foes[victim].speed = 0;
      tick(s, 3);
      require(s.world.kills > 0, "Installed sentry does not fire");
      std::cout << "PASS actual damage, mining, repair and working automatic sentry\n";
    }
    {
      Session base, upgraded;
      for (Session* s : {&base, &upgraded}) {
        s->mode = Mode::Mounted;
        s->module.kind = Gun::Crank;
        s->spawnTimer = 100000;
      }
      offer(upgraded, Upgrade::FireRate);
      upgraded.chooseUpgrade(0);
      tick(base, 10, {0, 0, false, true});
      tick(upgraded, 10, {0, 0, false, true});
      require(upgraded.shots > base.shots * 1.15f, "Fixed timestep swallows fire-rate upgrade");
      Expedition e(true, true);
      e.levels[size_t(Upgrade::Sentry)] = SLOT_COUNT;
      for (int i = 0; i < 50; ++i) {
        e.addReward();
        for (auto c : e.rewards.back().choices)
          require(c != Upgrade::Sentry, "Capped sentry still offered");
      }
      e.rewards.front().choices = {Upgrade::Sentry, Upgrade::Mining, Upgrade::Hull};
      e.validateFront();
      require(e.rewards.front().choices[0] != Upgrade::Sentry, "Old pending card becomes unusable");
      std::cout << "PASS visible fire-rate improvement, sentry cap and stale offer validation\n";
    }
    {
      require(SLOT_COUNT == 210 && nearestTurretSlot(0, 0) == -1,
              "Grid size or central exclusion wrong");
      for (int i = 0; i < SLOT_COUNT; ++i) {
        const auto& p = TURRET_SLOTS[i];
        require(std::abs(p.x - Expedition::DOCK.x) > 6.8f ||
                    std::abs(p.z - Expedition::DOCK.z) > 6.8f,
                "Uranium dock overlaps a turret installation");
        require(nearestTurretSlot(p.x, p.z) == i && std::hypot(p.x, p.z) > CORE_RADIUS + 7,
                "Slot outside usable ring");
        for (int j = i + 1; j < SLOT_COUNT; ++j)
          require(std::hypot(p.x - TURRET_SLOTS[j].x, p.z - TURRET_SLOTS[j].z) > 17.9f,
                  "Overlapping hex slots");
      }
      Session s(true, true);
      s.mode = Mode::Walking;
      s.spawnTimer = 100000;
      s.sentryStock = 2;
      require(!s.orderSentry(-1) && !s.orderSentry(SLOT_COUNT) && s.sentryStock == 2,
              "Invalid slot consumes inventory");
      require(s.orderSentry(0) && !s.orderSentry(0) && s.sentryStock == 1 &&
                  s.committedSlots() == 1,
              "Drop does not reserve slot");
      tick(s, 2);
      float h = s.sentryDrops.front().cargoHeight();
      require(s.world.towers.empty() && h > 0 && h < 100, "Floating sentry fires before landing");
      s.world.paused = true;
      tick(s, 2);
      require(s.sentryDrops.front().cargoHeight() == h, "Drop ignores pause");
      s.world.paused = false;
      tick(s, 4.2f);
      require(s.world.towers.size() == 1 && s.sentryDrops.front().installed && !s.slotAvailable(0),
              "Drop failed to install/reserve");
      float low = s.sentryDrops.front().balloonHeight();
      tick(s, 1);
      require(s.sentryDrops.front().balloonHeight() > low + 20,
              "Detached balloon does not accelerate upward");
      tick(s, 4);
      require(s.sentryDrops.empty() && s.world.towers.size() == 1,
              "Delivery duplicated tower or retained balloon");
      require(s.orderSentry(1), "Second free slot unavailable");
      require(Expedition::DOCK.y + 3.5f < CORE_HEIGHT, "Docked drone obstructs deck horizon");
      require(std::hypot(std::max(0.f, std::abs(Expedition::DOCK.x) - 13.6f),
                         std::max(0.f, std::abs(Expedition::DOCK.z) - 10.6f)) > CORE_RADIUS,
              "Docked drone intersects the core");
      Session full(true, true);
      full.mode = Mode::Walking;
      for (int i = 0; i < SLOT_COUNT; ++i) {
        offer(full, Upgrade::Sentry);
        require(full.chooseUpgrade(0), "Grid capacity card refused");
      }
      require(full.sentryStock == SLOT_COUNT && full.world.towers.empty(),
              "Unplaced cards not reserved globally");
      full.expedition.addReward();
      for (auto c : full.expedition.rewards.front().choices)
        require(c != Upgrade::Sentry, "Full grid offers more sentries");
      for (int i = 0; i < SLOT_COUNT; ++i)
        require(full.orderSentry(i), "Cannot reserve all hexes");
      tick(full, 10.2f);
      require(full.world.towers.size() == SLOT_COUNT && full.sentryDrops.empty() &&
                  full.sentryStock == 0,
              "Full hex grid delivery accounting failed");
      std::cout << "PASS 210 hexes, manual stock, reservations, descent, pause, powered return and "
                   "full-grid loot cap\n";
    }
    {
      Session s(true, true);
      s.mode = Mode::Walking;
      s.spawnTimer = 100000;
      s.sentryStock = 2;
      s.tactical = true;
      float time = s.world.time, hp = s.expedition.hp, uranium = s.expedition.uranium;
      tick(s, 2);
      require(s.world.time == time && s.expedition.hp == hp && s.expedition.uranium == uranium,
              "Choosing a placement does not freeze the world");
      require(s.orderSentry(0), "Cannot place while selecting");
      tick(s, 1);
      require(s.sentryDrops.front().age == 0 && s.world.time == time,
              "Time resumes with more placements pending");
      s.tactical = false;
      tick(s, .5f);
      require(s.world.time > time && s.sentryDrops.front().age > 0,
              "Leaving placement does not resume simulation");
      s.tactical = true;
      float age = s.sentryDrops.front().age;
      tick(s, .5f);
      require(s.sentryDrops.front().age == age, "In-flight drops move during placement selection");
      require(s.orderSentry(1) && s.sentryStock == 0 && !s.placingSentry(),
              "Last placement does not clear automatic pause");
      tick(s, 1);
      require(s.sentryDrops.front().age > age, "Delivery does not resume after choosing last slot");
      std::cout << "PASS automatic placement pause, camera-mode exit and resume after last order\n";
    }
    {
      Session s(true, true);
      s.start(Gun::Pulse);
      require(s.world.active.empty(), "Expedition starts with an immediate nearby pack");
      std::array<bool, MAX_FOES> seen{};
      for (int frame = 0; frame < 60 * 60; ++frame) {
        s.step(1.f / 60);
        for (int id : s.world.active) {
          if (seen[id]) continue;
          seen[id] = true;
          const auto& foe = s.world.foes[id];
          float radius = std::hypot(foe.x, foe.z);
          require(radius >= Frontier::SPAWN_MIN - .001f && radius <= Frontier::SPAWN_MAX + .001f,
                  "Natural opening enemy spawned inside the frontier ring");
        }
      }
      require(s.world.active.size() > 1 && s.expedition.hp == s.expedition.maxHp,
              "Opening does not allow a gradual approach before reaching the core");
      tick(s, .5f);
      require(s.expedition.shipments == 1 && s.expedition.rewards.size() == 1 &&
                  s.expedition.hp > s.expedition.maxHp * .5f,
              "First natural payout is unreachable");
      std::cout << "PASS natural first minute, HP=" << s.expedition.hp
                << " ships=" << s.expedition.shipments << '\n';
      tick(s, 240);
      require(s.expedition.dead(), "Unprotected core never falls");
    }
    std::cout << "ALL EXPEDITION TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
