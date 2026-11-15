#pragma once
#include "command_game.hpp"
#include <stdexcept>

namespace command {
struct FactoryVisualTest {
  int stage = 0;
  bool complete = false, pauseVerified = false;
  float frozenTime = 0, frozenClock = 0;
  double stageAt = 0;
  template <class Key, class Launch>
  void step(Session& s, double wall, Key key, Launch launch, std::string& capture, bool& running) {
    if (wall > RoverFactories::READY_AT + 12) {
      running = false;
      return;
    }
    auto& f = s.frontier;
    if (stage == 0 && wall > .3) {
      launch();
      stage = 1;
    } else if (stage == 1 && s.mode == Mode::Walking) {
      for (int id : s.world.active)
        s.world.foes[id].hp = 0;
      s.world.step(1.f / 60);
      s.spawnTimer = f.nextBoss = f.nextElite = f.nextHorde = 100000;
      s.expedition.hp = s.expedition.maxHp = 100000;
      s.expedition.miningMultiplier = 0;
      int enemy = s.world.spawn(Crawler, 0, 120);
      s.world.foes[enemy].hp = s.world.foes[enemy].maxHp = 100000;
      s.world.foes[enemy].speed = 0;
      if (!s.debugGrant(Upgrade::RoverFactory, Rarity::Rare))
        throw std::runtime_error("Factory card was not installed");
      stage = 2;
    } else if (stage == 2 && f.factories.age > RoverFactories::ARRIVAL * .35f) {
      capture = "factories-01-inbound.png";
      stage = 23;
    } else if (stage == 23 && f.factories.age > RoverFactories::ARRIVAL * .72f) {
      capture = "factories-01b-approach.png";
      stage = 3;
    } else if (stage == 3 && f.factories.age > RoverFactories::ARRIVAL + .6f) {
      capture = "factories-02-drop.png";
      stage = 4;
    } else if (stage == 4 && f.factories.age > RoverFactories::ARRIVAL + RoverFactories::DROP + 2) {
      capture = "factories-03-return.png";
      stage = 5;
    } else if (stage == 5 && f.factories.age > RoverFactories::CONNECT_AT + .7f) {
      capture = "factories-04-pipes.png";
      stage = 6;
    } else if (stage == 6 && f.factories.ready() && f.roversMade == 6) {
      for (int i = 0; i < 6; ++i)
        if (f.factories.produced[i] != 1 || f.factories.droneVisible(i))
          throw std::runtime_error("Factory delivery or first six outputs failed");
      capture = "factories-05-ready.png";
      if (!s.debugGrant(Upgrade::RoverBatch, Rarity::Epic) || s.expedition.roverBatch != 10)
        throw std::runtime_error("Epic series must produce ten");
      f.productionClock = 0;
      stage = 7;
    } else if (stage == 7 && f.roversMade == 16 && f.factories.queuedCount() == 0) {
      int pairs = 0, singles = 0;
      for (int n : f.factories.lastBatch) {
        pairs += n == 2;
        singles += n == 1;
      }
      if (pairs != 4 || singles != 2)
        throw std::runtime_error("Ten-rover series not balanced");
      capture = "factories-06-batch10.png";
      key(SDL_SCANCODE_P);
      stage = 8;
    } else if (stage == 8 && s.world.paused) {
      frozenTime = s.world.time;
      frozenClock = f.productionClock;
      stageAt = wall;
      stage = 9;
    } else if (stage == 9 && wall - stageAt > .3) {
      pauseVerified = s.world.time == frozenTime && f.productionClock == frozenClock;
      key(SDL_SCANCODE_P);
      stage = 10;
    } else if (stage == 10 && !s.world.paused) {
      complete = pauseVerified;
      running = false;
    }
  }
};
} // namespace command
