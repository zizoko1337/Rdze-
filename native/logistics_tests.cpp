#include "command_game.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool value, const char* message) {
  if (!value)
    throw std::runtime_error(message);
}
void tick(Game& g, float seconds) {
  for (int n = 0; n < int(std::round(seconds * 60)); ++n)
    g.step(1.f / 60);
}
int foe(Game& g, float x, float z) {
  int id = g.spawn(Crawler, x, z);
  g.foes[id].speed = 0;
  g.foes[id].hp = g.foes[id].maxHp = 1000000;
  g.grid.rebuild(g.foes, g.active);
  return id;
}
int main() {
  try {
    require(coreHitFraction({-25, 5, 0}, {25, 5, 0}) < 1, "Cross-core ray accepted");
    require(coreHitFraction({-25, 25, 0}, {25, 25, 0}) > 1, "Ray above cylinder blocked");
    require(coreHitFraction({20, 5, 0}, {30, 5, 0}) > 1, "Same-side ray blocked");
    require(coreHitFraction({0, 30, 0}, {0, 0, 0}) < 1, "Core top ignored");
    require(coreHitFraction({-25, 5, 14}, {25, 5, 14}) > 1, "Tangent incorrectly blocked");
    for (int w = 0; w < WeaponCount; ++w) {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      g.towerRange = 2;
      require(g.addTower(Weapon(w), 0, 21), "Fixture tower rejected");
      int blocked = foe(g, 0, -21);
      g.focusTarget = blocked;
      g.focusGeneration = g.foes[blocked].generation;
      g.focusTime = 100;
      tick(g, 12);
      if (w == Mortar) {
        require(g.foes[blocked].hp < g.foes[blocked].maxHp && g.fired[w] > 0,
                "Mortar must fire across core");
      } else {
        require(g.foes[blocked].hp == g.foes[blocked].maxHp && g.fired[w] == 0,
                "Non-mortar fired through core or shared-focus bypassed obstruction");
        int visible = foe(g, 0, 34);
        tick(g, 12);
        require(g.foes[visible].hp < g.foes[visible].maxHp,
                "Blocked target prevents visible fallback");
        require(g.foes[blocked].hp == g.foes[blocked].maxHp,
                "Area weapon bypasses core obstruction");
      }
    }
    std::cout
        << "PASS only mortar crosses core; all eight others, focus, fallback and area filtering\n";
    for (auto type : {Rocket, Venom}) {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      Ordnance p;
      p.type = type;
      p.pos = p.start = {0, 5, 25};
      p.endpoint = {0, 5, -25};
      p.speed = 10000;
      p.damage = 100;
      p.radius = 10;
      g.shots.push_back(p);
      g.updateShots(.1f);
      require(g.shots.empty() && g.gas.empty() && g.damageDone[type] == 0,
              "Swept rocket/toxin crosses core or creates a blast behind it");
    }
    std::cout << "PASS swept projectiles stop at core, no tunnelling or poison cloud\n";
    {
      RoverFactories f;
      f.allocate(6, 2048);
      for (int n : f.lastBatch)
        require(n == 1, "Base factory does not get one rover");
      RoverFactories ten;
      ten.allocate(10, 2048);
      int pairs = 0, singles = 0;
      for (int n : ten.lastBatch) {
        pairs += n == 2;
        singles += n == 1;
      }
      require(pairs == 4 && singles == 2, "Ten-rover batch must be 2/2/2/2/1/1");
      std::array<int, 6> totals{};
      for (int batch = 6; batch <= 128; ++batch) {
        RoverFactories series;
        series.allocate(batch, 2048);
        auto [small, large] = std::minmax_element(series.lastBatch.begin(), series.lastBatch.end());
        require(*large - *small <= 1 && series.queuedCount() == batch, "Unbalanced series");
        f.queued.fill(0);
        f.allocate(batch, 2048);
        for (int i = 0; i < 6; ++i)
          totals[i] += f.lastBatch[i];
      }
      auto [small, large] = std::minmax_element(totals.begin(), totals.end());
      require(*large - *small <= 1, "Remainders not rotated fairly across series");
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      Frontier frontier;
      Expedition expedition(true, true);
      expedition.levels[size_t(Upgrade::RoverFactory)] = 1;
      require(expedition.roverBatch == 6, "Factory unlock must produce six");
      for (int n = 0; n < 180; ++n)
        frontier.stepRovers(g, expedition, 1.f / 60);
      require(frontier.roversMade == 0 && !frontier.factories.ready(),
              "Production before delivery");
      for (int i = 0; i < 6; ++i)
        require(frontier.factories.droneVisible(i), "Delivery lacks six drones");
      g.paused = true;
      float approachAge = frontier.factories.age, frozenSpawn = 100000;
      Vec3 approachPosition = frontier.factories.dronePosition(0);
      frontier.step(g, expedition, 1, frozenSpawn);
      require(frontier.factories.age == approachAge &&
                  length(frontier.factories.dronePosition(0) - approachPosition) == 0,
              "Paused drone keeps flying");
      g.paused = false;
      for (int n = 0; n < int(std::ceil((RoverFactories::READY_AT - 3 + .5f) * 60)); ++n)
        frontier.stepRovers(g, expedition, 1.f / 60);
      require(frontier.factories.ready() && frontier.factories.connection() == 1,
              "Factories never connect");
      require(frontier.roversMade == 6, "First full six-factory series missing");
      for (int i = 0; i < 6; ++i)
        require(frontier.factories.produced[i] == 1 && !frontier.factories.droneVisible(i),
                "Drone did not leave or factory did not produce");
      float age = frontier.factories.age, clock = frontier.productionClock;
      g.paused = true;
      float spawn = 100000;
      frontier.step(g, expedition, 1, spawn);
      require(frontier.factories.age == age && frontier.productionClock == clock,
              "Pause advances delivery or production");
    }
    std::cout << "PASS six deliveries, deferred activation, 6/10/128 balanced batches and pause\n";
    {
      RoverFactories convoy, alternative;
      convoy.initialize(12345);
      alternative.initialize(87654);
      bool varied = false;
      for (int i = 0; i < 6; ++i) {
        convoy.age = alternative.age = 0;
        Vec3 from = convoy.dronePosition(i);
        require(std::hypot(from.x, from.z) >= 145 && std::hypot(from.x, from.z) <= 215.01f &&
                    from.y >= 70 && from.y <= 105,
                "Drone must start far away, not above its factory");
        varied |= length(from - alternative.dronePosition(i)) > 10;
        convoy.age = RoverFactories::ARRIVAL + i * RoverFactories::STAGGER;
        Vec3 at = convoy.dronePosition(i), destination = RoverFactories::position(i);
        require(length(at - (destination + Vec3{0, 10, 0})) < .001f, "Approach misses drop site");
        require(std::hypot(from.x - at.x, from.z - at.z) > 120,
                "Approach has no horizontal travel");
        for (int frame = 0; frame <= 120; ++frame) {
          convoy.age = frame * RoverFactories::ARRIVAL / 120 + i * RoverFactories::STAGGER;
          Vec3 drone = convoy.dronePosition(i), cargo = convoy.cargoPosition(i);
          require(std::isfinite(drone.x) && std::isfinite(drone.y) && std::isfinite(drone.z),
                  "Invalid route");
          require(std::hypot(drone.x, drone.z) >= CORE_RADIUS + 3.59f,
                  "Drone crosses through core");
          require(length((drone - cargo) - Vec3{0, 4, 0}) < .001f,
                  "Cargo does not travel with drone");
        }
        for (float boundary :
             {RoverFactories::ARRIVAL, RoverFactories::ARRIVAL + RoverFactories::DROP}) {
          convoy.age = boundary + i * RoverFactories::STAGGER - .001f;
          Vec3 before = convoy.dronePosition(i), cargo = convoy.cargoPosition(i);
          convoy.age += .002f;
          require(length(convoy.dronePosition(i) - before) < .05f &&
                      length(convoy.cargoPosition(i) - cargo) < .05f,
                  "Delivery transition teleports");
        }
      }
      require(varied, "Different seeds produce identical convoy paths");
      RoverFactories slow, fast;
      slow.initialize(12345);
      fast.initialize(12345);
      for (int n = 0; n < 180; ++n)
        slow.advance(1.f / 30);
      for (int n = 0; n < 720; ++n)
        fast.advance(1.f / 120);
      for (int i = 0; i < 6; ++i)
        require(length(slow.dronePosition(i) - fast.dronePosition(i)) < .01f,
                "Route depends on frame rate");
      std::cout << "PASS distant randomized approaches, carried cargo, smooth transitions and "
                   "30/120 FPS\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
