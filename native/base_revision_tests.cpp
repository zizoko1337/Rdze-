#include "command_game.hpp"
#include "game_audio.hpp"
#include "radar_scene.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool b, const char* why) {
  if (!b)
    throw std::runtime_error(why);
}
int main() {
  try {
    Session s(true, true);
    s.start(Gun::Toxin);
    s.mode = Mode::Mounted;
    s.lift = s.hatch = 1;
    for (int id : s.world.active)
      s.world.foes[id].hp = 0;
    s.world.step(.01f);
    s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
    require(s.toxinCycle().lift == 1, "Initial loaded catapult not clear for firing");
    s.shots = 1;
    float period = s.module.interval();
    for (float p : {.05f, .15f, .5f, .7f, .84f, .94f, 1.f}) {
      s.cooldown = period * (1 - p);
      auto cycle = s.toxinCycle();
      if (p < .23f)
        require(cycle.lift == 1 && !cycle.pouring, "Nozzle overlaps moving spoon");
      if (p > .4f && p < .78f)
        require(cycle.lift < .001f && cycle.pouring, "Nozzle fails to lower and fill");
      if (p > .92f)
        require(cycle.lift > .999f && !cycle.pouring, "Nozzle not raised before firing");
    }
    s.cooldown = period * .1f;
    auto shots = s.shots;
    s.fire();
    require(s.shots == shots, "Catapult fires before nozzle clears");
    s.cooldown = 0;
    s.fire();
    require(s.shots == shots + 1, "Ready catapult cannot fire");
    require(std::abs(RADAR_TV_POSITION.x - 6.49519f) < .001f &&
                std::abs(RADAR_TV_POSITION.z - 3.75f) < .001f,
            "Radar not at two o'clock");
    std::cout << "PASS raised/lowered nozzle, filling interval, firing gate and radar placement\n";
    {
      Game g(false);
      g.autoWaves = false;
      for (auto type : {Crawler, Titan, Queen, Brain}) {
        int id = g.spawn(type, 400, 400);
        auto& e = g.foes[id];
        e.hp = e.maxHp = 10000;
        e.speed = 0;
        e.poison = 1;
        e.poisonDps = 0;
      }
      for (int n = 0; n < 60; ++n)
        g.step(1.f / 60);
      for (int id : g.active) {
        float expected = g.foes[id].type >= Titan ? 9900 : 9980;
        require(std::abs(g.foes[id].hp - expected) < .1f,
                "Additional poison is not 20/100 HP per second");
      }
      std::cout << "PASS one poison-status bonus, 20 DPS regular / 100 DPS all boss species\n";
    }
    s.expedition.hp = 751;
    require(s.coreWarningLevel() == 0, "Green threshold wrong");
    s.expedition.hp = 750;
    require(s.coreWarningLevel() == 1, "Yellow threshold wrong");
    s.expedition.hp = 301;
    require(s.coreWarningLevel() == 1, "Yellow low boundary wrong");
    s.expedition.hp = 300;
    require(s.coreWarningLevel() == 2 && length(s.criticalShake()) == 0,
            "20 percent boundary wrong");
    s.expedition.hp = 290;
    s.world.time = .33f;
    require(length(s.criticalShake()) > 0 && length(s.criticalShake()) < .08f,
            "Critical shake not light");
    s.world.paused = true;
    require(length(s.criticalShake()) == 0, "Shake continues through pause");
    float energy = 0;
    for (int n = 0; n < 1000; ++n) {
      float v = GameAudio::heavyWave(n % 2 ? 100 : 101, n / 37.f, n / 1000.f, .4f);
      require(std::isfinite(v) && std::abs(v) <= 1.1f, "Service sound has invalid samples");
      energy += v * v;
    }
    require(energy > 20, "Service sounds are silent");
    std::cout << "PASS health lamps, critical-only shake, pause and bounded bass sound synthesis\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
