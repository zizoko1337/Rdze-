#pragma once
#include "command_game.hpp"
#include <SDL3/SDL.h>
#include <stdexcept>

namespace command {
// Exercises the application's real keyboard handling; fixtures only shorten the five-minute wait.
struct CoffeeVisualTest {
  int stage = 0, cycle = 0;
  bool began = false, poured = false, drank = false, paused = false, complete = false;
  bool lockChecked = false;
  double stageAt = 0, frozenAge = 0;
  float startedAt = 0, frozenTime = 0;
  std::array<bool, 5> cycles{};
  static void check(bool value, const char* message) {
    if (!value)
      throw std::runtime_error(message);
  }
  template <class Key, class Launch>
  void step(Session& s, double wall, Key key, Launch launch, SDL_Window* window,
            std::string& capture, bool& running) {
    if (wall > 40) {
      running = false;
      return;
    }
    std::string prefix = "coffee-" + std::to_string(cycle + 1);
    if (stage == 0 && wall > .3) {
      launch();
      stage = 1;
    } else if (stage == 1 && s.mode == Mode::Walking) {
      s.expedition.hp = s.expedition.maxHp = 100000;
      s.player = Session::coffeePoint({0, 1.72f, -4.2f});
      s.yaw = Session::coffeeYaw();
      s.pitch = -.13f;
      s.coffee.awakeRemaining = 0;
      s.coffee.sleepyTime = 16;
      key(SDL_SCANCODE_W, true);
      stage = 2;
    } else if (stage == 2 && s.nearCoffee()) {
      key(SDL_SCANCODE_W, false);
      capture = "coffee-00-sleepy.png";
      stageAt = wall;
      stage = 3;
    } else if (stage == 3 && wall - stageAt > .25) {
      s.coffee.sleepyTime = 21.8;
      capture = "coffee-00-blink.png";
      stageAt = wall;
      stage = 4;
    } else if (stage == 4 && wall - stageAt > .25) {
      key(SDL_SCANCODE_E, true);
      stage = 5;
    } else if (stage == 5) {
      if (s.coffee.active) {
        if (!began) {
          began = true;
          startedAt = s.world.time - float(s.coffee.age);
          key(SDL_SCANCODE_W, true);
          key(SDL_SCANCODE_TAB, true);
          key(SDL_SCANCODE_E, true);
          key(SDL_SCANCODE_APOSTROPHE, true);
          key(SDL_SCANCODE_SEMICOLON, true);
        }
        if (s.coffee.age > .2) {
          check(!s.tactical && !s.upgradeLabOpen && !s.bestiaryOpen &&
                    length(s.player - Session::coffeeStand()) < .001f,
                "Coffee interaction lock failed in application");
          lockChecked = true;
        }
        if (!poured && s.coffee.pouring() && s.coffee.cupFill() > .40f) {
          poured = true;
          capture = prefix + "-pour.png";
        }
        if (!drank && s.coffee.drinking() && s.coffee.cupFill() < .65f) {
          drank = true;
          capture = prefix + "-drink.png";
        }
        if (cycle == 0 && !paused && s.coffee.age > .6) {
          key(SDL_SCANCODE_P, true);
          stage = 6;
        }
      } else if (began) {
        key(SDL_SCANCODE_W, false);
        check(poured && drank && s.coffee.cups == cycle + 1 && s.coffee.fatigue() == 0,
              "Coffee cycle did not complete its visual phases/effect");
        check(s.world.time - startedAt >= s.coffee.duration - .03f,
              "Coffee incorrectly stopped world simulation");
        cycles[cycle] = true;
        capture = prefix + "-restored.png";
        stageAt = wall;
        stage = 8;
      }
    } else if (stage == 6 && s.world.paused) {
      frozenAge = s.coffee.age;
      frozenTime = s.world.time;
      stageAt = wall;
      capture = "coffee-01-paused.png";
      stage = 7;
    } else if (stage == 7 && wall - stageAt > .25) {
      check(s.coffee.age == frozenAge && s.world.time == frozenTime, "Coffee pause failed");
      paused = true;
      key(SDL_SCANCODE_P, true);
      stage = 5;
    } else if (stage == 8 && wall - stageAt > .3) {
      ++cycle;
      if (cycle < 5) {
        check(s.debugGrant(Upgrade(int(Upgrade::CoffeeII) + cycle - 1), Rarity::Common),
              "Coffee visual upgrade rejected");
        if (cycle == 2)
          check(s.debugGrant(Upgrade::CoffeeMachineII, Rarity::Rare), "Rare machine rejected");
        if (cycle == 4)
          check(s.debugGrant(Upgrade::CoffeeMachineIII, Rarity::Legendary),
                "Legendary machine rejected");
        s.player = Session::coffeeStand();
        s.yaw = Session::coffeeYaw();
        s.pitch = -.13f;
        began = poured = drank = false;
        stageAt = wall;
        stage = 9;
      } else {
        s.coffee.awakeRemaining = 0;
        s.coffee.sleepyTime = 16;
        SDL_SetWindowSize(window, 1280, 720);
        stageAt = wall;
        stage = 10;
      }
    } else if (stage == 9 && wall - stageAt > .3) {
      capture = prefix + "-machine.png";
      key(SDL_SCANCODE_E, true);
      stage = 5;
    } else if (stage == 10 && wall - stageAt > .4) {
      capture = "coffee-06-resized-blur.png";
      stageAt = wall;
      stage = 11;
    } else if (stage == 11 && wall - stageAt > .3) {
      key(SDL_SCANCODE_R, true);
      stage = 12;
    } else if (stage == 12 && s.mode == Mode::Selection) {
      check(s.coffee.awakeRemaining == 300 && s.expedition.coffeeTier == 1 &&
                s.expedition.coffeeMachineTier == 1 && !s.coffee.active,
            "Coffee state survives application restart");
      complete = paused && lockChecked &&
                 std::all_of(cycles.begin(), cycles.end(), [](bool x) { return x; });
      running = false;
    }
  }
};
} // namespace command
