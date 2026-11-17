#include "command_game.hpp"
#include "enemy_spray.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace command;
void require(bool value, const char* message) {
  if (!value)
    throw std::runtime_error(message);
}
void tick(Game& g, float seconds) {
  for (int i = 0; i < int(std::round(seconds * 60)); ++i)
    g.step(1.f / 60);
}
int main() {
  try {
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      g.recordDeaths = true;
      Frontier frontier;
      int root = g.spawn(Slime, 150, 0);
      require(g.foes[root].maxHp == 8000, "Slime root HP");
      float total = 0;
      int killed = 0;
      for (;;) {
        int target = -1;
        for (int id : g.active)
          if (g.foes[id].type == Slime && g.foes[id].hp > 0) {
            target = id;
            break;
          }
        if (target < 0)
          break;
        auto old = g.foes[target];
        total += old.hp;
        g.hurt(target, old.hp, Laser);
        ++killed;
        frontier.processDeaths(g);
        require(frontier.crystals.size() == (killed == 7 ? 1u : 0u),
                "Slime loot before final piece");
      }
      require(total == 24000 && killed == 7 && g.bossKills == 1 && g.slimeFamilies.empty(),
              "Slime family budget / termination");
      require(Frontier::spawnHp(Slime, 3600) == 8000, "Time scaling changes slime budget");
      std::cout << "PASS slime 8000/4000/2000, 24000 total, one final crystal\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      int boss = g.spawn(Devourer, 160, 0);
      g.foes[boss].speed = 0;
      for (int n = 0; n < 8; ++n) {
        float angle = n * PI / 4, radius = 24.f + n * 3;
        int id = g.spawn(Runner, 160 + std::sin(angle) * radius, std::cos(angle) * radius);
        g.foes[id].hp = g.foes[id].maxHp = 300;
      }
      int outside = g.spawn(Crawler, 211, 0), otherBoss = g.spawn(Titan, 180, 0);
      g.foes[outside].speed = g.foes[otherBoss].speed = 0;
      size_t existing = g.active.size();
      g.hurt(boss, 3000, Laser);
      require(g.foes[boss].feedUsed == 1 && g.foes[boss].feedPending == 1, "75% trigger");
      tick(g, .02f);
      int offerings = 0;
      require(g.active.size() == existing, "Devourer must not spawn new enemies");
      require(g.foes[outside].summoner < 0 && g.foes[otherBoss].summoner < 0,
              "Devourer claims outside radius or another boss");
      for (int id : g.active)
        if (g.foes[id].summoner == boss) {
          ++offerings;
          require(std::hypot(g.foes[id].x - 160, g.foes[id].z) <= 50.01f, "Claim outside 50m");
          g.hurt(id, 250, Laser);
        }
      require(offerings == 8, "Offering count");
      tick(g, 8.2f);
      require(std::abs(g.foes[boss].hp - 9400) < .1f, "Heal must use remaining offering HP");
      require(g.kills == 0, "Absorption counted as a kill");
      g.hurt(boss, 401, Laser);
      tick(g, .02f);
      require(g.foes[boss].feedPending == 0 && g.foes[boss].absorbClock == 0,
              "Repeated 75% phase after healing");
      for (int n = 0; n < 2; ++n) {
        int id = g.spawn(Runner, 160, 30 + n * 5.f);
        g.foes[id].hp = 300;
      }
      g.hurt(boss, 3000, Laser);
      tick(g, .02f);
      require(g.foes[boss].feedUsed == 3 && g.foes[boss].absorbClock > 0, "50% phase");
      // Kill the offerings: denied healing and no accidental elite/boss drops.
      for (int id : g.active)
        if (g.foes[id].summoner == boss && g.foes[id].hp > 0)
          g.hurt(id, 10000, Laser);
      float hp = g.foes[boss].hp;
      tick(g, 8.2f);
      require(g.foes[boss].hp == hp, "Dead offerings still heal");
      g.hurt(boss, hp - 2999, Laser);
      tick(g, .02f);
      require(g.foes[boss].feedUsed == 7 && g.foes[boss].absorbClock > 0, "25% phase");
      g.foes[boss].hp = 12000;
      tick(g, 8.2f);
      g.hurt(boss, 9001, Laser);
      tick(g, .02f);
      require(g.foes[boss].feedPending == 0 && g.foes[boss].absorbClock == 0,
              "Used phases rearmed by healing");
      std::cout << "PASS devourer existing foes only, 50m, boss exclusion, one-time thresholds and "
                   "remaining HP\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      int boss = g.spawn(Devourer, 180, 0);
      g.foes[boss].speed = 0;
      g.hurt(boss, 10000, Laser);
      require(g.foes[boss].feedUsed == 7 && g.foes[boss].feedPending == 7,
              "Burst damage skips thresholds");
      tick(g, .02f);
      require(g.foes[boss].feedPending == 6, "Burst phases not queued");
      require(g.active.size() == 1 && g.foes[boss].hp == 2000,
              "Empty feeding area spawns foes or grants healing");
      g.paused = true;
      float clock = g.foes[boss].absorbClock;
      tick(g, 2);
      require(g.foes[boss].absorbClock == clock, "Pause advances feeding");
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      int boss = g.spawn(SiegeBoss, SIEGE_RANGE, 0);
      g.foes[boss].speed = 0;
      tick(g, 5);
      require(std::abs(g.coreDamage - 200) < .01f && g.enemyShots.empty() &&
                  g.foes[boss].siegeSpraying,
              "Continuous boss spray must deal 200 damage in 5 seconds without projectiles");
      tick(g, 5);
      require(std::abs(g.coreDamage - 400) < .02f, "Spray is not continuous 40 DPS");
      g.paused = true;
      float frozen = g.coreDamage;
      tick(g, 2);
      require(g.coreDamage == frozen, "Paused spray deals damage");
      g.paused = false;
      g.foes[boss].stun = 1;
      tick(g, .5f);
      require(g.coreDamage == frozen && !g.foes[boss].siegeSpraying, "Stunned boss keeps spraying");
      g.foes[boss].stun = 0;
      g.foes[boss].x = SIEGE_RANGE + 1;
      g.foes[boss].ability = 0;
      tick(g, .1f);
      require(g.coreDamage == frozen && !g.foes[boss].siegeSpraying,
              "Siege boss sprays outside range");
      g.foes[boss].x = SIEGE_RANGE;
      tick(g, .1f);
      require(g.coreDamage > frozen && g.foes[boss].siegeSpraying,
              "Boss does not resume spray in range");
      g.hurt(boss, 1000000, Laser);
      frozen = g.coreDamage;
      tick(g, .2f);
      require(g.coreDamage == frozen, "Dead boss keeps spraying");
      std::cout
          << "PASS siege boss continuous 40 DPS, 63m, no projectiles, pause, stun and death\n";
    }
    {
      SiegeSpray spray{{63, 11.52f, 0}, 36, 24, 1, .7f};
      for (auto axes : {std::array<Vec3, 2>{{{1, 0, 0}, {0, 1, 0}}},
                        std::array<Vec3, 2>{{{0, 0, -1}, {0, 1, 0}}},
                        std::array<Vec3, 2>{{{1, 0, 0}, {0, .7071068f, -.7071068f}}}}) {
        Vec3 mouth = spray.muzzle(axes[0], axes[1]);
        Vec3 offset = mouth - spray.centre;
        require(std::abs(dot(offset, axes[0]) / spray.width - .42f) < .001f &&
                    std::abs(dot(offset, axes[1]) / spray.height - .30f) < .001f,
                "Spray muzzle not attached to trumpet's sprite socket");
        require(length(spray.point(0, axes[0], axes[1]) - mouth) < .001f,
                "Spray starts at body centre instead of trumpet");
        Vec3 impact = spray.impact(axes[0], axes[1]);
        require(std::abs(std::hypot(impact.x, impact.z) - CORE_RADIUS - .1f) < .001f,
                "Spray does not hit outer core wall");
        require(length(spray.point(1, axes[0], axes[1]) - impact) < .001f,
                "Spray endpoint is disconnected from core");
      }
      std::cout << "PASS trumpet socket in platform, rotated and overhead views\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      int boss = g.spawn(Wyrm, 0, 200);
      g.foes[boss].phase = 0;
      require(g.worms.size() == 1, "Missing worm body");
      bool left = false, right = false;
      float minX = 0, maxX = 0;
      float travelled = 0;
      for (int n = 0; n < 180; ++n) {
        float x = g.foes[boss].x;
        Vec3 before{g.foes[boss].x, 0, g.foes[boss].z};
        tick(g, 1.f / 60);
        left |= g.foes[boss].x < x;
        right |= g.foes[boss].x > x;
        minX = std::min(minX, g.foes[boss].x);
        maxX = std::max(maxX, g.foes[boss].x);
        travelled += length(Vec3{g.foes[boss].x, 0, g.foes[boss].z} - before);
      }
      float advance = 200 - std::hypot(g.foes[boss].x, g.foes[boss].z);
      require(left && right && maxX - minX > 70, "Worm side-to-side sway is too narrow");
      require(travelled > advance * 10, "Sideways motion does not dominate forward motion");
      require(std::abs(advance - g.foes[boss].speed * 3) < .02f && advance < 10,
              "Lateral speed changes normal inward advance");
      Vec3 previous{g.foes[boss].x, 0, g.foes[boss].z};
      for (auto p : g.worms.front().segments) {
        require(length(p - previous) <= 7.01f, "Disconnected worm segment");
        previous = p;
      }
      auto tail = g.worms.front().segments.back();
      g.grid.rebuild(g.foes, g.active);
      auto hit = trace(g, tail + Vec3{0, 30, 0}, {0, -1, 0}, 40);
      require(hit.id == boss, "Worm tail cannot be hit");
      float oldHp = g.foes[boss].hp;
      g.blast(tail, 4, 100, Mortar);
      require(g.foes[boss].hp == oldHp - 100, "Tail blast must damage shared HP once");
      g.foes[boss].glued = 1;
      auto stoppedBody = g.worms.front().segments;
      float x = g.foes[boss].x, z = g.foes[boss].z;
      tick(g, .5f);
      require(g.foes[boss].x == x && g.foes[boss].z == z, "Glue does not stop zigzag");
      for (size_t i = 0; i < stoppedBody.size(); ++i)
        require(length(g.worms.front().segments[i] - stoppedBody[i]) < .001f,
                "Stopped head keeps dragging its body");
      g.hurt(boss, 100000, Laser);
      tick(g, .02f);
      require(g.worms.empty(), "Dead worm body not removed");
      std::cout << "PASS dominant sideways zigzag, Snake body, shared hitboxes, glue and cleanup\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      constexpr Species expected[] = {Titan, Queen, Brain, Wyrm, Slime, Devourer, SiegeBoss, Tunneler};
      for (auto type : expected) {
        int id = g.boss();
        require(id >= 0 && g.foes[id].type == type, "Natural rotation omits a boss");
      }
      std::cout << "PASS all eight bosses in natural rotation\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
