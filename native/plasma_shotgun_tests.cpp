#include "command_game.hpp"
#include "manual_sound.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool v, const char* m) {
  if (!v)
    throw std::runtime_error(m);
}
void tick(Session& s, float seconds, bool held = false) {
  for (int n = 0; n < int(std::round(seconds * 60)); ++n)
    s.step(1.f / 60, {0, 0, false, held});
}
void aim(Session& s, Vec3 point) {
  for (int n = 0; n < 5; ++n) {
    auto d = normalize(point - s.eye());
    s.yaw = s.gunYaw = std::atan2(d.x, d.z);
    s.pitch = s.gunPitch = std::asin(d.y);
  }
}
int main() {
  try {
    {
      auto session = std::make_unique<Session>();
      auto& s = *session;
      s.mode = Mode::Mounted;
      s.lift = s.hatch = 1;
      s.module.kind = Gun::Shotgun;
      s.world.autoWaves = false;
      s.spawnTimer = 100000;
      int id = s.world.spawn(Titan, 0, 100);
      s.world.foes[id].speed = 0;
      s.world.foes[id].hp = s.world.foes[id].maxHp = 100000;
      s.world.grid.rebuild(s.world.foes, s.world.active);
      aim(s, s.world.targetPoint(id));
      float pitch = s.pitch;
      Vec3 direction = s.forward();
      s.fire();
      require(s.manualProjectiles.size() == 10 && s.shots == 1 && s.shotgun.rounds == 1 &&
                  !s.shotgun.reloading,
              "First barrel must fire ten pellets and leave second barrel loaded");
      require(std::abs(s.pitch - pitch - PI / 18) < .0001f,
              "Shot does not kick actual aim up by 10 degrees");
      for (const auto& p : s.manualProjectiles)
        require(p.damage == 3000 && p.emitter == 0 && dot(normalize(p.velocity), direction) > .999f,
                "Pellet damage, barrel or pre-recoil trajectory wrong");
      tick(s, .8f, true);
      require(s.shots == 1 && s.world.foes[id].hp == 70000,
              "Holding trigger fires both barrels or first shot is not 30000 damage");
      tick(s, .02f);
      aim(s, s.world.targetPoint(id));
      s.fire();
      require(s.shots == 2 && s.shotgun.rounds == 0 && s.shotgun.reloading &&
                  s.manualProjectiles.size() == 10,
              "Second barrel does not start full two-battery reload");
      for (const auto& p : s.manualProjectiles)
        require(p.emitter == 1, "Second shot comes from wrong barrel");
      tick(s, .7f, true);
      require(s.world.foes[id].hp == 40000, "Two shots do not deal 60000 nominal damage");
      float age = s.shotgun.age;
      s.world.paused = true;
      tick(s, 1);
      s.world.paused = false;
      s.terminalOpen = true;
      tick(s, 1);
      s.terminalOpen = false;
      require(s.shotgun.age == age, "Paused game or terminal advances reload");
      tick(s, 10.8f, true);
      require(s.shots == 2 && s.shotgun.reloading, "Third shot fires before lever returns");
      tick(s, .6f, true);
      require(!s.shotgun.reloading && s.shotgun.rounds == 2 && s.shots == 2,
              "Reload does not restore two batteries or held trigger fires automatically");
      tick(s, .02f);
      aim(s, s.world.targetPoint(id));
      s.fire();
      require(s.shots == 3, "Fresh trigger cannot fire reloaded gun");
      std::cout << "PASS two independent 10x3000 shots, alternating barrels, real +10deg aim, "
                   "trigger release, reload gate and pause\n";
    }
    {
      PlasmaShotgun cycle;
      cycle.begin(1);
      auto pose = [&](float p) { cycle.age = cycle.duration * p; };
      pose(.04f);
      require(cycle.elevation() > 0 && cycle.elevation() < 1.2f && cycle.breakAngle() == 0 &&
                  cycle.extraction() == 0,
              "Barrels open before the elevator has raised the carriage");
      pose(.085f);
      require(std::abs(cycle.elevation() - 1.2f) < .0001f &&
                  std::abs(cycle.extension() - .55f) < .0001f && cycle.breakAngle() == 0,
              "Lift must finish before the hinge starts opening");
      pose(.1f);
      require(cycle.breakAngle() > 0 && cycle.extraction() == 0,
              "Break must precede battery removal");
      pose(.325f);
      require(cycle.extraction() > 0 && cycle.extraction() < 1 && cycle.spentVisible(0) &&
                  cycle.spentVisible(1),
              "Pair extraction is missing");
      require(length((cycle.batteryPoint(1, false) - cycle.batteryPoint(0, false)) -
                     Vec3{1, 0, 0}) < .0001f,
              "Extraction separates the two batteries");
      pose(.465f);
      require(length(cycle.loaderHead() - PlasmaShotgun::LOADER_DOCK) < .0001f &&
                  cycle.loaderDoors() < .1f && !cycle.freshVisible(0),
              "Loader does not stow spent batteries before returning with fresh ones");
      pose(.60f);
      require(cycle.freshVisible(0) && cycle.freshVisible(1) && !cycle.spentVisible(0) &&
                  cycle.insertion() > 0 && cycle.insertion() < 1,
              "Charged batteries are not inserted together on the second trip");
      pose(.685f);
      require(cycle.insertion() == 1 && cycle.loaderGrip() == 0 &&
                  length(cycle.batteryPoint(0, true) - cycle.chamberPoint(0)) < .0001f &&
                  cycle.breakAngle() > .9f,
              "Barrels close before both batteries loaded");
      Vec3 previous = PlasmaShotgun::LOADER_DOCK;
      for (int n = 0; n <= 1000; ++n) {
        pose(n / 1000.f);
        Vec3 current = cycle.loaderHead();
        require(length(current - previous) < .15f, "Loader pops into view instead of travelling");
        previous = current;
        bool fresh = cycle.freshVisible(0);
        require(length((cycle.batteryPoint(1, fresh) - cycle.batteryPoint(0, fresh)) -
                       Vec3{1, 0, 0}) < .0001f,
                "Loader loses pair spacing");
      }
      pose(.90f);
      float pull = cycle.lever();
      require(cycle.breakAngle() == 0 && pull > 0, "Final lever does not follow closing");
      pose(.94f);
      require(cycle.lever() > 0, "Lever return is missing");
      require(cycle.elevation() == 1.2f, "Carriage lowers before the charging lever returns");
      pose(.98f);
      require(cycle.lever() == 0 && cycle.elevation() > 0 && cycle.elevation() < 1.2f,
              "Elevator does not return after charging");
      cycle.step(10);
      require(!cycle.reloading && cycle.lever() == 0 && cycle.elevation() == 0 &&
                  cycle.extension() == 0 && cycle.rounds == 2,
              "Reload ends with mechanism displaced");
      cycle.begin(100);
      require(cycle.duration == 10, "Rate upgrades bypass long reload floor");
      cycle.begin(.1f);
      require(cycle.duration == 15, "Reload exceeds 15 second ceiling");
      ManualFeedback fx;
      fx.step(.01f, Gun::Shotgun, true, false, false, false);
      fx.shot(Gun::Shotgun);
      fx.step(.05f, Gun::Shotgun, true, false, false, false);
      require(std::abs(fx.pose.roll) > .1f && -fx.pose.offset.z > .15f,
              "Shotgun lacks huge tilt or backward camera impulse");
      std::cout << "PASS continuous overhead loader, paired extraction/stow/insertion, "
                   "10-15s "
                   "bounds and huge camera tilt\n";
    }
    {
      auto rms = [](Gun gun) {
        ManualSound sound;
        ManualAudioState state;
        state.gun = gun;
        state.active = true;
        sound.set(state);
        state.shots = 1;
        sound.set(state);
        double sum = 0;
        float peak = 0;
        for (int n = 0; n < 24000; ++n) {
          float v = sound.sample();
          sum += v * v;
          peak = std::max(peak, std::abs(v));
        }
        return std::pair{std::sqrt(sum / 24000), peak};
      };
      auto shotgun = rms(Gun::Shotgun), rail = rms(Gun::Rail);
      require(shotgun.first > rail.first * 5 && shotgun.second > .25f && shotgun.second <= .48f,
              "Shotgun report is not substantially louder");
      std::cout << "PASS loud plasma report, RMS ratio=" << shotgun.first / rail.first
                << " peak=" << shotgun.second << '\n';
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
