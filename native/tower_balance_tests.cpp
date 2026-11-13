#include "command_game.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}
void tick(Game& g, float seconds) {
  for (int n = 0; n < int(std::round(seconds * 60)); ++n)
    g.step(1.f / 60);
}
int enemy(Game& g, Species kind, float x, float z, bool elite = false) {
  int id = g.spawn(kind, x, z);
  g.foes[id].speed = 0;
  g.foes[id].hp = g.foes[id].maxHp = 100000000;
  g.foes[id].elite = elite;
  g.foes[id].ability = 100000;
  g.grid.rebuild(g.foes, g.active);
  return id;
}
int main() {
  try {
    require(WEAPONS[Laser].range > WEAPONS[Minigun].range &&
                WEAPONS[Laser].damage / WEAPONS[Laser].cooldown <
                    WEAPONS[Minigun].damage / WEAPONS[Minigun].cooldown &&
                Game::turnSpeed(Laser) > Game::turnSpeed(Minigun),
            "Starter roles not distinct");
    for (auto weapon : {Sniper, Lance}) {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->addTower(weapon, 0, 42);
      int normal = enemy(*g, Crawler, 0, 50);
      tick(*g, .2f);
      require(g->towers[0].target == normal, "No normal fallback");
      int elite = enemy(*g, Brute, 0, 70, true);
      tick(*g, .2f);
      require(g->towers[0].target == elite, "Elite does not preempt normal lock");
      int boss = enemy(*g, Titan, 0, 100);
      tick(*g, .2f);
      require(g->towers[0].target == boss, "Boss does not preempt elite lock");
      g->focusTarget = normal;
      g->focusGeneration = g->foes[normal].generation;
      g->focusTime = 1;
      tick(*g, .2f);
      require(g->towers[0].target == normal, "Explicit shared focus lost");
      g->focusTime = 0;
      tick(*g, .2f);
      require(g->towers[0].target == boss, "Priority not restored after focus");
      g->foes[boss].hp = 0;
      tick(*g, .2f);
      require(g->towers[0].target == elite, "Dead boss remains locked");
      g->foes[elite].z = 500;
      tick(*g, .2f);
      require(g->towers[0].target == normal, "Out-of-range elite blocks fallback");
    }
    std::cout << "PASS starter roles, boss/elite/normal priority, preemption, focus and fallback\n";
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->addTower(Lance, 0, 42);
      int first = enemy(*g, Crawler, 0, 70);
      g->foes[first].hp = 1;
      auto& t = g->towers[0];
      t.state = 1;
      t.phase = 3;
      int next = -1;
      for (int frame = 0; frame < 180; ++frame) {
        if (frame == 60)
          next = enemy(*g, Titan, 0, 72);
        if (frame == 90) {
          g->foes[next].z = 500;
          g->grid.rebuild(g->foes, g->active);
        }
        if (frame == 120) {
          g->foes[next].x = 80;
          g->foes[next].z = 42;
          g->grid.rebuild(g->foes, g->active);
        }
        if (frame == 150) {
          g->paused = true;
          float remaining = t.phase;
          tick(*g, 1);
          require(t.phase == remaining, "Pause advances active discharge");
          g->paused = false;
        }
        g->step(1.f / 60);
        require(t.state == (frame < 179 ? 1 : 2) && t.pulse == 1 && !g->beams.empty(),
                "Lance beam interrupted by a kill, empty range, or misalignment");
        if (frame == 0)
          require(g->foes[first].hp <= 0, "First discharge target survives");
        if (frame == 89)
          require(g->foes[next].hp < g->foes[next].maxHp, "Active discharge does not hit next target");
        if (frame >= 120)
          require(t.target == next, "Active lance does not acquire off-axis priority target");
      }
      require(t.state == 2, "Discharge does not end after three seconds");
      tick(*g, .1f);
      require(g->beams.empty(), "Beam continues emitting during cooldown");
      std::cout << "PASS full 3s visible discharge after kill, empty range, retargeting, aiming and pause\n";
    }
    {
      auto lance = std::make_unique<Game>(false), mini = std::make_unique<Game>(false);
      for (auto* g : {lance.get(), mini.get()}) {
        g->autoWaves = false;
        enemy(*g, Titan, 0, 65);
      }
      lance->addTower(Lance, 0, 42);
      mini->addTower(Minigun, 0, 42);
      tick(*lance, 330);
      tick(*mini, 330);
      double ratio = lance->damageDone[Lance] / mini->damageDone[Minigun];
      require(ratio > 2.9 && ratio < 3.1, "Lance full-cycle DPS not three miniguns");
      std::cout << "PASS 330s actual DPS: lance=" << lance->damageDone[Lance] / 330
                << " minigun=" << mini->damageDone[Minigun] / 330 << " ratio=" << ratio << '\n';
      lance = std::make_unique<Game>(false);
      lance->autoWaves = false;
      lance->addTower(Lance, 0, 42);
      enemy(*lance, Titan, 50, 42);
      for (int n = 0; n < 60; ++n) {
        float a = lance->towers[0].angle;
        lance->step(1.f / 60);
        require(std::abs(lance->towers[0].angle - a) <= .2501f / 60,
                "Lance exceeds slow turn limit");
      }
      require(lance->damageDone[Lance] == 0, "Lance skips charge/aim");
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      g->addTower(Tesla, 0, 42);
      int a = enemy(*g, Crawler, 0, 65), b = enemy(*g, Brute, 2, 65, true),
          c = enemy(*g, Titan, -2, 65);
      tick(*g, .2f);
      require(g->foes[a].maxHp - g->foes[a].hp == 160 && g->foes[b].maxHp - g->foes[b].hp == 56 &&
                  g->foes[c].maxHp - g->foes[c].hp == 16,
              "Tesla category damage wrong");
      g->towers.clear();
      int far = enemy(*g, Crawler, 10, 100);
      g->blast({0, 0, 65}, 4, 180, Rocket, .55f);
      require(g->foes[c].maxHp - g->foes[c].hp < 180 && g->foes[far].hp == g->foes[far].maxHp,
              "Rocket boss reduction or small splash missing");
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      int id = enemy(*g, Crawler, 0, 100);
      g->foes[id].hp = g->foes[id].maxHp = 100000;
      g->foes[id].speed = 10;
      g->addTowerPoison(id, 32);
      tick(*g, 1);
      require(std::abs(g->foes[id].z - 93.5f) < .02f, "Tower toxin slow is not 35 percent");
      g->foes[id].speed = 0;
      g->addTowerPoison(id, 64);
      require(g->foes[id].towerPoisonDps == 96, "Tower poison does not stack");
      float hp = g->foes[id].hp;
      tick(*g, 1);
      require(std::abs(hp - g->foes[id].hp - 96) < .6f, "Stacked damage not applied once per tick");
      g->paused = true;
      tick(*g, 8);
      require(g->foes[id].towerPoisonDps == 96, "Pause expires poison");
      g->paused = false;
      tick(*g, 4.6f);
      require(std::abs(g->foes[id].towerPoisonDps - 64) < .001f,
              "Older dose not expired separately");
      tick(*g, 1.1f);
      require(g->foes[id].towerPoisonDps < .001f, "Poison keeps stale damage after expiry");
      g->foes[id].hp = 0;
      tick(*g, .1f);
      int reused = enemy(*g, Crawler, 0, 100);
      require(reused == id && g->foes[reused].towerPoisonDps == 0, "Poison survives slot reuse");
    }
    std::cout << "PASS Tesla crowd specialization, small rocket splash, additive poison, expiry "
                 "and reuse\n";
    {
      auto s = std::make_unique<Session>(true, true);
      s->mode = Mode::Walking;
      s->spawnTimer = s->frontier.nextBoss = s->frontier.nextElite = 100000;
      require(s->expedition.roverBatch == 5 && s->expedition.roverInterval() == 12,
              "Factory base is not five per 12 seconds");
      int a = enemy(s->world, Crawler, 20, 0), b = enemy(s->world, Crawler, 20, 2),
          c = enemy(s->world, Crawler, 20, 5);
      for (int id : {a, b, c})
        s->world.foes[id].hp = s->world.foes[id].maxHp = 10000;
      s->frontier.rovers.push_back({{18, .4f, 0}});
      s->step(1.f / 60);
      require(s->world.foes[a].maxHp - s->world.foes[a].hp == 180 &&
                  s->world.foes[b].hp < s->world.foes[b].maxHp &&
                  s->world.foes[c].hp == s->world.foes[c].maxHp,
              "Rover splash not equivalent to small rocket blast");
      require(s->world.damageDone[Rocket] == 0, "Rover damages pollute rocket turret stats");
    }
    {
      auto g = std::make_unique<Game>(false);
      g->autoWaves = false;
      for (int n = 0; n < 6000; ++n) {
        int id = enemy(*g, Crawler, 100 + n % 100, 100 + n / 100);
        g->addTowerPoison(id, 32);
        g->addTowerPoison(id, 32);
      }
      auto* storage = g->foes.data();
      auto start = std::chrono::steady_clock::now();
      tick(*g, 8);
      double ms =
          std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
              .count() /
          480;
      require(g->foes.data() == storage, "Poison reallocates foe storage");
      for (int id : g->active)
        require(g->foes[id].towerPoisonDps < .001f, "Mass poison failed to expire");
      std::cout << "PASS 6000 poisoned foes: " << ms << " ms/tick, fixed storage\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
