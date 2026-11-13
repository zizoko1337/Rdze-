#include "command_game.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool value, const char* message) {
  if (!value)
    throw std::runtime_error(message);
}
bool combat(Upgrade u) {
  return isTower(u) && !isSupport(towerWeapon(u));
}
bool hasCombat(const RewardOffer& offer) {
  return std::any_of(offer.choices.begin(), offer.choices.end(), combat);
}
int main() {
  try {
    for (auto u : {Upgrade::TowerDamage, Upgrade::TowerRate, Upgrade::TowerRange}) {
      auto s = std::make_unique<Session>(true, true);
      for (int n = 0; n < 5; ++n)
        require(s->debugGrant(u, Rarity(n)), "Global tier rejected before fifth choice");
      require(!s->debugGrant(u, Rarity::Legendary) && s->expedition.levels[size_t(u)] == 5,
              "Sixth global choice accepted");
      for (auto other : {Upgrade::TowerDamage, Upgrade::TowerRate, Upgrade::TowerRange})
        if (other != u)
          require(s->expedition.eligible(other), "Global caps share a counter");
      s->expedition.rewards.push_back({1, {u, Upgrade::Hull, Upgrade::Mining}});
      s->expedition.validateFront();
      require(s->expedition.rewards.front().choices[0] != u, "Queued capped upgrade survives");
    }
    std::cout << "PASS separate five-choice global caps, mixed rarities and stale offers\n";
    for (int type = 0; type < WeaponCount; ++type) {
      auto s = std::make_unique<Session>(true, true);
      for (int stat = 0; stat < 3; ++stat)
        require(!s->expedition.eligible(typedTowerUpgrade(Weapon(type), stat)),
                "Unknown type upgrade offered");
      s->expedition.towerUnlocked[type] = true;
      for (int stat = 0; stat < 3; ++stat) {
        auto u = typedTowerUpgrade(Weapon(type), stat);
        for (int level = 0; level < 5; ++level) {
          for (int rarity = 0; rarity < 5; ++rarity)
            require(s->expedition.eligible(u, Rarity(rarity)) == (level == rarity),
                    "Wrong typed rarity eligible");
          auto before = s->world.weaponInfo(Weapon(type));
          s->expedition.rewards.push_front(
              {1,
               {u, Upgrade::Hull, Upgrade::Mining},
               {Rarity((level + 1) % 5), Rarity::Common, Rarity::Common}});
          s->terminalOpen = true;
          require(!s->chooseUpgrade(0), "Skipped stage accepted by real terminal");
          s->expedition.rewards.clear();
          s->terminalOpen = false;
          require(s->debugGrant(u, Rarity::Legendary), "Next typed stage rejected");
          require(s->expedition.levels[size_t(u)] == level + 1, "Typed counter skipped a level");
          auto after = s->world.weaponInfo(Weapon(type));
          require(stat == 0   ? after.damage > before.damage
                  : stat == 1 ? after.cooldown < before.cooldown
                              : after.range > before.range,
                  "Typed card has no actual effect");
        }
        require(!s->debugGrant(u, Rarity::Common), "Sixth typed stage accepted");
      }
      Weapon other = Weapon((type + 1) % WeaponCount);
      auto untouched = s->world.weaponInfo(other);
      require(untouched.damage == WEAPONS[other].damage &&
                  untouched.range == WEAPONS[other].range &&
                  untouched.cooldown == WEAPONS[other].cooldown,
              "Typed upgrade leaks to another type");
      float damage = s->world.weaponInfo(Weapon(type)).damage;
      require(s->debugGrant(Upgrade::TowerDamage, Rarity::Common),
              "Global upgrade rejected after specialization");
      require(std::abs(s->world.weaponInfo(Weapon(type)).damage - damage -
                       WEAPONS[type].damage * .1f) < .001f,
              "Global and typed bonuses do not add relative to base");
      auto preview = s->world.previewWeaponInfo(Weapon(type), 21, 0);
      require(preview.range == s->world.weaponInfo(Weapon(type)).range,
              "Delivery preview ignores specialization");
    }
    std::cout << "PASS all 27 independent typed paths, exact C/U/R/E/L order, real bonuses and "
                 "previews\n";
    {
      auto s = std::make_unique<Session>(true, true);
      s->module.kind = Gun::Crank;
      s->expedition.manualKind = Gun::Crank;
      s->expedition.manualLaser = false;
      s->expedition.manualMinigun = true;
      for (int i = 0; i < int(Upgrade::Count); ++i) {
        auto u = Upgrade(i);
        if (requiredManualGun(u) != Gun::Count && requiredManualGun(u) != Gun::Crank)
          require(!s->expedition.eligible(u) && !s->debugGrant(u, Rarity::Legendary),
                  "Foreign manual card accepted");
      }
      require(s->expedition.eligible(Upgrade::Damage) && s->expedition.eligible(Upgrade::FireRate),
              "Universal manual cards filtered out");
      require(s->debugGrant(Upgrade::VenomTower, Rarity::Common) &&
                  s->debugGrant(Upgrade::VenomDamage, Rarity::Legendary),
              "Manual minigun blocks owned toxic tower specialization");
      require(s->expedition.levels[size_t(Upgrade::VenomDamage)] == 1, "Lab bypasses tier order");
    }
    std::cout << "PASS manual weapon filtering and independently owned tower specializations\n";
    for (auto type : {Venom, Flame, Rocket, Mortar, Lance}) {
      auto base = std::make_unique<Session>(true, true);
      auto boosted = std::make_unique<Session>(true, true);
      for (auto* s : {base.get(), boosted.get()}) {
        s->mode = Mode::Walking;
        s->spawnTimer = s->frontier.nextBoss = s->frontier.nextElite = 100000;
        s->expedition.towerUnlocked[type] = true;
        s->world.addTower(type, 0, 42);
        int target = s->world.spawn(Crawler, 0, 64);
        s->world.foes[target].hp = s->world.foes[target].maxHp = 1000000;
        s->world.foes[target].speed = 0;
        s->world.grid.rebuild(s->world.foes, s->world.active);
      }
      require(boosted->debugGrant(typedTowerUpgrade(type, 0), Rarity::Common),
              "Combat setup rejected");
      for (int frame = 0; frame < 16 * 60; ++frame) {
        base->step(1.f / 60);
        boosted->step(1.f / 60);
      }
      double a = base->world.damageDone[type], b = boosted->world.damageDone[type];
      require(a > 0 && b > a * 1.03 && b < a * 1.17,
              "Specialized damage missing from projectile/DOT/beam");
      if (type == Flame)
        require(boosted->world.foes[0].burnDps > base->world.foes[0].burnDps,
                "Afterburn ignores typed damage");
    }
    {
      auto s = std::make_unique<Session>(true, true);
      s->mode = Mode::Walking;
      s->spawnTimer = 100000;
      s->expedition.towerUnlocked[Lance] = true;
      s->world.addTower(Lance, 0, 42);
      int id = s->world.spawn(Crawler, 0, 64);
      s->world.foes[id].hp = 1000000;
      s->world.foes[id].speed = 0;
      require(s->debugGrant(Upgrade::LanceRate, Rarity::Common), "Lance rate setup rejected");
      for (int n = 0; n < 335; ++n)
        s->step(1.f / 60);
      require(s->world.fired[Lance] > 0, "Typed lance rate does not accelerate charging");
    }
    std::cout << "PASS real poison, afterburn, explosions, lance beam and faster charging\n";
    for (auto source :
         {RewardSource::Uranium, RewardSource::EliteCrystal, RewardSource::BossCrystal}) {
      for (bool late : {false, true}) {
        Expedition e(true, true);
        e.activeTime = late ? 1000 : 0;
        e.rewardRandom.state = 2111 + int(source) * 31 + int(late);
        int delivered = 0, dry = 0;
        for (int n = 0; n < 20000; ++n) {
          e.addReward(source);
          const auto& offer = e.rewards.back();
          bool found = hasCombat(offer);
          delivered += found;
          dry = found ? 0 : dry + 1;
          require(dry <= 2, "Three successive offers without combat tower");
          if (n == 0 && source == RewardSource::Uranium)
            require(found, "First payment lacks a combat tower");
          for (int c = 0; c < 3; ++c) {
            auto u = offer.choices[c];
            require(e.eligible(u, offer.rarities[c]), "Natural reward violates rarity/eligibility");
            for (int j = c + 1; j < 3; ++j)
              require(u != offer.choices[j], "Repeated card in offer");
            if (source == RewardSource::BossCrystal && !combat(u))
              require(offer.rarities[c] >= Rarity::Rare, "Boss non-delivery card below Rare");
          }
          e.rewards.clear();
        }
        require(delivered >= 8000, "Combat offer rate below 40 percent");
        if (!late && source == RewardSource::Uranium)
          require(delivered >= 12000, "Early combat offer rate below 60 percent");
      }
    }
    std::cout << "PASS 120000 offers, category floors, first payout, bad-luck protection and exact "
                 "rarities\n";
    {
      Expedition e(true, true);
      for (auto u :
           {Upgrade::Sentry, Upgrade::MinigunTower, Upgrade::VenomTower, Upgrade::FlameTower}) {
        e.levels[size_t(u)] = 1;
        e.towerUnlocked[towerWeapon(u)] = true;
      }
      e.levels[size_t(Upgrade::Sentry)] = SLOT_COUNT - 3;
      e.addReward();
      require(!hasCombat(e.rewards.back()), "Full inventory receives a new turret");
      e.releaseTower(Laser);
      e.offersWithoutCombat = 2;
      e.addReward();
      require(hasCombat(e.rewards.back()), "Existing common types cannot repeat at 4-type limit");
      for (int n = 0; n < 500; ++n) {
        e.addReward();
        for (auto u : e.rewards.back().choices)
          if (combat(u))
            require(fixedRarity(u) == int(Rarity::Common), "Fifth type leaks into capped build");
        e.rewards.clear();
      }
    }
    std::cout << "PASS full board, freed slot, four-type limit and repeat deliveries\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
