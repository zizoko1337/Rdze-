#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool value, const char* message) {
  if (!value)
    throw std::runtime_error(message);
}
void mount(Session& s, Gun gun) {
  s.module.kind = gun;
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  s.spawnTimer = 100000;
}
void aim(Session& s, Vec3 point) {
  for (int n = 0; n < 4; ++n) {
    Vec3 d = normalize(point - s.eye());
    s.gunYaw = s.yaw = std::atan2(d.x, d.z);
    s.gunPitch = s.pitch = std::asin(d.y);
  }
}
int main() {
  try {
    {
      GroundMarks pool;
      pool.spot({0, 20, 0}, 1, 3);
      pool.spot({std::numeric_limits<float>::quiet_NaN(), 0, 0}, 1, 3);
      require(pool.marks.empty(), "Airborne or non-finite marks accepted");
      for (int i = 0; i < 10000; ++i)
        pool.spot({float(i), 0, 0}, .1f, 3);
      require(pool.marks.size() == GroundMarks::capacity, "Unbounded ground mark storage");
      pool.tick(4);
      require(pool.marks.empty(), "Ground marks never expire");
      for (int i = 0; i < 10000; ++i)
        pool.spot({0, 0, 0}, .3f, 4, 1);
      require(pool.marks.size() == 1, "Stationary fire piles up ground opacity");
      std::cout << "PASS bounded pool, expiry, stationary merging and invalid impacts\n";
    }
    for (Gun gun : {Gun::Pulse, Gun::Rail, Gun::Arc, Gun::Flame}) {
      Session s;
      mount(s, gun);
      aim(s, {0, 0, 35});
      s.fire();
      require(!s.world.groundMarks.marks.empty(), "Ground shot left no mark");
      auto count = s.world.groundMarks.marks.size();
      s.cooldown = 0;
      s.gunPitch = s.pitch = .3f;
      s.fire();
      require(s.world.groundMarks.marks.size() == count, "Sky shot left a ground mark");
    }
    {
      Session s;
      mount(s, Gun::Pulse);
      int id = s.world.spawn(Crawler, 0, 35);
      s.world.grid.rebuild(s.world.foes, s.world.active);
      aim(s, s.world.targetPoint(id));
      s.fire();
      require(s.world.groundMarks.marks.empty(), "Enemy hit painted the ground behind it");
    }
    {
      Session s;
      mount(s, Gun::Crank);
      s.bullets.push_back({{0, 1, 35}, {0, 1, 35}, {0, -200, 0}, 1, 10});
      s.step(.01f);
      require(s.bullets.empty() && s.world.groundMarks.marks.size() == 1,
              "Fast minigun round skipped the floor impact");
      float life = s.world.groundMarks.marks[0].life;
      s.world.paused = true;
      s.step(.05f);
      require(s.world.groundMarks.marks[0].life == life, "Pause ages ground marks");
      s.world.paused = false;
      for (int n = 0; n < 300; ++n)
        s.step(.01f);
      require(s.world.groundMarks.marks.empty(), "Minigun mark failed to fade");
      std::cout
          << "PASS actual laser, rail, Tesla, flame and swept minigun impacts; sky, enemy, pause\n";
    }
    {
      Session s;
      mount(s, Gun::Lance);
      aim(s, {0, 0, 45});
      s.lanceActive = true;
      s.lanceRemaining = 3;
      s.stepLance(.04f, true);
      aim(s, {2, 0, 45});
      s.stepLance(.04f, true);
      require(s.world.groundMarks.marks.size() == 2 &&
                  length(s.world.groundMarks.marks.back().b - s.world.groundMarks.marks.back().a) >
                      1,
              "Lance sweep has no connected trail");
      s.gunPitch = s.pitch = .3f;
      s.stepLance(.04f, true);
      aim(s, {4, 0, 45});
      s.stepLance(.04f, true);
      auto& m = s.world.groundMarks.marks.back();
      require(length(m.b - m.a) < .001f, "Lance draws a bridge through sky/enemy interruption");
      s.world.groundMarks.tick(2);
      require(!s.world.groundMarks.marks.empty() && m.duration - m.life > m.heat,
              "Lance never cools from glow to scorch");
      s.world.groundMarks.tick(3);
      require(s.world.groundMarks.marks.empty(), "Lance trail never disappears");
      std::cout << "PASS connected lance trail, interruption, cooling and disappearance\n";
    }
    {
      Session s;
      mount(s, Gun::Howitzer);
      aim(s, {0, 0, 45});
      s.fire();
      for (int n = 0; n < 1000 && !s.manualProjectiles.empty(); ++n)
        s.stepManualProjectiles(.01f);
      require(!s.world.groundMarks.marks.empty(), "Ground explosion leaves no scorch");
      s.world.groundMarks.marks.clear();
      ManualProjectile p{};
      p.kind = Gun::Smart;
      p.pos = p.endpoint = {0, 20, 45};
      p.age = 6;
      p.damage = 1;
      s.manualProjectiles.push_back(p);
      s.stepManualProjectiles(.01f);
      require(s.world.groundMarks.marks.empty(), "Air explosion scorches distant floor");
      std::cout << "PASS ground explosion scorch and airborne explosion rejection\n";
    }
    std::cout << "ALL GROUND EFFECT TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
