#include "command_game.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool ok, const char* message) {
  if (!ok)
    throw std::runtime_error(message);
}
bool close(float a, float b) {
  return std::abs(a - b) < .01f;
}
void arm(Game& g, EnemyMutationSystem& m) {
  require(m.apply(g, EnemyMutation::NextBossHealth, Rarity::Common) &&
              m.apply(g, EnemyMutation::NextBossSpeed, Rarity::Uncommon),
          "Next-boss charges rejected");
}
int main() {
  try {
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      g.time = 180;
      Frontier f;
      f.nextElite = f.nextHorde = f.mutations.nextMutation = 100000;
      f.mutations.apply(g, EnemyMutation::Health, Rarity::Rare);
      f.mutations.apply(g, EnemyMutation::Speed, Rarity::Rare);
      f.mutations.apply(g, EnemyMutation::BossCompanion, Rarity::Legendary);
      int existing = f.spawnFoe(g, Wyrm, 0, 300);
      float existingHp = g.foes[existing].hp, existingSpeed = g.foes[existing].speed;
      require(!f.mutations.apply(g, EnemyMutation::NextBossHealth, Rarity::Uncommon) &&
                  !f.mutations.apply(g, EnemyMutation::NextBossSpeed, Rarity::Common),
              "Next-boss mutations accept wrong rarities");
      arm(g, f.mutations);
      require(g.foes[existing].hp == existingHp && g.foes[existing].speed == existingSpeed &&
                  !f.mutations.apply(g, EnemyMutation::NextBossHealth, Rarity::Common) &&
                  !f.mutations.apply(g, EnemyMutation::NextBossSpeed, Rarity::Uncommon),
              "Pending charges stack or change a living boss");
      int ordinary = f.spawnFoe(g, Crawler, 300, 0);
      int elite = f.spawnFoe(g, Runner, 300, 0, true);
      require(!g.foes[ordinary].doubleBaseHp && !g.foes[elite].doubleBaseSpeed &&
                  g.enemyGenome.nextBossHealth && g.enemyGenome.nextBossSpeed,
              "Regular enemy or elite consumes a boss charge");
      float timer = 100000;
      f.stepWaves(g, .01f, timer);
      int first = g.active.back();
      require(g.foes[first].type == Titan && g.foes[first].doubleBaseHp &&
                  g.foes[first].doubleBaseSpeed && close(g.foes[first].maxHp, 7000 * 2 * 1.22f) &&
                  close(g.foes[first].speed, Frontier::spawnSpeed(Titan, 180) * 2 * 1.08f) &&
                  !g.enemyGenome.nextBossHealth && !g.enemyGenome.nextBossSpeed,
              "First scheduled boss loses x2 boosts during encounter scaling");
      g.time = 182;
      f.stepWaves(g, 2, timer);
      int second = g.active.back();
      require(g.foes[second].type == Queen && !g.foes[second].doubleBaseHp &&
                  !g.foes[second].doubleBaseSpeed && close(g.foes[second].maxHp, 7500 * 1.22f),
              "One charge affects the whole multi-boss series");
      g.hurt(first, g.foes[first].maxHp * .5f, Venom);
      f.mutations.apply(g, EnemyMutation::Health, Rarity::Common);
      f.mutations.apply(g, EnemyMutation::Speed, Rarity::Common);
      require(close(g.foes[first].maxHp, 7000 * 2 * 1.32f) &&
                  close(g.foes[first].hp / g.foes[first].maxHp, .5f) &&
                  close(g.foes[first].speed, Frontier::spawnSpeed(Titan, 180) * 2 * 1.11f),
              "Later global mutation removes encounter boosts or heals the boss");
      arm(g, f.mutations);
      require(f.mutations.history[f.mutations.history.size() - 2].rarity == Rarity::Common &&
                  f.mutations.history.back().rarity == Rarity::Uncommon,
              "Consumed mutation does not re-enter pool or captain notices have wrong rarity");
      std::cout << "PASS Common x2 base HP / Uncommon x2 speed, exactly one boss in series and "
                   "later genome refresh\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      Frontier f;
      require(f.mutations.apply(g, EnemyMutation::NextBossSpeed, Rarity::Uncommon),
              "Independent speed mutation rejected");
      int wyrm = f.spawnFoe(g, Wyrm, 0, 240);
      require(!g.foes[wyrm].doubleBaseHp && g.foes[wyrm].maxHp == 14000,
              "Speed-only mutation also doubles HP");
      g.step(.1f);
      require(close(240 - std::hypot(g.foes[wyrm].x, g.foes[wyrm].z), WYRM_FORWARD_SPEED * 2 * .1f),
              "Speed mutation changes a stat but not actual boss movement");
      require(f.mutations.apply(g, EnemyMutation::NextBossHealth, Rarity::Common),
              "Independent HP mutation rejected");
      int tunneler = f.spawnFoe(g, Tunneler, 300, 0);
      require(g.foes[tunneler].maxHp == 44000 && !g.foes[tunneler].doubleBaseSpeed &&
                  g.foes[tunneler].speed == TunnelerBody::FORWARD,
              "HP-only mutation also changes speed");
      std::cout << "PASS independent one-shot effects and actual doubled Wyrm movement\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      EnemyMutationSystem m;
      arm(g, m);
      int root = g.spawn(Slime, 300, 0);
      float rootSpeed = g.foes[root].speed;
      require(g.foes[root].maxHp == 16000, "Empowered Slime root lacks doubled HP");
      arm(g, m); // A future boss can be armed while this family is still alive.
      float total = 0;
      int pieces = 0;
      for (;;) {
        int target = -1;
        for (int id : g.active)
          if (g.foes[id].type == Slime && g.foes[id].hp > 0) {
            target = id;
            break;
          }
        if (target < 0)
          break;
        const auto foe = g.foes[target];
        require(foe.doubleBaseHp && foe.doubleBaseSpeed &&
                    close(foe.speed, rootSpeed * std::pow(1.3f, float(foe.slimeStage))),
                "Slime split loses encounter modifiers");
        total += foe.hp;
        ++pieces;
        g.hurt(target, foe.hp, Venom);
      }
      require(total == 48000 && pieces == 7 && g.bossKills == 1 && g.enemyGenome.nextBossHealth &&
                  g.enemyGenome.nextBossSpeed,
              "Slime family multiplies boosts repeatedly or consumes next encounter charges");
      int next = g.spawn(Titan, 0, 300);
      require(g.foes[next].doubleBaseHp && g.foes[next].doubleBaseSpeed &&
                  !g.enemyGenome.nextBossHealth && !g.enemyGenome.nextBossSpeed,
              "Next real boss does not receive reserved charges after Slime split");
      std::cout << "PASS Slime family inherits boosts once, 48000 total HP and future charges "
                   "survive splits\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      EnemyMutationSystem m;
      arm(g, m);
      g.time = 10000;
      g.freeIds.clear();
      require(g.spawn(Titan, 300, 0) < 0 && g.enemyGenome.nextBossHealth &&
                  g.enemyGenome.nextBossSpeed,
              "Charge expires with time or is consumed by a failed spawn");
      g.freeIds.push_back(0);
      int id = g.spawn(Tunneler, 300, 0);
      require(g.foes[id].maxHp == 44000 && g.foes[id].doubleBaseSpeed,
              "Segmented boss misses deferred charge");
      auto fresh = std::make_unique<Game>(false);
      require(!fresh->enemyGenome.nextBossHealth && !fresh->enemyGenome.nextBossSpeed,
              "Next-boss charges survive expedition reset");
      EnemyMutationSystem draws;
      bool hp = false, speed = false;
      fresh->time = 180;
      for (int n = 0; n < 1000 && !(hp && speed); ++n) {
        fresh->enemyGenome = {};
        draws.roll(*fresh);
        const auto& last = draws.history.back();
        hp |= last.kind == EnemyMutation::NextBossHealth && last.rarity == Rarity::Common;
        speed |= last.kind == EnemyMutation::NextBossSpeed && last.rarity == Rarity::Uncommon;
      }
      require(hp && speed, "Natural pool omits one-shot boss mutations");
      std::cout
          << "PASS deferred/failed spawn, segmented boss, new run reset and real rarity rolls\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
