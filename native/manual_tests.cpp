#include "command_game.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool yes, const char* message) {
  if (!yes)
    throw std::runtime_error(message);
}
void tick(Session& s, float time, bool fire = false) {
  for (int i = 0; i < int(std::round(time * 60)); ++i)
    s.step(1.f / 60, {0, 0, false, fire});
}
void mount(Session& s, Gun gun) {
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  s.module.kind = gun;
  // These scenarios isolate flight and steering; acquisition delay has its own tests.
  if (gun == Gun::Smart)
    s.module.instantLock = true;
  s.spawnTimer = 100000;
}
int enemy(Session& s, float x, float z, float hp = 10000) {
  int id = s.world.spawn(Crawler, x, z);
  auto& f = s.world.foes[id];
  f.hp = f.maxHp = hp;
  f.speed = 0;
  s.world.grid.rebuild(s.world.foes, s.world.active);
  return id;
}
void aim(Session& s, Vec3 point) {
  for (int i = 0; i < 4; ++i) {
    Vec3 d = normalize(point - s.eye());
    s.yaw = s.gunYaw = std::atan2(d.x, d.z);
    s.pitch = s.gunPitch = std::asin(d.y);
  }
}
int main() {
  try {
    {
      Session s;
      mount(s, Gun::Arc);
      std::array<int, 12> ids;
      for (int i = 0; i < 12; ++i)
        ids[i] = enemy(s, i * 22.f, 100);
      aim(s, s.world.targetPoint(ids[0]));
      s.fire();
      for (int i = 0; i < 12; ++i)
        require(s.world.foes[ids[i]].hp == (i < 10 ? 9660 : 10000),
                "Tesla chain damage, 56m jumps or 10-target limit wrong");
      tick(s, 1, true);
      require(s.shots == 1, "Tesla ignores long cooldown");
      std::cout << "PASS aimed Tesla, chained jumps beyond blast radius, 10 unique targets and "
                   "cooldown\n";
    }
    {
      Session s;
      mount(s, Gun::Howitzer);
      int a = enemy(s, 0, 100), b = enemy(s, 10, 100), c = enemy(s, 25, 100);
      aim(s, s.world.targetPoint(a));
      s.fire();
      tick(s, .2f);
      require(s.world.foes[a].hp == 10000 && s.manualProjectiles.size() == 1,
              "Mortar damages before shell arrival");
      Vec3 p = s.manualProjectiles[0].pos;
      s.world.paused = true;
      tick(s, 2);
      require(length(s.manualProjectiles[0].pos - p) == 0, "Shell ignores pause");
      s.world.paused = false;
      tick(s, 3);
      require(s.manualProjectiles.empty() && s.world.foes[a].hp == 8900 &&
                  s.world.foes[b].hp == 8900 && s.world.foes[c].hp == 10000,
              "Mortar area damage or radius wrong");
      require(s.muzzle().x > 1, "Mortar muzzle is not beside the sight");
      std::cout << "PASS side-mounted mortar, travel, pause and real 15m / 1100 damage blast\n";
    }
    {
      Session s;
      mount(s, Gun::Toxin);
      int id = enemy(s, 0, 100);
      aim(s, s.world.targetPoint(id));
      s.fire();
      tick(s, .2f);
      require(s.toxicPatches.empty() && s.world.foes[id].hp == 10000,
              "Toxin cloud exists before grenade arrives");
      tick(s, 11);
      require(s.manualProjectiles.empty() && s.toxicPatches.empty(),
              "Toxin patch does not expire after eight seconds");
      tick(s, 8);
      // First poison application occurs on the patch's first 0.1s damage tick.
      require(std::abs(s.world.damageDone[Venom] - 798) < .1f &&
                  std::abs(s.world.foes[id].hp - 9202) < 1,
              "Toxin patch plus residual poison has incorrect accumulated damage");
      Session slow;
      mount(slow, Gun::Toxin);
      int foe = enemy(slow, 0, 70);
      slow.world.foes[foe].speed = 10;
      slow.toxicPatches.push_back({{0, .2f, 70}, 8, 0, 0});
      tick(slow, .2f);
      float before = slow.world.foes[foe].z;
      tick(slow, 1);
      require(std::abs(before - slow.world.foes[foe].z - 5) < .1f,
              "Toxin does not slow by fifty percent");
      slow.world.frost = true;
      before = slow.world.foes[foe].z;
      tick(slow, .2f);
      require(before - slow.world.foes[foe].z < 1, "Toxin slow does not stack with core frost");
      float life = slow.toxicPatches[0].life;
      slow.world.paused = true;
      tick(slow, 2);
      require(life == slow.toxicPatches[0].life, "Toxic patch ignores pause");
      std::cout << "PASS travelling toxin, exact patch lifetime / DOT, 50% slow, core stacking and "
                   "pause\n";
    }
    {
      Session s;
      mount(s, Gun::Smart);
      int far = enemy(s, 0, 160), near = enemy(s, 12, 85), outside = enemy(s, 100, 80);
      aim(s, s.world.targetPoint(far));
      s.updateSmartTarget();
      require(s.smartTarget == far, "Smart chooses nearest world enemy instead of sight centre");
      s.fire();
      require(s.manualProjectiles.size() == 1 && s.world.foes[far].hp == 10000,
              "Smart missile is hitscan");
      Vec3 initial = s.manualProjectiles[0].velocity;
      for (int i = 0; i < 180 && !s.manualProjectiles.empty(); ++i) {
        s.world.foes[far].x += .08f;
        s.step(1.f / 60);
      }
      require(s.manualProjectiles.empty() && s.world.foes[far].hp == 9820 &&
                  s.world.foes[near].hp == 10000 && s.world.foes[outside].hp == 10000 &&
                  initial.x != 0,
              "Smart missile fails to home on moving selected target");
      s.cooldown = 0;
      s.yaw = s.gunYaw = PI;
      s.fire();
      require(s.shots == 1, "Smart fires without target in window");
      Session reuse;
      mount(reuse, Gun::Smart);
      int old = enemy(reuse, 0, 160, 100);
      aim(reuse, reuse.world.targetPoint(old));
      reuse.fire();
      reuse.world.hurt(old, 100, Laser);
      reuse.step(1.f / 60);
      int recycled = enemy(reuse, 200, 300);
      require(old == recycled, "Recycled target fixture failed");
      tick(reuse, 5);
      require(reuse.world.foes[recycled].hp == 10000, "Smart retargets recycled enemy slot");
      std::cout << "PASS smart centre-priority window, curved flight, moving lock, no-target "
                   "safety and generation validation\n";
    }
    {
      Session s;
      mount(s, Gun::Rail);
      s.pitch = s.gunPitch = 0;
      std::array<int, 10> ids;
      for (int i = 0; i < 10; ++i) {
        ids[i] = enemy(s, 0, 50 + i * 30.f);
        s.world.foes[ids[i]].altitude = s.eye().y - s.world.foes[ids[i]].size * .47f;
      }
      int miss = enemy(s, 20, 100);
      s.fire();
      for (int i = 0; i < 10; ++i)
        require(std::abs(s.world.foes[ids[i]].hp -
                         (i < 8 ? 10000 - 750 * (1 + length(s.world.targetPoint(ids[i]) - s.eye()) *
                                                         .01f)
                                : 10000)) < .02f,
                "Railgun piercing is not capped at eight nearest intersections");
      require(s.world.foes[miss].hp == 10000, "Railgun damages targets outside ray");
      std::cout << "PASS railgun geometric piercing, nearest-eight cap and accurate misses\n";
    }
    {
      for (float distance : {100.f, 200.f, 500.f}) {
        Session s;
        mount(s, Gun::Rail);
        s.pitch = s.gunPitch = 0;
        int id = enemy(s, 0, s.eye().z + distance);
        s.world.foes[id].altitude = s.eye().y - s.world.foes[id].size * .47f;
        s.fire();
        require(std::abs(s.world.foes[id].hp - (10000 - 750 * (1 + distance * .01f))) < .02f,
                "Sniper damage does not increase by one percent per metre to each target");
      }
      std::cout << "PASS sniper 100/200/500m damage: 1500/2250/4500\n";
    }
    {
      Session s;
      mount(s, Gun::Lance);
      int id = enemy(s, 0, 100);
      aim(s, s.world.targetPoint(id));
      tick(s, 1.7f, true);
      require(s.world.foes[id].hp == 10000 && s.shots == 0, "Lance fires before charging");
      float charge = s.lanceCharge;
      s.world.paused = true;
      tick(s, 2, true);
      require(s.lanceCharge == charge, "Lance charge ignores pause");
      s.world.paused = false;
      tick(s, 3.3f, true);
      require(s.module.spec().damage == WEAPONS[arsenal::Lance].damage,
              "Manual and automatic lance have different base beam DPS");
      require(s.shots == 1 && std::abs(s.world.foes[id].hp - (10000 - 3 * 1650 - 200)) < 2 && s.cooldown > 0,
              "Lance full three-second beam damage / cooling wrong");
      Session cancel;
      mount(cancel, Gun::Lance);
      tick(cancel, 1, true);
      tick(cancel, .1f);
      require(cancel.lanceCharge == 0 && cancel.shots == 0, "Partial charge survives release");
      std::cout << "PASS lance charge, pause, 3s continuous 1650DPS beam, cooling and release "
                   "cancellation\n";
    }
    {
      Session s;
      mount(s, Gun::Flame);
      int a = enemy(s, 0, 40), b = enemy(s, 17, 40), side = enemy(s, 40, 40), far = enemy(s, 0, 90);
      aim(s, s.world.targetPoint(a));
      s.fire();
      require(s.world.foes[a].hp == 9976 && s.world.foes[b].hp == 9976 &&
                  s.world.foes[side].hp == 10000 && s.world.foes[far].hp == 10000,
              "Flame cone or 63m range wrong");
      tick(s, 3.1f);
      require(std::abs(s.world.foes[a].hp - 9886) < 1, "Flame burn is not 30DPS for three seconds");
      int edge = enemy(s, 0, s.eye().z + 62.5f);
      s.cooldown = 0;
      s.fire();
      require(s.world.foes[edge].hp == 9976,
              "Flame cannot reach three hexes along ground from elevated core");
      std::cout << "PASS 63m / three-hex flame cone, area packet and persistent afterburn\n";
    }
    {
      for (int kind = 0; kind < GUN_COUNT; ++kind) {
        Session s;
        mount(s, Gun(kind));
        int id = enemy(s, 0, kind == int(Gun::Flame) ? 40 : 100);
        aim(s, s.world.targetPoint(id));
        s.module.damageMultiplier = 2;
        s.module.rateMultiplier = 1.5f;
        tick(s, 9, true);
        require(s.world.foes[id].hp < 10000 && s.shots > 0,
                "Upgraded manual module does not actually damage enemy");
      }
      std::cout
          << "PASS all nine replaceable modules, held firing and shared damage/rate upgrades\n";
    }
    {
      Session s;
      mount(s, Gun::Flame);
      int primary = enemy(s, 0, 40), neighbour = enemy(s, 10, 40);
      s.expedition.levels[size_t(Upgrade::Focus)] = 1;
      s.world.focusDamageBonus = .4f;
      aim(s, s.world.targetPoint(primary));
      s.fire();
      require(s.world.focusTarget == primary && s.world.rings.size() == 1 &&
                  std::abs(s.world.foes[primary].hp - 9966.4f) < .01f &&
                  s.world.foes[neighbour].hp == 9976,
              "Area fire marks every target or amplifies all enemies instead of primary");
      s.toxicPatches.push_back({{10, .2f, 40}, 8, 0, 65});
      s.stepToxicPatches(.1f);
      require(s.world.focusTarget == primary && s.world.rings.size() == 1,
              "Toxin DOT changes focused target or creates a marker for every enemy tick");
      float beforeA = s.world.foes[primary].hp, beforeB = s.world.foes[neighbour].hp;
      s.manualBlast({0, .2f, 40}, 15, 100, Mortar, primary);
      require(std::abs(beforeA - s.world.foes[primary].hp - 140) < .01f &&
                  std::abs(beforeB - s.world.foes[neighbour].hp - 100) < .01f,
              "Blast focus bonus incorrectly affects whole area");
      std::cout << "PASS stable primary focus for area weapons, single-target bonus and bounded "
                   "marker effects\n";
    }
    std::cout << "ALL MANUAL ARSENAL TESTS PASSED\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
