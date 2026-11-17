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
void bury(Game& g, int id) {
  g.foes[id].altitude = -40;
  for (int n = 0; n < TunnelerBody::SEGMENTS; ++n)
    g.tunneler(id)->segments[n] = {200 + n * 10.f, -30, 240};
}
int main() {
  try {
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      int id = g.spawn(Tunneler, 0, 240);
      auto& body = *g.tunneler(id);
      int direction = body.direction;
      bool surfaced = false, buried = false;
      float previous = 0, sideways = 0;
      for (int n = 0; n < 240; ++n) {
        g.step(1.f / 60);
        float angle = std::atan2(g.foes[id].x, g.foes[id].z);
        float delta = std::atan2(std::sin(angle - previous), std::cos(angle - previous));
        require(delta * direction > 0, "Tunneler does not keep its chosen lateral direction");
        sideways += std::abs(delta) * std::hypot(g.foes[id].x, g.foes[id].z);
        previous = angle;
        surfaced |= g.headPoint(id).y > 10;
        buried |= !g.partExposed(id, -1);
      }
      float inward = 240 - std::hypot(g.foes[id].x, g.foes[id].z);
      require(sideways > inward * 12 && std::abs(inward - g.foes[id].speed * 4) < .03f,
              "Lateral orbit changes slow inward progression");
      require(surfaced && buried && body.SEGMENTS == 32 && body.GAP * body.SEGMENTS >= 220,
              "Missing repeated dives or long body");
      int above = 0, below = 0;
      for (int part = 0; part < body.SEGMENTS; ++part)
        (g.partExposed(id, part) ? above : below)++;
      require(above > 5 && below > 5,
              "Entire body dives at once instead of forming sinusoidal arcs");
      float hp = g.foes[id].maxHp;
      for (int turn = 1; turn <= 3; ++turn) {
        g.hurt(id, hp * .3f, Venom);
        require(body.reversals == turn && body.direction == (turn % 2 ? -direction : direction),
                "Missing direction reversal at 70/40/10 percent HP");
      }
      g.hurt(id, 1, Venom);
      require(body.reversals == 3, "Damage repeats an already consumed reversal");
      auto stopped = body.segments;
      Vec3 head = g.headPoint(id);
      g.foes[id].stun = 2;
      tick(g, 1);
      require(length(g.headPoint(id) - head) < .001f, "Stunned tunneler head moves");
      for (int part = 0; part < body.SEGMENTS; ++part)
        require(length(body.segments[part] - stopped[part]) < .001f, "Stunned body is dragged");
      std::cout << "PASS fast one-direction orbit, slow inward motion, 32 diving segments and "
                   "three HP reversals\n";
    }
    {
      auto game = std::make_unique<Game>(false);
      auto& g = *game;
      g.autoWaves = false;
      int id = g.spawn(Tunneler, 0, 240);
      bury(g, id);
      auto& body = *g.tunneler(id);
      body.segments[0] = {80, 10, 120};
      body.segments[1] = {83, 10, 120};
      g.grid.rebuild(g.foes, g.active);
      auto hit = trace(g, {80, 50, 120}, {0, -1, 0}, 60);
      require(hit.id == id && hit.part >= 0, "Exposed body cannot be hit while head is buried");
      require(g.hurt(id, 100, Laser, true, DamageClass::Direct, hit.part) == 100,
              "Visible segment does not share boss HP");
      require(trace(g, {0, 50, 240}, {0, -1, 0}, 100).id < 0, "Ray locks onto buried head");
      require(trace(g, {220, 50, 240}, {0, -1, 0}, 100).id < 0,
              "Ray hits underground segment through terrain");
      for (auto source : {Laser, Minigun, Tesla, Sniper, Lance, Rocket, Mortar, Flame})
        require(g.hurt(id, 100, source, true, DamageClass::Direct, -1) == 0,
                "Non-poison damage reaches a buried part");
      float hp = g.foes[id].hp;
      g.blast({220, 0, 240}, 4, 100, Mortar);
      require(g.foes[id].hp == hp,
              "Splash damages buried body just because another part is exposed");
      g.blast({80, 10, 120}, 10, 100, Mortar);
      require(g.foes[id].hp == hp - 100, "Explosion damages shared health more than once");
      Tower tower;
      tower.type = Laser;
      tower.x = 80;
      tower.z = 100;
      require(g.visibleTarget(tower, 30) == id && g.priorityTarget(tower, 30) == id,
              "Tower cannot acquire an exposed body beyond the head's grid cell");
      tower.target = id;
      tower.generation = g.foes[id].generation;
      require(g.inRange(tower), "Tower loses exposed segment when head is out of range");
      body.burnPart = 2;
      require(g.hurt(id, 100, Flame, true, DamageClass::Persistent) == 0,
              "Burn keeps damaging a part after it goes underground");
      bury(g, id);
      require(!g.targetable(id) && g.visibleTarget(tower, 650) < 0,
              "Fully buried body remains targetable");
      require(g.hurt(id, 100, Laser, false, DamageClass::Service) == 0 &&
                  g.hurt(id, 100, Laser, false, DamageClass::Collision) == 0,
              "Services or collisions bypass underground immunity");
      int poisonContacts = 0;
      g.targetsNear(220, 240, 4, [&](int) { ++poisonContacts; }, true);
      require(poisonContacts == 1 && g.hurt(id, 100, Venom, true, DamageClass::Persistent) == 100,
              "Poison cannot reach buried segment");
      auto generation = g.foes[id].generation;
      g.hurt(id, 100000, Venom);
      tick(g, .02f);
      require(g.tunnelers.empty() && g.bossKills == 1, "Dead tunneler leaves ghost segments");
      int reused = g.spawn(Crawler, 0, 240);
      require(reused == id && g.foes[id].generation != generation && !g.tunneler(id),
              "Old body survives reused enemy ID");
      std::cout << "PASS per-part ray/splash immunity, shared HP, body targeting, poison, burn and "
                   "cleanup\n";
    }
    {
      auto session = std::make_unique<Session>();
      auto& s = *session;
      s.start(Gun::Rail);
      s.mode = Mode::Mounted;
      s.lift = s.hatch = 1;
      s.world.autoWaves = false;
      for (int id : s.world.active)
        s.world.foes[id].hp = 0;
      s.world.step(.01f);
      int id = s.world.spawn(Tunneler, 0, 240);
      bury(s.world, id);
      s.world.tunneler(id)->segments[0] = {0, 10, 80};
      s.world.tunneler(id)->segments[1] = {0, 10, 85};
      s.world.grid.rebuild(s.world.foes, s.world.active);
      for (int n = 0; n < 5; ++n) {
        auto d = normalize(Vec3{0, 10, 80} - s.eye());
        s.yaw = s.gunYaw = std::atan2(d.x, d.z);
        s.pitch = s.gunPitch = std::asin(d.y);
      }
      float hp = s.world.foes[id].hp;
      s.fireRail();
      require(s.world.foes[id].hp < hp && s.world.hits[Sniper] == 1,
              "Piercing sniper misses body or damages each intersected segment separately");
      std::cout << "PASS actual manual sniper hits exposed body once with buried head\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
