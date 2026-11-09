#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool b, const char* message) {
  if (!b)
    throw std::runtime_error(message);
}
void setup(Session& s, Gun gun) {
  s.start(gun);
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  for (int id : s.world.active)
    s.world.foes[id].hp = 0;
  s.world.step(.01f);
  s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
}
int enemy(Session& s, float x, float z) {
  int id = s.world.spawn(Crawler, x, z);
  s.world.foes[id].hp = s.world.foes[id].maxHp = 10000;
  s.world.foes[id].speed = 0;
  s.world.grid.rebuild(s.world.foes, s.world.active);
  return id;
}
void aim(Session& s, Vec3 target) {
  for (int i = 0; i < 5; ++i) {
    Vec3 d = normalize(target - s.eye());
    s.yaw = s.gunYaw = std::atan2(d.x, d.z);
    s.pitch = s.gunPitch = std::asin(d.y);
  }
}
int main() {
  try {
    {
      Session s;
      setup(s, Gun::Arc);
      std::array<int, 10> ids;
      for (int i = 0; i < 10; ++i)
        ids[i] = enemy(s, 20 + i * 45.f, 100);
      aim(s, {0, 0, 100});
      s.fire();
      for (int i = 0; i < 10; ++i)
        require(std::abs(s.world.foes[ids[i]].hp - (i < 9 ? 9660 : 10000)) < .1f,
                "Ground Tesla does not consume one link or fails 56m jump");
      Session direct;
      setup(direct, Gun::Arc);
      int a = enemy(direct, 0, 100), b = enemy(direct, 50, 100), outside = enemy(direct, 110, 100);
      aim(direct, direct.world.targetPoint(a));
      direct.fire();
      require(direct.world.foes[a].hp == 9660 && direct.world.foes[b].hp == 9660 &&
                  direct.world.foes[outside].hp == 10000,
              "Direct Tesla 56m boundary wrong");
      std::cout << "PASS Tesla ground origin, first-link cost, 56m jumps and direct range limit\n";
    }
    {
      Session s;
      setup(s, Gun::Lance);
      int a = enemy(s, 0, 100), b = enemy(s, 30, 100);
      aim(s, s.world.targetPoint(a));
      s.lanceCharge = 1.8f;
      s.stepLance(.01f, true);
      require(std::abs(s.world.foes[a].hp - 9793.6f) < .02f,
              "Lance first contact lacks 200 damage");
      s.stepLance(.01f, true);
      require(std::abs(s.world.foes[a].hp - 9787.2f) < .02f, "Contact bonus repeats every tick");
      aim(s, s.world.targetPoint(b));
      s.stepLance(.01f, true);
      require(std::abs(s.world.foes[b].hp - 9793.6f) < .02f,
              "Sweeping onto new target lacks bonus");
      aim(s, s.world.targetPoint(a));
      s.stepLance(.01f, true);
      require(std::abs(s.world.foes[a].hp - 9780.8f) < .03f,
              "Returning to target repeats contact bonus");
      require(s.world.beams.empty(), "Lance still emits old tick-based beam segments");
      s.world.hurt(a, s.world.foes[a].hp, Laser);
      s.world.step(.01f);
      int reused = enemy(s, 0, 100);
      require(reused == a, "Reuse fixture wrong");
      aim(s, s.world.targetPoint(reused));
      s.stepLance(.01f, true);
      require(std::abs(s.world.foes[reused].hp - 9793.6f) < .03f,
              "New generation in same slot loses contact bonus");
      std::cout
          << "PASS lance 200 first-contact, sweeps, no repeat, safe reuse and no stale beam FX\n";
    }
    {
      Session s(true, true);
      setup(s, Gun::Lance);
      require(s.debugGrant(Upgrade::UraniumReserve, Rarity::Common) && s.module.uraniumReserve &&
                  minimumRarity(Upgrade::UraniumReserve) == Rarity::Legendary &&
                  !s.debugGrant(Upgrade::UraniumReserve, Rarity::Legendary),
              "Uranium reserve eligibility or uniqueness wrong");
      s.expedition.uranium = 10;
      s.expedition.miningMultiplier = 0;
      s.lanceCharge = 1.8f;
      for (int i = 0; i < 60; ++i)
        s.stepLance(.05f, true);
      require(s.lanceActive && std::abs(s.expedition.uranium - 10) < .002f,
              "Reserve consumes uranium during normal three seconds");
      s.stepLance(.5f, true);
      require(s.lanceReserveActive && std::abs(s.expedition.uranium - 7.5f) < .002f,
              "Reserve does not continue beam at 5 uranium/s");
      s.world.paused = true;
      s.step(.05f, {0, 0, false, true});
      require(std::abs(s.expedition.uranium - 7.5f) < .002f, "Paused reserve consumes fuel");
      s.world.paused = false;
      s.stepLance(.01f, false);
      require(!s.lanceActive && !s.lanceReserveActive && s.cooldown > 0,
              "Release does not cut reserve");
      s.cooldown = 0;
      s.lanceActive = true;
      s.lanceRemaining = 0;
      s.expedition.uranium = .1f;
      s.stepLance(.1f, true);
      require(!s.lanceActive && s.expedition.uranium == 0 && s.cooldown > 0,
              "Fuel exhaustion underflows or fails to stop beam");
      Session wrong(true, true);
      setup(wrong, Gun::Pulse);
      require(!wrong.expedition.eligible(Upgrade::UraniumReserve) &&
                  !wrong.debugGrant(Upgrade::UraniumReserve, Rarity::Legendary),
              "Wrong gun receives lance reserve");
      std::cout << "PASS Legendary lance-only reserve, three-second switch, fuel, pause, release "
                   "and exhaustion\n";
    }
    {
      Session s;
      setup(s, Gun::Flame);
      require(s.toggleFlame() && s.flameFocused && s.flameRange() == 94.5f &&
                  std::abs(2 * std::tan(s.flameHalfAngle()) * s.flameRange() - 5) < .001f,
              "Focused flame range or five-metre width wrong");
      s.pitch = s.gunPitch = 0;
      int center = enemy(s, 0, 90), wide = enemy(s, 3, 90), distant = enemy(s, 0, 96);
      for (int id : {center, wide, distant})
        s.world.foes[id].altitude = s.eye().y - s.world.foes[id].size * .47f;
      s.fire();
      require(s.world.foes[center].hp == 9976 && s.world.foes[wide].hp == 10000 &&
                  s.world.foes[distant].hp == 10000,
              "Focused flame damage cone wrong");
      s.step(.05f);
      require(s.flameMorph > 0, "Nozzle does not animate");
      require(s.toggleFlame() && !s.flameFocused && s.flameRange() == 63,
              "Cannot return to wide mode");
      s.world.paused = true;
      require(!s.toggleFlame(), "Mode changes through paused UI");
      std::cout
          << "PASS focused 94.5m/5m flame cone, actual hits, nozzle transition and mode toggle\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
