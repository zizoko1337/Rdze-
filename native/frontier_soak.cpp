#include "command_game.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace command;
// Optimistic reference defence: real rewards/combat, instantaneous visits to the computer,
// accurate manual aim. It checks pacing, not the difficulty of human movement or aiming.
int main() {
  Session s(true, true);
  s.start(Gun::Pulse);
  s.mode = Mode::Mounted;
  s.lift = s.hatch = 1;
  auto out = std::filesystem::path("artifacts/frontier");
  std::filesystem::create_directories(out);
  std::ofstream csv(out / "balance-15min.csv");
  csv << "seconds,hp,max_hp,alive,kills,bosses,killed_bosses,towers,upgrades,rovers,crystals\n";
  int nextSample = 60;
  size_t peak = 0;
  auto score = [&](Upgrade u) {
    if (u == Upgrade::Hull && s.expedition.hp < s.expedition.maxHp * .7f)
      return 300;
    if (isTower(u))
      return isSupport(towerWeapon(u)) ? 100 : s.world.towers.size() < 20 ? 200 : 110;
    switch (u) {
    case Upgrade::Mining:
      return s.world.time < 500 ? 170 : 60;
    case Upgrade::Purge:
      return 190;
    case Upgrade::RoverFactory:
      return 160;
    case Upgrade::TowerDamage:
      return 150;
    case Upgrade::Damage:
      return 130;
    case Upgrade::FireRate:
      return 125;
    case Upgrade::TowerRate:
      return 140;
    case Upgrade::TowerRange:
      return 135;
    case Upgrade::Frost:
      return 145;
    case Upgrade::Focus:
      return 120;
    case Upgrade::RoverBatch:
      return 115;
    case Upgrade::RoverRate:
      return 114;
    default:
      return 10;
    }
  };
  for (int frame = 0; frame < 900 * 60 && !s.expedition.dead(); ++frame) {
    while (!s.expedition.rewards.empty()) {
      s.expedition.validateFront();
      int choice = 0;
      const auto& cards = s.expedition.rewards.front().choices;
      for (int i = 1; i < 3; ++i)
        if (score(cards[i]) > score(cards[choice]))
          choice = i;
      s.terminalOpen = true;
      if (!s.chooseUpgrade(choice))
        return 2;
    }
    while (s.sentryStock > 0) {
      int best = -1;
      float cost = 1e30f;
      for (int i = 0; i < SLOT_COUNT; ++i)
        if (s.slotAvailable(i)) {
          const auto& p = TURRET_SLOTS[i];
          float value = p.x * p.x + p.z * p.z;
          if (isSupport(s.pendingSentry()))
            for (const auto& t : s.world.towers)
              if (!isSupport(t.type) && supportNeighbor(p.x, p.z, t.x, t.z))
                value -= 10000;
          if (value < cost) {
            best = i;
            cost = value;
          }
        }
      if (best < 0 || !s.orderSentry(best))
        return 3;
    }
    s.tactical = false;
    s.frontier.requestCollection(s.frontier.nearestCrystal());
    int target = s.world.grid.nearest(0, 0, 500, s.world.foes);
    for (int id : s.world.active)
      if (s.world.foes[id].hp > 0 && s.world.foes[id].type >= Titan) {
        target = id;
        break;
      }
    if (target >= 0)
      for (int adjust = 0; adjust < 2; ++adjust) {
        Vec3 d = normalize(s.world.targetPoint(target) - s.eye());
        s.yaw = s.gunYaw = std::atan2(d.x, d.z);
        s.pitch = s.gunPitch = std::asin(d.y);
      }
    s.step(1.f / 60, {0, 0, false, target >= 0});
    peak = std::max(peak, s.world.active.size());
    if (s.world.time >= nextSample) {
      csv << nextSample << ',' << s.expedition.hp << ',' << s.expedition.maxHp << ','
          << s.world.active.size() << ',' << s.world.kills << ',' << s.frontier.bossesSpawned << ','
          << s.world.bossKills << ',' << s.world.towers.size() << ',' << s.expedition.selected
          << ',' << s.frontier.roversMade << ',' << s.frontier.collected << '\n';
      nextSample += 60;
    }
  }
  bool pass = !s.expedition.dead() && s.world.time > 895 && s.frontier.bossesSpawned >= 4;
  std::ofstream json(out / "balance-15min.json");
  json << "{\n  \"completed_15_minutes\": " << (pass ? "true" : "false")
       << ",\n  \"seconds\": " << s.world.time << ",\n  \"hp\": " << s.expedition.hp
       << ",\n  \"max_hp\": " << s.expedition.maxHp << ",\n  \"peak_enemies\": " << peak
       << ",\n  \"kills\": " << s.world.kills
       << ",\n  \"bosses_spawned\": " << s.frontier.bossesSpawned
       << ",\n  \"bosses_killed\": " << s.world.bossKills
       << ",\n  \"towers\": " << s.world.towers.size()
       << ",\n  \"upgrades\": " << s.expedition.selected
       << ",\n  \"crystals_collected\": " << s.frontier.collected
       << ",\n  \"rover_hits\": " << s.frontier.roverHits
       << ",\n  \"instant_terminal_and_accurate_manual_aim\": true\n}\n";
  std::cout << "BALANCE " << (pass ? "PASS" : "FAIL") << " time=" << s.world.time
            << " hp=" << s.expedition.hp << " kills=" << s.world.kills
            << " bosses=" << s.world.bossKills << '/' << s.frontier.bossesSpawned
            << " towers=" << s.world.towers.size() << " peak=" << peak << '\n';
  return pass ? 0 : 1;
}
