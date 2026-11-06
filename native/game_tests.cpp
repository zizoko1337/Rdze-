#include "gameplay.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace arsenal;
void require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}
void run(Game& g, float seconds) {
  int ticks = int(std::round(seconds * 60));
  for (int i = 0; i < ticks; ++i)
    g.step(1.f / 60);
}
int stationary(Game& g, float x, float z, float hp = 100000) {
  int id = g.spawn(Crawler, x, z);
  g.foes[id].speed = 0;
  g.foes[id].hp = g.foes[id].maxHp = hp;
  return id;
}
int main() {
  try {
    {
      Game g(false);
      g.autoWaves = false;
      require(g.canBuild(0, 0) != nullptr, "Core footprint accepts construction");
      require(g.canBuild(250, 0) != nullptr, "Power radius ignored");
      require(g.canBuild(std::numeric_limits<float>::quiet_NaN(), 0) != nullptr,
              "Nonfinite build accepted");
      require(g.addTower(Laser, 30, 0), "Valid build rejected");
      require(!g.addTower(Tesla, 34, 0), "Overlapping turret accepted");
      require(g.removeTower(30, 0), "Remove failed");
      require(g.addTower(Tesla, 30, 0), "Rebuild after removal failed");
      std::cout << "PASS construction, collision, removal and rebuild\n";
    }
    for (int w = 0; w < WeaponCount; ++w) {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Weapon(w), 30, 0);
      int id = stationary(g, 55, 0);
      stationary(g, 57, 1);
      run(g, 15);
      require(g.damageDone[w] > 0, "A weapon dealt no damage");
      require(g.fired[w] > 0, "A weapon never fired");
      require(g.foes[id].hp < 100000, "Primary target not hit");
      require(g.shots.size() < 1000 && g.particles.size() <= 18000 && g.beams.size() <= 12000,
              "Unbounded effects");
      std::cout << "PASS weapon " << w + 1 << " damage=" << g.damageDone[w]
                << " shots=" << g.fired[w] << '\n';
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Tesla, 30, 0);
      int a = stationary(g, 50, 0, 1000), b = stationary(g, 35, 20, 1000),
          c = stationary(g, 95, 0, 1000);
      run(g, .2f);
      require(g.foes[a].hp == 855 && g.foes[b].hp == 855,
              "Tesla does not hit every in-range enemy");
      require(g.foes[c].hp == 1000, "Tesla hits outside range");
      run(g, 4);
      require(g.foes[a].hp == 855, "Tesla cooldown ignored");
      run(g, 2);
      require(g.foes[a].hp == 710, "Tesla did not recharge");
      std::cout << "PASS Tesla radius and long cooldown\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Lance, 30, 0);
      stationary(g, 50, 0);
      run(g, 5.9f);
      require(g.damageDone[Lance] == 0, "Lance fired before charging");
      run(g, 2);
      require(g.damageDone[Lance] > 200, "Lance is not continuous");
      run(g, 1.3f);
      double d = g.damageDone[Lance];
      run(g, 1);
      require(g.damageDone[Lance] == d, "Lance ignores cooldown");
      std::cout << "PASS charged continuous beam cycle\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Flame, 30, 0);
      int front = stationary(g, 50, 0), back = stationary(g, 23, 0);
      g.towers[0].target = front;
      g.towers[0].generation = g.foes[front].generation;
      run(g, 1);
      require(g.foes[front].hp < 100000 && g.foes[back].hp == 100000, "Flame cone broken");
      g.towers.clear();
      double d = g.damageDone[Flame];
      run(g, 1);
      require(g.damageDone[Flame] > d, "Burn does not persist");
      std::cout << "PASS flame cone and afterburn\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Venom, 30, 0);
      int id = stationary(g, 50, 0);
      run(g, 2);
      require(g.foes[id].poison > 0 && !g.gas.empty(), "Poison cloud missing");
      g.towers.clear();
      g.shots.clear();
      double d = g.damageDone[Venom];
      run(g, 2);
      require(g.damageDone[Venom] > d, "Poison does not persist");
      run(g, 15);
      require(g.foes[id].poison <= 0 && g.gas.empty(), "Poison never expires");
      std::cout << "PASS poison clouds, damage over time and expiry\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Mortar, 30, 0);
      int a = stationary(g, 65, 0, 1000), b = stationary(g, 68, 0, 1000);
      run(g, 1);
      require(!g.shots.empty() && g.shots[0].pos.y > 25, "Mortar lacks ballistic arc");
      run(g, 2);
      require(g.foes[a].hp < 1000 && g.foes[b].hp < 1000, "Mortar splash missing");
      std::cout << "PASS mortar trajectory and area impact\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.addTower(Laser, 30, 0);
      int locked = stationary(g, 60, 0);
      run(g, .1f);
      int closer = stationary(g, 40, 0);
      run(g, .1f);
      require(g.towers[0].target == locked, "Tower switches from live locked target");
      uint32_t gen = g.foes[locked].generation;
      g.foes[locked].hp = 0;
      run(g, .1f);
      int recycled = stationary(g, 70, 0);
      require(recycled == locked && !g.valid(recycled, gen), "Stale generation accepted");
      require(g.towers[0].target == closer, "Tower did not reacquire after death");
      float t = g.time;
      g.paused = true;
      run(g, 1);
      require(g.time == t, "Paused game advances");
      std::cout << "PASS persistent locks, safe slot reuse and pause\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.advanceWave();
      g.advanceWave();
      g.advanceWave();
      require(g.wave == 4 && g.bossSerial == 1, "Wave progression does not spawn boss");
      int queen = g.boss();
      require(g.foes[queen].type == Queen, "Second boss is not a different species");
      g.foes[queen].ability = 0;
      size_t n = g.active.size();
      run(g, .1f);
      require(g.active.size() >= n + 5, "Queen does not summon brood");
      require(std::string(SPECIES[Titan].texture) != SPECIES[Queen].texture, "Boss art reused");
      std::cout << "PASS progression, distinct bosses and brood ability\n";
    }
    {
      Game g(false);
      g.autoWaves = false;
      g.horde(5000);
      for (int i = 0; i < 90; ++i) {
        float a = i * 2.39996f, r = 25 + std::sqrt(float(i)) * 13;
        g.addTower(Weapon(i % 9), std::sin(a) * r, std::cos(a) * r);
      }
      auto* data = g.foes.data();
      run(g, 10);
      require(data == g.foes.data(), "Enemy pool moved");
      require(g.kills > 0, "Mass combat did not kill anything");
      for (int i : g.active)
        require(std::isfinite(g.foes[i].x) && std::isfinite(g.foes[i].z),
                "Invalid position in mass combat");
      std::cout << "PASS mixed arsenal with 5000 enemies, kills=" << g.kills << "\n";
    }
    std::cout << "ALL ARSENAL TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
