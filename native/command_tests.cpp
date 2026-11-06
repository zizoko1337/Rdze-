#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
void run(Session& s, float seconds, Input input = {}) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    s.step(1.f / 60, input);
}
void aim(Session& s, Vec3 target) {
  Vec3 d = normalize(target - s.eye());
  s.yaw = std::atan2(d.x, d.z);
  s.pitch = std::asin(d.y);
  s.gunYaw = s.yaw;
  s.gunPitch = s.pitch;
}
void mount(Session& s, Gun gun = Gun::Pulse) {
  s.module.kind = gun;
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  s.spawnTimer = 100000;
}
int target(Session& s, float x = 0, float z = 95) {
  int id = s.world.spawn(Crawler, x, z);
  s.world.foes[id].speed = 0;
  s.world.grid.rebuild(s.world.foes, s.world.active);
  return id;
}
int main() {
  try {
    {
      Session s;
      mount(s);
      float initialPitch = s.pitch;
      s.look(.4f, -.15f);
      require(s.yaw == 0 && s.pitch == initialPitch, "Mounted camera snaps on mouse input");
      s.updateAim(.075f);
      require(s.yaw > .2f && s.yaw < .3f, "Inertia is too sluggish or too immediate");
      require(s.gunYaw == s.yaw && s.gunPitch == s.pitch, "Aim and gun disagree during smoothing");
      float previous = s.yaw;
      for (int i = 0; i < 120; ++i) {
        s.updateAim(1.f / 240);
        require(s.yaw >= previous && s.yaw <= .40001f, "Camera overshoots or oscillates");
        previous = s.yaw;
      }
      require(std::abs(s.yaw - .4f) < .001f, "Camera keeps drifting after input stops");
      Session low, high;
      mount(low);
      mount(high);
      low.look(1, .3f);
      high.look(1, .3f);
      for (int i = 0; i < 9; ++i)
        low.updateAim(1.f / 30);
      for (int i = 0; i < 72; ++i)
        high.updateAim(1.f / 240);
      require(std::abs(low.yaw - high.yaw) < .00001f, "Smoothing depends on frame rate");
      s.look(.5f, 10);
      s.world.paused = true;
      previous = s.yaw;
      s.updateAim(1);
      require(s.yaw == previous, "Paused camera still moves");
      s.world.paused = false;
      s.cancelAim();
      s.updateAim(1);
      require(s.yaw == previous, "Pointer release retains pending motion");
      s.mode = Mode::Walking;
      s.look(.2f, 0);
      require(std::abs(s.yaw - previous - .2f) < .00001f, "Walking view gained unwanted lag");
      Session edge;
      edge.mode = Mode::Walking;
      run(edge, 5, {1, 0});
      require(std::hypot(edge.player.x, edge.player.z) > 13 &&
                  std::hypot(edge.player.x, edge.player.z) <= Session::WALK_RADIUS + .001f,
              "Walking boundary does not follow outer railing");
      std::cout << "PASS light camera inertia, frame-rate independence, aim alignment and outer "
                   "railing boundary\n";
    }
    {
      Session s;
      run(s, 1);
      require(s.world.time == 0, "Selection advances combat");
      s.start(Gun::Crank);
      require(s.module.kind == Gun::Crank && s.world.towers.empty() && !s.world.autoWaves,
              "Wrong training configuration");
      require(!s.interact(), "Can enter seat from across deck");
      run(s, 1.1f, {0, 1});
      require(s.nearSeat(), "Approaching chair does not enable interaction");
      Vec3 before = s.player;
      require(s.interact(), "Cannot enter seat");
      run(s, .3f, {0, 1, false, true});
      require(s.shots == 0 && s.lift == 0 && s.hatch > 0,
              "Gun fires or lift moves through closed hatch");
      s.world.paused = true;
      float t = s.transition;
      run(s, 1);
      require(s.transition == t, "Paused lift moves");
      s.world.paused = false;
      run(s, 3.5f);
      require(s.mode == Mode::Mounted && s.eye().y > Session::ROOF + 2 && s.hatch == 1,
              "Seat did not rise above roof");
      run(s, 1, {1, 1});
      require(length(s.player - before) < .001f, "Walking while seated");
      require(s.interact(), "Cannot exit seat");
      run(s, 4);
      require(s.mode == Mode::Walking && s.lift == 0 && s.hatch == 0 &&
                  length(s.player - before) < .001f,
              "Return to deck failed");
      require(s.interact(), "Cannot reenter");
      run(s, .8f);
      s.interact();
      run(s, 4);
      require(s.mode == Mode::Walking && s.hatch == 0, "Cannot cancel ascent safely");
      std::cout << "PASS selection, proximity, hatch, lift, pause, exit and interrupted ascent\n";
    }
    {
      Session s;
      mount(s);
      int a = target(s), b = target(s, 0, 120);
      aim(s, s.world.targetPoint(a));
      s.fire();
      require(s.world.foes[a].hp <= 0 && s.world.foes[b].hp > 0,
              "Laser does not kill only first crawler");
      require(s.shots == 1, "Missing laser shot");
      s.fire();
      run(s, .9f, {0, 0, false, true});
      require(s.shots == 1, "Laser exceeds one shot per second");
      run(s, .12f, {0, 0, false, true});
      require(s.shots == 2, "Laser failed to recharge");
      auto shots = s.shots;
      s.world.paused = true;
      run(s, 2, {0, 0, false, true});
      s.fire();
      require(s.shots == shots, "Fires while paused");
      std::cout << "PASS laser one-shot crawler, nearest hit, 1 s cooldown and pause\n";
    }
    {
      Session s;
      mount(s);
      int id = target(s);
      auto miss = trace(s.world, s.eye(), {1, 0, 0}, 500);
      require(miss.id == -1, "Manual shot auto-aims at enemy");
      auto shortRay = trace(s.world, s.eye(), normalize(s.world.targetPoint(id) - s.eye()), 10);
      require(shortRay.id == -1, "Hit beyond weapon range");
      Vec3 p = s.world.targetPoint(id);
      auto fast = trace(s.world, p - Vec3{0, 0, 10}, {0, 0, 1}, 20);
      require(fast.id == id && fast.distance < 10, "Swept bullet tunnels through enemy");
      auto behind = trace(s.world, p + Vec3{0, 0, 10}, {0, 0, 1}, 100);
      require(behind.id == -1, "Hits enemy behind ray");
      std::cout << "PASS manual aim misses, finite range and swept nearest collision\n";
    }
    {
      Session s;
      mount(s, Gun::Crank);
      int id = target(s, 0, 70);
      s.world.foes[id].hp = 10000;
      aim(s, s.world.targetPoint(id));
      s.fire();
      require(s.impacts == 0 && s.bullets.size() == 1,
              "Minigun is hitscan instead of travelling bullets");
      Vec3 first = s.bullets[0].velocity;
      run(s, .1f, {0, 0, false, true});
      require(s.bullets.size() >= 2 && length(s.bullets.back().velocity - first) > .1f,
              "No real projectile spread");
      run(s, 2, {0, 0, false, true});
      require(s.shots >= 25 && s.shots <= 31 && s.impacts > 0 && s.impacts < s.shots,
              "Minigun cadence, damage or spread invalid");
      run(s, 4);
      require(s.bullets.empty(), "Projectile pool never expires");
      std::cout << "PASS moving minigun bullets, spread, cadence, impacts and lifetime\n";
    }
    {
      Session s;
      mount(s);
      int id = target(s);
      s.world.foes[id].hp = 250;
      s.module.damageMultiplier = 2;
      s.module.rateMultiplier = 2;
      aim(s, s.world.targetPoint(id));
      s.fire();
      require(s.world.foes[id].hp <= 0 && std::abs(s.cooldown - .5f) < .001f,
              "Module upgrade modifiers ignored");
      Session training;
      training.start(Gun::Pulse);
      run(training, 20);
      require(training.world.wave == 1 && training.world.active.size() <= 23 &&
                  training.world.towers.empty(),
              "Training unexpectedly escalates");
      for (int i : training.world.active)
        require(training.world.foes[i].type == Crawler && training.world.foes[i].maxHp == 100,
                "Training changes base enemy HP/type");
      std::cout << "PASS replaceable module modifiers and slow training spawns\n";
    }
    std::cout << "ALL COMMAND TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
