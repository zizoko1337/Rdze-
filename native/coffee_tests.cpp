#include "command_game.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}
void tick(Session& s, float seconds, Input input = {}) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60, input);
}
void stand(Session& s) {
  s.mode = Mode::Walking;
  s.player = Session::coffeeStand();
  s.yaw = Session::coffeeYaw();
  s.pitch = -.13f;
  s.spawnTimer = 100000;
}
int main() {
  try {
    {
      CoffeeState c;
      c.tick(1000, false);
      require(c.awakeRemaining == 300 && c.fatigue() == 0, "Pre-extraction clock advances");
      c.tick(299.5f, true);
      require(c.fatigue() == 0, "Sleepiness before five minutes");
      c.tick(.5f, true);
      require(c.fatigue() > 0, "Sleepiness missing at five minutes");
      c.tick(15, true);
      require(c.fatigue() == 1 && std::abs(c.walkingMultiplier() - .45f) < .001f,
              "Full fatigue movement penalty");
      c.sleepyTime = 3.8;
      require(c.blink() == 1, "Blink never fully covers screen");
      c.sleepyTime = 4.49;
      require(c.blink() < .01f, "Eyelids do not reopen");
      c.tick(-1, true);
      require(c.sleepyTime == 4.49, "Invalid dt changes timer");
      std::cout << "PASS five-minute threshold, progressive fatigue and complete blinking\n";
    }
    {
      auto holder = std::make_unique<Session>(true);
      auto& s = *holder;
      stand(s);
      tick(s, 2);
      require(s.coffee.awakeRemaining == 300, "Deck exploration consumes caffeine");
      s.expedition.miningStarted = true;
      tick(s, 1);
      double remaining = s.coffee.awakeRemaining;
      for (bool* flag : {&s.world.paused, &s.terminalOpen, &s.servicesOpen, &s.radioOpen,
                         &s.upgradeLabOpen, &s.bestiaryOpen}) {
        *flag = true;
        tick(s, 1);
        require(s.coffee.awakeRemaining == remaining, "Paused activity consumes caffeine");
        *flag = false;
      }
      s.tactical = true;
      s.sentryStock = 1;
      tick(s, 1);
      require(s.coffee.awakeRemaining == remaining, "Placement consumes caffeine");
      s.sentryStock = 0;
      tick(s, 1);
      require(s.coffee.awakeRemaining < remaining - .99, "Tactical observation prevents fatigue");
      s.expedition.hp = 0;
      remaining = s.coffee.awakeRemaining;
      tick(s, 1);
      require(s.coffee.awakeRemaining == remaining, "Death consumes caffeine");
      std::cout << "PASS active simulation clock, all pause sources, tactical view and death\n";
    }
    {
      auto holder = std::make_unique<Session>(true, true);
      auto& s = *holder;
      s.mode = Mode::Walking;
      s.player = {0, CORE_HEIGHT + 1.72f, -7};
      s.yaw = 0;
      Vec3 before = s.player;
      s.move({1, 0}, .1f);
      float fresh = length(s.player - before);
      s.player = before;
      s.coffee.awakeRemaining = 0;
      s.coffee.sleepyTime = 15;
      s.move({1, 0}, .1f);
      require(std::abs(length(s.player - before) / fresh - .45f) < .001f,
              "Actual movement not slowed");
      s.mode = Mode::Mounted;
      s.yaw = s.gunYaw = s.pitch = s.gunPitch = 0;
      for (int fps : {30, 60, 144, 240}) {
        s.cancelAim();
        s.yaw = s.pitch = 0;
        for (int i = 0; i < fps; ++i) {
          float yaw = s.yaw, pitch = s.pitch;
          s.look(10, 10);
          s.updateAim(1.f / fps);
          require(std::hypot(s.yaw - yaw, s.pitch - pitch) <= .6501f / fps,
                  "Sleepy turret exceeds angular speed limit");
          require(s.gunYaw == s.yaw && s.gunPitch == s.pitch, "Actual gun and view diverge");
        }
      }
      s.coffee = CoffeeState{};
      s.cancelAim();
      s.look(1, 0);
      float yaw = s.yaw;
      s.updateAim(.1f);
      require(s.yaw - yaw > .5f, "Fresh turret retains sleepy speed cap");
      std::cout << "PASS real walk speed and shared turret/camera turn cap at 30-240 FPS\n";
    }
    for (int machine = 1; machine <= 3; ++machine) {
      for (int tier = 1; tier <= 5; ++tier) {
        auto holder = std::make_unique<Session>(true, true);
        auto& s = *holder;
        stand(s);
        s.expedition.coffeeTier = tier;
        s.expedition.coffeeMachineTier = machine;
        s.coffee.awakeRemaining = 0;
        s.coffee.sleepyTime = 20;
        require(s.nearCoffee() && s.interact() && s.coffee.active, "Cannot use nearby machine");
        require(!s.interact(), "E restarts active coffee animation");
        float duration = s.coffee.duration;
        int drinkFrames = 0, pourFrames = 0, elapsed = 0;
        bool raised = false, filling = false, emptying = false;
        float previousFill = 0;
        while (s.coffee.active && elapsed < 200) {
          bool drinking = s.coffee.drinking();
          drinkFrames += drinking;
          pourFrames += s.coffee.pouring();
          raised |= s.coffee.cupLift() > .99f;
          float fill = s.coffee.cupFill();
          filling |= fill > previousFill;
          emptying |= drinking && fill < previousFill;
          previousFill = fill;
          if (elapsed == 10) {
            s.world.paused = true;
            double age = s.coffee.age;
            float time = s.world.time;
            tick(s, 1);
            require(s.coffee.age == age && s.world.time == time, "Pause advances coffee or combat");
            s.world.paused = false;
          }
          s.look(4, 1);
          s.move({1, 1, true, true}, .1f);
          s.fire();
          s.step(1.f / 60, {1, 1, true, true});
          if (s.coffee.age >= s.coffee.alignEnd())
            require(length(s.player - Session::coffeeStand()) < .001f &&
                        std::abs(s.yaw - Session::coffeeYaw()) < .001f,
                    "Input escapes coffee lock");
          ++elapsed;
        }
        require(elapsed == int(duration * 60) && std::abs(drinkFrames - 30) <= 1,
                "Incorrect 3/2/1 second cycle or half-second sip");
        require(pourFrames > 0 && raised && filling && emptying, "Missing visible coffee phases");
        require(s.coffee.cups == 1 && s.coffee.fatigue() == 0 &&
                    s.coffee.awakeRemaining > COFFEE_MINUTES[tier - 1] * 60 - .3,
                "Wrong serving effect or duplicate cup application");
        require(s.world.time > duration - .02f && s.expedition.uranium > duration * 1.99f &&
                    s.shots == 0 && s.mode == Mode::Walking,
                "World paused or gun fired while drinking");
        s.look(.2f, 0);
        require(std::abs(s.yaw - Session::coffeeYaw() - .2f) < .001f, "Controls remain locked");
      }
    }
    std::cout << "PASS all 15 coffee/machine combinations, fill/lift/drink/empty, input lock and "
                 "live world\n";
    {
      auto holder = std::make_unique<Session>(true, true);
      auto& s = *holder;
      stand(s);
      s.player = {0, CORE_HEIGHT + 1.72f, -9};
      require(!s.startCoffee(), "Coffee usable from far away");
      stand(s);
      s.yaw += PI;
      require(!s.startCoffee(), "Coffee usable facing away");
      stand(s);
      s.player = Session::coffeePoint({0, 1.72f, 2});
      s.yaw += PI;
      require(!s.startCoffee(), "Coffee usable through back panel");
      stand(s);
      s.move({0, 1, true}, 1);
      float a = Session::coffeeYaw();
      Vec3 off = s.player - Session::COFFEE_MACHINE;
      require(std::abs(off.x * std::cos(a) - off.z * std::sin(a)) >= 1.649f ||
                  std::abs(off.x * std::sin(a) + off.z * std::cos(a)) >= 1.199f,
              "Operator walks through machine");
      double remaining = s.coffee.awakeRemaining;
      require(s.debugGrant(Upgrade::CoffeeIV, Rarity::Common), "Cannot acquire tier IV directly");
      require(s.expedition.coffeeTier == 4 && s.coffee.awakeRemaining == remaining,
              "Acquisition applies caffeine without drinking");
      for (auto u : {Upgrade::CoffeeII, Upgrade::CoffeeIII, Upgrade::CoffeeIV})
        require(!s.debugGrant(u, Rarity::Legendary), "Duplicate or downgrade accepted");
      require(s.debugGrant(Upgrade::CoffeeV, Rarity::Common), "Tier V rejected");
      require(s.debugGrant(Upgrade::CoffeeMachineIII, Rarity::Common),
              "Direct legendary machine rejected");
      require(!s.debugGrant(Upgrade::CoffeeMachineII, Rarity::Rare) &&
                  !s.debugGrant(Upgrade::CoffeeMachineIII, Rarity::Legendary),
              "Machine downgrade accepted");
      s.expedition.rewards.push_back(
          {1, {Upgrade::CoffeeII, Upgrade::CoffeeV, Upgrade::CoffeeMachineII}});
      s.expedition.validateFront();
      for (auto u : s.expedition.rewards.front().choices)
        require(s.expedition.eligible(u), "Queued stale upgrade not replaced");
      stand(s);
      s.coffee.awakeRemaining = 0;
      require(s.startCoffee(), "Coffee after upgrades rejected");
      s.expedition.hp = 0;
      double age = s.coffee.age;
      tick(s, 4);
      require(s.coffee.age == age && s.coffee.cups == 0, "Death completes drink");
      auto fresh = std::make_unique<Session>(true);
      require(fresh->coffee.awakeRemaining == 300 && !fresh->coffee.active &&
                  fresh->expedition.coffeeTier == 1 && fresh->expedition.coffeeMachineTier == 1,
              "Restart retains coffee progression");
      std::cout
          << "PASS proximity, collision, upgrade jumps, no downgrades, queue refresh and reset\n";
    }
    {
      Expedition e(true, true);
      std::array<bool, 6> seen{};
      for (int n = 0; n < 60000; ++n) {
        e.addReward(n % 2 ? RewardSource::Uranium : RewardSource::BossCrystal);
        const auto& offer = e.rewards.back();
        for (int i = 0; i < 3; ++i) {
          auto u = offer.choices[i];
          if (coffeeUpgradeTier(u) || machineUpgradeTier(u)) {
            require(fixedRarity(u) == int(offer.rarities[i]), "Wrong coffee rarity");
            seen[int(u) - int(Upgrade::CoffeeII)] = true;
          }
        }
        e.rewards.clear();
      }
      for (bool value : seen)
        require(value, "Coffee upgrade unreachable");
      e.coffeeTier = 5;
      e.coffeeMachineTier = 3;
      for (int n = 0; n < 3000; ++n) {
        e.addReward();
        for (auto u : e.rewards.back().choices)
          require(!coffeeUpgradeTier(u) && !machineUpgradeTier(u), "Maxed coffee remains in reward pool");
        e.rewards.clear();
      }
      std::cout << "PASS natural reward coverage, exact rarities and removal of maxed upgrades\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
