#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
void tick(Session& s, float seconds) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60);
}
bool install(Session& s, Upgrade u, Rarity rarity = Rarity::Uncommon) {
  s.expedition.rewards.push_front(
      {99, {u, Upgrade::Hull, Upgrade::Mining}, {rarity, Rarity::Uncommon, Rarity::Uncommon}});
  s.terminalOpen = true;
  return s.chooseUpgrade(0);
}
void quiet(Session& s) {
  s.mode = Mode::Walking;
  s.spawnTimer = 100000;
}
int target(Game& g, float x, float z, Species species = Crawler) {
  int id = g.spawn(species, x, z);
  g.foes[id].speed = 0;
  g.foes[id].hp = g.foes[id].maxHp = 1000000;
  g.grid.rebuild(g.foes, g.active);
  return id;
}
int main() {
  try {
    {
      Expedition e(true, true), lucky(true, true), normal(true, true);
      lucky.luck = 100;
      std::array<int, 5> counts{};
      std::array<bool, size_t(Upgrade::Count)> seen{};
      for (int i = 0; i < 10000; ++i) {
        auto base = normal.rollRarity(), improved = lucky.rollRarity();
        require(improved >= base, "Luck made a draw worse");
        ++counts[int(base)];
        e.addReward();
        const auto& o = e.rewards.back();
        for (int c = 0; c < 3; ++c) {
          auto u = o.choices[c];
          require(minimumRarity(u) <= o.rarities[c], "Card below required rarity");
          require(e.eligible(u), "Ineligible card in pool");
          if (u == Upgrade::Luck || u == Upgrade::FlatDamage)
            require(o.rarities[c] == Rarity::Common, "Minor bonus occupies a rare draw");
          for (int j = c + 1; j < 3; ++j)
            require(u != o.choices[j], "Repeated choice in one offer");
          seen[size_t(u)] = true;
        }
        e.rewards.clear();
      }
      for (int c : counts)
        require(c > 0, "Rarity never rolls");
      for (int i = 0; i < int(Upgrade::Count); ++i)
        if (Upgrade(i) != Upgrade::RoverBatch && Upgrade(i) != Upgrade::RoverRate)
          require(seen[i], "Upgrade unreachable from normal mining rewards");
      e.levels[size_t(Upgrade::RoverFactory)] = 1;
      for (int n = 0; n < 500; ++n) {
        e.addReward();
        for (auto u : e.rewards.back().choices)
          seen[size_t(u)] = true;
        e.rewards.clear();
      }
      require(seen[size_t(Upgrade::RoverBatch)] && seen[size_t(Upgrade::RoverRate)],
              "Production upgrades never appear after factory unlock");
      Expedition first(true, true);
      first.step(50, 0);
      require(std::any_of(first.rewards.front().choices.begin(),
                          first.rewards.front().choices.end(), isTower),
              "First payout does not introduce defence");
      std::cout
          << "PASS five rarities, all 29 cards reachable, unique choices and monotonic luck\n";
    }
    {
      Session s(true, true);
      quiet(s);
      constexpr Upgrade types[] = {Upgrade::Sentry,         Upgrade::MinigunTower,
                                   Upgrade::TeslaTower,     Upgrade::LanceTower,
                                   Upgrade::RateRelayTower, Upgrade::FrostSupportTower};
      for (int i = 0; i < SLOT_COUNT; ++i) {
        auto u = types[i % std::size(types)];
        require(install(s, u, Rarity::Legendary), "Mixed inventory acquisition rejected");
      }
      require(s.expedition.acquiredTowers() == SLOT_COUNT && s.sentryStock == SLOT_COUNT,
              "Mixed tower cap accounting failed");
      for (int i = 0; i < 100; ++i) {
        s.expedition.addReward();
        for (auto u : s.expedition.rewards.back().choices)
          require(!isTower(u), "Full mixed grid offers a tower");
      }
      s.expedition.rewards.front().choices = {Upgrade::TeslaTower, Upgrade::Sentry,
                                              Upgrade::LanceTower};
      s.expedition.validateFront();
      const auto& cards = s.expedition.rewards.front().choices;
      require(!isTower(cards[0]) && !isTower(cards[1]) && !isTower(cards[2]) &&
                  cards[0] != cards[1] && cards[1] != cards[2] && cards[0] != cards[2],
              "Stale mixed tower offer did not repair uniquely");
      for (int i = 0; i < SLOT_COUNT; ++i)
        require(s.orderSentry(i), "Mixed delivery rejected");
      tick(s, 6.1f);
      require(s.world.towers.size() == SLOT_COUNT && s.sentryInventory.empty(),
              "Mixed deliveries not installed");
      for (int i = 0; i < SLOT_COUNT; ++i)
        require(s.world.towers[i].type == towerWeapon(types[i % std::size(types)]),
                "Delivery changed weapon type");
      std::cout
          << "PASS 210 mixed towers, reserved inventory, typed deliveries and full-grid pool\n";
    }
    {
      // Compare actual damage, not just stat fields. Includes projectile impacts and both DOTs.
      for (int w = 0; w < WeaponCount; ++w) {
        Session base(true, true), upgraded(true, true);
        for (Session* s : {&base, &upgraded}) {
          quiet(*s);
          require(s->world.addTower(Weapon(w), 0, 42), "Cannot add damage-test tower");
          target(s->world, 0, 64);
        }
        require(install(upgraded, Upgrade::TowerDamage), "Tower damage rejected");
        tick(base, 16);
        tick(upgraded, 16);
        double a = base.world.damageDone[w], b = upgraded.world.damageDone[w];
        require(a > 0 && std::abs(b / a - 1.2) < .015,
                "Tower damage bonus fails on a weapon, projectile or DOT");
      }
      Session base(true, true), boosted(true, true);
      for (Session* s : {&base, &boosted}) {
        quiet(*s);
        s->world.addTower(Minigun, 0, 42);
        target(s->world, 0, 64);
      }
      install(boosted, Upgrade::TowerRate, Rarity::Epic);
      tick(base, 10);
      tick(boosted, 10);
      require(boosted.world.fired[Minigun] > base.world.fired[Minigun] * 1.25,
              "Tower rate boost swallowed by timestep");
      Session ranged(true, true);
      quiet(ranged);
      install(ranged, Upgrade::TowerRange, Rarity::Epic);
      ranged.world.addTower(Laser, 0, 42); // Future towers must inherit the bonus.
      int id = target(ranged.world, 0, 120);
      tick(ranged, 1);
      require(ranged.world.foes[id].hp < 1000000, "Future tower cannot fire at upgraded range");
      Session lance(true, true);
      quiet(lance);
      lance.world.addTower(Lance, 0, 42);
      target(lance.world, 0, 64);
      install(lance, Upgrade::TowerRate, Rarity::Epic);
      tick(lance, 4.8f);
      require(lance.world.fired[Lance] == 1, "Lance charging ignores fire-rate upgrade");
      std::cout
          << "PASS actual damage on nine weapons, poison, burn, cadence, range and lance charge\n";
    }
    {
      Session s(true, true);
      quiet(s);
      install(s, Upgrade::Damage, Rarity::Legendary);
      install(s, Upgrade::FlatDamage, Rarity::Common);
      install(s, Upgrade::Luck, Rarity::Common);
      require(s.module.damageMultiplier == 1.75f && s.flatDamage == 1 && s.expedition.luck == 1,
              "Rarity-scaled or minor manual bonuses incorrect");
      install(s, Upgrade::Radar);
      install(s, Upgrade::Frost, Rarity::Rare);
      install(s, Upgrade::Focus, Rarity::Rare);
      install(s, Upgrade::Purge, Rarity::Epic);
      for (auto u : {Upgrade::Radar, Upgrade::Frost, Upgrade::Focus, Upgrade::Purge})
        require(!s.expedition.eligible(u), "Unique module can be acquired twice");
      s.expedition.rewards.clear();
      s.expedition.rewards.push_back({1, {Upgrade::Radar, Upgrade::Frost, Upgrade::Purge}});
      s.expedition.validateFront();
      for (auto u : s.expedition.rewards.front().choices)
        require(s.expedition.eligible(u), "Stale unique module still offered");
      require(!install(s, Upgrade::Purge, Rarity::Legendary), "One-time purge module stacks");
      s.terminalOpen = false;
      int crawler = target(s.world, 30, 40), boss = target(s.world, 70, 0, Titan);
      s.expedition.hp = 749;
      tick(s, .1f);
      require(s.expedition.purgeUsed && s.world.foes[crawler].hp <= 0 &&
                  s.world.foes[boss].hp == 500000,
              "Purge does not kill mobs / halve boss max HP at threshold");
      s.expedition.hp = 1400;
      int next = target(s.world, 30, 40);
      tick(s, .1f);
      s.expedition.hp = 749;
      tick(s, .1f);
      require(s.world.foes[next].hp == 1000000 && s.world.foes[boss].hp == 500000,
              "Purge fired a second time");
      std::cout
          << "PASS tier-scaled manual bonuses, luck, unique modules and one-use threshold purge\n";
    }
    {
      Session base(true, true), cold(true, true);
      quiet(base);
      quiet(cold);
      int a = base.world.spawn(Crawler, 0, 40), b = cold.world.spawn(Crawler, 0, 40);
      base.world.foes[a].speed = cold.world.foes[b].speed = 10;
      install(cold, Upgrade::Frost, Rarity::Rare);
      tick(base, 1);
      tick(cold, 1);
      require(cold.world.foes[b].z > base.world.foes[a].z + 1.5f, "Frost does not slow movement");
      Session focus(true, true);
      quiet(focus);
      focus.world.addTower(Laser, 0, 42);
      int near = target(focus.world, 0, 60), far = target(focus.world, 0, 90);
      tick(focus, .1f);
      require(focus.world.towers[0].target == near, "Initial nearest lock incorrect");
      install(focus, Upgrade::Focus, Rarity::Rare);
      // Mark through a real manual laser hit, with the gun looking away from the nearer enemy.
      focus.mode = Mode::Mounted;
      focus.lift = 1;
      focus.world.foes[near].x = 20;
      focus.world.grid.rebuild(focus.world.foes, focus.world.active);
      Vec3 d = normalize(focus.world.targetPoint(far) - focus.eye());
      focus.pitch = focus.gunPitch = std::asin(d.y);
      focus.yaw = focus.gunYaw = std::atan2(d.x, d.z);
      focus.fire();
      tick(focus, .1f);
      require(focus.world.focusTarget == far && focus.world.towers[0].target == far,
              "Actual manual hit does not override nearest target");
      focus.world.foes[far].z = 220;
      tick(focus, .2f);
      require(focus.world.towers[0].target == near, "Focus ignores turret range");
      focus.world.paused = true;
      float timer = focus.world.focusTime;
      tick(focus, 1);
      require(focus.world.focusTime == timer, "Focus timer ignores pause");
      std::cout << "PASS proximity frost, manual-hit focus, range fallback and pause\n";
    }
    {
      for (int tier = 0; tier < 5; ++tier) {
        Session s(true, true);
        quiet(s);
        require(install(s, Upgrade::Focus, Rarity(tier)), "Focus rarity rejected");
        int id = target(s.world, 0, 90), other = target(s.world, 20, 90);
        s.mode = Mode::Mounted;
        s.lift = 1;
        Vec3 d = normalize(s.world.targetPoint(id) - s.eye());
        s.pitch = s.gunPitch = std::asin(d.y);
        s.yaw = s.gunYaw = std::atan2(d.x, d.z);
        s.fire();
        float multiplier = 1 + tier * .1f;
        require(std::abs(s.world.damageDone[Laser] - 150 * multiplier) < .01,
                "Marking shot does not apply rarity bonus");
        double before = s.world.damageDone[Rocket];
        s.world.hurt(id, 100, Rocket);
        require(std::abs(s.world.damageDone[Rocket] - before - 100 * multiplier) < .01,
                "Tower damage ignores marked vulnerability");
        before = s.world.damageDone[Venom];
        s.world.foes[id].poison = 1;
        s.world.foes[id].poisonDps = 22;
        tick(s, .5f);
        require(std::abs(s.world.damageDone[Venom] - before - 11 * multiplier) < .01,
                "Poison ignores marked vulnerability");
        before = s.world.damageDone[Sniper];
        s.world.hurt(other, 100, Sniper);
        require(s.world.damageDone[Sniper] - before == 100, "Bonus leaks to unmarked target");
        s.world.foes[id].poison = 0;
        s.world.paused = true;
        tick(s, 4);
        require(s.world.focusTime > 2, "Pause consumed focus vulnerability");
        s.world.paused = false;
        tick(s, 3.1f);
        before = s.world.damageDone[Rocket];
        s.world.hurt(id, 100, Rocket);
        require(s.world.damageDone[Rocket] - before == 100, "Expired mark still boosts damage");
        s.markTarget(id);
        uint32_t oldGeneration = s.world.foes[id].generation;
        s.world.foes[id].hp = 0;
        tick(s, .05f);
        int replacement = target(s.world, 0, 90);
        require(replacement == id && s.world.foes[id].generation != oldGeneration,
                "Slot reuse fixture failed");
        before = s.world.damageDone[Rocket];
        s.world.hurt(replacement, 100, Rocket);
        require(s.world.damageDone[Rocket] - before == 100,
                "Reused enemy slot inherits old vulnerability");
      }
      std::cout
          << "PASS focus 0/10/20/30/40%, marking hit, towers, DOT, expiry, pause and slot reuse\n";
    }
    std::cout << "ALL PROGRESSION TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
