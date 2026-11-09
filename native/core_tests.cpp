#include "deck_instruments.hpp"
#include "radar_scene.hpp"
#include <iostream>
#include <stdexcept>
using namespace command;
void require(bool b, const char* message) {
  if (!b)
    throw std::runtime_error(message);
}
void tick(Session& s, float seconds) {
  for (int n = 0; n < int(std::round(seconds * 60)); ++n)
    s.step(1.f / 60);
}
int main() {
  try {
    {
      for (float heading : {0.f, PI * .5f, PI, -PI * .5f, .73f}) {
        auto ahead = radarLocal(std::sin(heading) * 100, std::cos(heading) * 100, heading);
        auto right = radarLocal(std::cos(heading) * 100, -std::sin(heading) * 100, heading);
        require(std::abs(ahead.x) < .001f && std::abs(ahead.z - 100) < .001f,
                "Looking direction is not always at top of radar");
        require(std::abs(right.x - 100) < .001f && std::abs(right.z) < .001f,
                "Radar mirrors targets to the right");
      }
      for (const auto& upgrade : UPGRADES)
        require(std::string(upgrade.name).find("Radar") == std::string::npos,
                "Default radar still occupies an upgrade card");
      std::cout << "PASS heading-up radar, cardinal and oblique directions, no radar upgrade\n";
    }
    {
      Expedition e(true, true);
      e.hp = 500;
      e.step(10, 0);
      require(e.regeneration == 3 && e.hp == 530, "Core base regeneration is not 3 HP/s");
      e.hp = 1499;
      e.step(1, 0);
      require(e.hp == e.maxHp, "Regeneration exceeds maximum HP");
      e.hp = 1;
      e.step(1, 2);
      require(e.dead() && e.hp == 0, "Regeneration prevents lethal damage");
      e.step(10, 0);
      require(e.hp == 0, "Regeneration resurrects dead core");
      std::cout << "PASS base healing, max HP clamp, lethal priority and no resurrection\n";
    }
    {
      Session s(true);
      s.start(Gun::Pulse);
      s.expedition.hp = 1000;
      tick(s, 1);
      require(s.expedition.hp == 1000, "Regeneration starts before extraction");
      s.startExtraction();
      s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = 100000;
      s.world.active.clear();
      tick(s, 2);
      require(std::abs(s.expedition.hp - 1006) < .02f, "Fixed-step healing inaccurate");
      float hp = s.expedition.hp;
      s.world.paused = true;
      tick(s, 1);
      s.world.paused = false;
      s.terminalOpen = true;
      tick(s, 1);
      s.terminalOpen = false;
      s.servicesOpen = true;
      tick(s, 1);
      s.servicesOpen = false;
      s.upgradeLabOpen = true;
      tick(s, 1);
      s.upgradeLabOpen = false;
      s.bestiaryOpen = true;
      tick(s, 1);
      s.bestiaryOpen = false;
      require(s.expedition.hp == hp, "Menu or pause permits free healing");
      float expected = 3;
      for (int i = 0; i < 5; ++i) {
        require(s.debugGrant(Upgrade::Nanobots, Rarity(i)), "Nanobot upgrade unavailable");
        expected += RARITIES[i].power;
        require(s.expedition.regeneration == expected, "Nanobot rarity scaling incorrect");
        require(upgradeDescription(Upgrade::Nanobots, Rarity(i)).find("HP/s") != std::string::npos,
                "Nanobot card omits actual effect");
      }
      hp = s.expedition.hp;
      tick(s, 1);
      require(std::abs(s.expedition.hp - hp - expected) < .02f,
              "Stacked nanobots do not affect actual healing");
      require(s.expedition.levels[size_t(Upgrade::Nanobots)] == 5,
              "Nanobots are incorrectly unique");
      // Regeneration must not hide a crossed emergency-purge threshold.
      s.expedition.levels[size_t(Upgrade::Purge)] = 1;
      s.expedition.hp = s.expedition.maxHp * .5f - .001f;
      tick(s, 1.f / 60);
      require(s.expedition.purgeUsed, "Regeneration masks emergency threshold");
      std::cout
          << "PASS extraction gate, menu freeze, all five nanobot tiers, stacking and purge\n";
    }
    {
      Session s(true);
      s.start(Gun::Pulse);
      for (float side : {-1.f, 1.f}) {
        float yaw = consoleYaw(side);
        Vec3 facing{-std::sin(yaw), 0, -std::cos(yaw)};
        Vec3 towardCenter = normalize(Vec3{-side * 6.8f, 0, 3.5f});
        require(dot(facing, towardCenter) > .9999f, "Console does not face center");
        s.player = {side * 6.8f, CORE_HEIGHT + 1.72f, -3.5f};
        s.move({}, .01f);
        float x = s.player.x - side * 6.8f, z = s.player.z + 3.5f;
        float localX = x * std::cos(yaw) - z * std::sin(yaw);
        float localZ = x * std::sin(yaw) + z * std::cos(yaw);
        require(std::abs(localX) >= 1.899f || std::abs(localZ) >= 1.249f,
                "Console collider does not rotate with housing");
      }
      s.player = {0, CORE_HEIGHT + 1.72f, -9};
      s.move({}, .01f);
      require(s.player.z == -9, "Pump blocks initial spawn");
      s.player = {0, CORE_HEIGHT + 1.72f, Session::URANIUM_PUMP.z};
      s.move({}, .01f);
      require(std::abs(s.player.z - Session::URANIUM_PUMP.z) >= 1.249f,
              "Player can walk through pump");
      require(Session::URANIUM_PUMP.z < 0 && Session::START_BUTTON.z > 0,
              "Pump is not opposite start button");
      std::cout << "PASS inward consoles, rotated collisions, pump placement and clear spawn\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
