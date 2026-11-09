#include "bestiary.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool value, const char* message) {
  if (!value)
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
void quiet(Session& s) {
  s.start(Gun::Pulse);
  s.startExtraction();
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = s.frontier.nextHorde = 100000;
}
int foe(Game& g, Species type, float x, float z) {
  int id = g.spawn(type, x, z);
  g.foes[id].speed = 0;
  g.foes[id].hp = g.foes[id].maxHp = 1000000;
  return id;
}
int main() {
  try {
    {
      Session s(true);
      quiet(s);
      const auto p = TURRET_SLOTS[0];
      s.world.addTower(Laser, p.x, p.z);
      s.expedition.levels[size_t(Upgrade::Sentry)] = 1;
      int id = foe(s.world, Saboteur, p.x + 3, p.z);
      require(s.world.foes[id].elite, "Saboteur is not intrinsically elite");
      tick(s, 2.9f);
      require(s.world.towers.size() == 1, "Saboteur destroys before three seconds");
      tick(s, .15f);
      require(s.world.towers.empty() && s.expedition.levels[size_t(Upgrade::Sentry)] == 0 &&
                  s.expedition.towerUnlocked[Laser] && s.slotAvailable(0),
              "Destruction does not release occupied hex and inventory count");
      require(s.world.coreDamage == 0, "Saboteur attacks core while working on turret");
      std::cout << "PASS three-second elite sabotage, free hex, known technology and stock\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Laser, 42, 0);
      g.addTower(Minigun, 63, 0);
      int id = foe(g, Saboteur, 45, 0);
      tick(g, 2);
      g.foes[id].stun = .3f;
      tick(g, 1);
      require(g.foes[id].sabotage < 1 && g.towers.size() == 2,
              "Stun fails to interrupt continuous sabotage");
      g.removeTower(42, 0);
      g.foes[id].x = 66;
      tick(g, 2.8f);
      require(g.towers.size() == 1, "Attack progress transfers to another tower ID");
      g.hurt(id, g.foes[id].hp, Laser);
      tick(g, 1);
      require(g.towers.size() == 1, "Dead saboteur completes destruction");
      Game walk(false);
      walk.autoWaves = false;
      walk.addTower(FrostTower, 42, 0);
      int moving = walk.spawn(Saboteur, 90, 0);
      tick(walk, 4);
      require(walk.foes[moving].x < 90 && walk.foes[moving].towerTarget != 0,
              "Saboteur does not approach turret");
      Game core(false);
      core.autoWaves = false;
      core.spawn(Saboteur, 17, 0);
      tick(core, .2f);
      require(core.coreDamage > 0, "Saboteur without turrets has no core fallback");
      std::cout << "PASS stun and death interruption, stable targeting, movement and fallback\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      int id = foe(g, SiegeSpitter, SIEGE_RANGE, 0);
      g.foes[id].ability = 0;
      tick(g, 1);
      require(g.coreDamage == 0 && g.enemyShots.size() == 1,
              "Acid ignores ballistic travel time or deals per-frame melee damage");
      tick(g, 1);
      require(g.coreDamage == 45 && g.enemyShots.empty(), "Ranged projectile damage wrong");
      tick(g, 4.8f);
      require(g.coreDamage == 45, "Siege spitter attacks too quickly");
      tick(g, .3f);
      require(g.coreDamage == 90, "Second slow spit missing");
      Game outside(false);
      outside.autoWaves = false;
      int distant = foe(outside, SiegeSpitter, SIEGE_RANGE + 1, 0);
      outside.foes[distant].ability = 0;
      tick(outside, 8);
      require(outside.coreDamage == 0 && outside.enemyShots.empty(),
              "Spitter shoots outside range");
      std::cout << "PASS second-ring ranged acid, travel, five-second cadence and range limit\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Laser, 42, 0);
      g.addTower(RateRelay, 63, 0);
      g.addTower(FrostTower, 84, 0);
      g.addTower(Laser, 110, 0);
      int brain = foe(g, Brain, 42, 0);
      int target = foe(g, Crawler, 44, 10);
      require(g.towers[0].localRate == 1.2f && g.movementSlowAt(90, 0) == .5f,
              "Support fixture wrong");
      g.foes[brain].ability = 0;
      tick(g, .1f);
      require(g.towers[0].disabledUntil > g.time && g.towers[1].disabledUntil > g.time &&
                  g.towers[2].disabledUntil > g.time && g.towers[3].disabledUntil == 0,
              "Brain pulse does not respect inclusive two-hex radius");
      require(g.towers[0].localRate == 1 && !g.hasFrostSupports && g.supportLinks.empty() &&
                  g.movementSlowAt(90, 0) == 1,
              "Disabled supports continue buffing or freezing");
      require(g.previewWeaponInfo(Laser, 63, 21).cooldown == g.weaponInfo(Laser).cooldown,
              "Preview includes disabled support");
      float hp = g.foes[target].hp;
      tick(g, 3.8f);
      require(g.foes[target].hp == hp && g.towers[0].shots == 0, "Disabled tower still fires");
      tick(g, .25f);
      require(g.towers[0].disabledUntil == 0 && g.towers[0].shots > 0 &&
                  g.towers[0].localRate == 1.2f && g.movementSlowAt(90, 0) == .5f,
              "Tower and support do not recover after four seconds");
      tick(g, 2);
      require(g.towers[0].disabledUntil > g.time, "Brain pulse does not repeat");
      g.foes[brain].stun = 10;
      tick(g, 4.2f);
      require(g.towers[0].disabledUntil == 0, "Stunned brain keeps pulsing");
      std::cout << "PASS brain radius, attack suppression, support shutdown, recovery and stun\n";
    }
    {
      Session s(true);
      quiet(s);
      s.bestiaryOpen = true;
      Bestiary menu;
      float time = s.world.time, uranium = s.expedition.uranium;
      for (int i = 0; i < SpeciesCount; ++i) {
        auto type = Species(i);
        require(menu.summon(s, type) == 1, "Menu cannot summon all species");
        const auto& e = s.world.foes[s.world.active.back()];
        require(e.maxHp == Frontier::spawnHp(type, time) &&
                    e.speed == Frontier::spawnSpeed(type, time) && e.type == type,
                "Bestiary stats disagree with actual summon");
      }
      tick(s, 2);
      require(s.world.time == time && s.expedition.uranium == uranium && s.shots == 0,
              "Bestiary fails to pause simulation");
      menu.count = 100;
      require(menu.summon(s, Crawler) == 100, "Batch spawn fails");
      int brain = -1;
      for (int id : s.world.active)
        if (s.world.foes[id].type == Brain)
          brain = id;
      s.world.hurt(brain, s.world.foes[brain].hp, Laser);
      s.frontier.processDeaths(s.world);
      require(s.frontier.crystals.size() == 1 && s.frontier.crystals.front().boss,
              "Brain is not a crystal-dropping boss");
      s.bestiaryOpen = false;
      tick(s, .1f);
      require(s.world.time > time, "Closing bestiary does not resume time");
      s.world.time = 135;
      s.frontier.eliteSpawned = 1;
      s.frontier.nextElite = 135;
      s.frontier.stepWaves(s.world, .01f, s.spawnTimer);
      require(s.frontier.spawned[Saboteur] == 2, "Natural elite schedule omits saboteur");
      s.world.time = 310;
      bool seen = false;
      for (int i = 0; i < 1000; ++i)
        seen |= s.frontier.waveSpecies(s.world) == SiegeSpitter;
      require(seen, "Natural waves omit siege spitter");
      s.world.bossSerial = 2;
      int boss = s.world.boss();
      require(s.world.foes[boss].type == Brain, "Third boss is not the brain");
      std::cout
          << "PASS bestiary stats, every species, batches, pause, boss loot and natural schedule\n";
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      for (int i = 0; i < 180; ++i) {
        const auto p = TURRET_SLOTS[i];
        g->addTower(Weapon(i % TowerTypeCount), p.x, p.z);
      }
      for (int i = 0; i < 6000; ++i) {
        float a = i * 2.399963f, r = 100 + i % 200;
        g->spawn(i % 100 == 0   ? Saboteur
                 : i % 100 == 1 ? SiegeSpitter
                                : Crawler,
                 std::sin(a) * r, std::cos(a) * r);
      }
      auto start = std::chrono::steady_clock::now();
      tick(*g, 3);
      double ms =
          std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
              .count() /
          180;
      for (int id : g->active)
        require(std::isfinite(g->foes[id].x) && std::isfinite(g->foes[id].z),
                "Invalid horde movement");
      require(g->enemyShots.size() <= 512, "Enemy projectile pool exceeds cap");
      std::cout << "PASS 6000 enemies / 180 towers CPU simulation: " << ms << " ms/tick\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
