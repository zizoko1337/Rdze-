#pragma once
#include "renderer.hpp"
#include "gameplay.hpp"

namespace arsenal {
inline Instance instance(Vec3 p, Vec3 scale, Color c, float yaw = 0, float pitch = 0,
                         float alpha = 1) {
  return {p.x, p.y, p.z, scale.x, scale.y, scale.z, yaw, c.r, c.g, c.b, alpha, pitch};
}
inline Instance lineInstance(Vec3 a, Vec3 b, Color color, float width, float alpha = 1) {
  Vec3 middle = (a + b) * .5f, d = b - a;
  return {middle.x, middle.y, middle.z, width, d.x, d.y, d.z, color.r, color.g, color.b, alpha, 0};
}

struct ViewState {
  bool tactical = false, building = true;
  Weapon selected = Laser;
  Vec3 player{0, CORE_HEIGHT + 1.72f, 11}, pan{0, 0, 0};
  float yaw = 0, pitch = -.24f, zoom = 140, orbit = 0;
  bool placement = false;
  float placeX = 0, placeZ = 0;
  int hovered = -1;
  void camera(Camera& c, float aspect) {
    c.farClip = 10000;
    if (tactical) {
      c.eye = {pan.x + std::sin(orbit) * zoom * .7f, zoom, pan.z - std::cos(orbit) * zoom * .7f};
      c.target = pan;
    } else {
      c.eye = player;
      c.target = player + Vec3{std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                               std::cos(yaw) * std::cos(pitch)};
    }
    c.matrix(aspect);
  }
  void move(float side, float forward, float dt, bool fast) {
    float norm = std::max(1.f, std::hypot(side, forward));
    side /= norm;
    forward /= norm;
    if (tactical) {
      float v = zoom * .55f * dt;
      pan.x += (std::cos(orbit) * side - std::sin(orbit) * forward) * v;
      pan.z += (std::sin(orbit) * side + std::cos(orbit) * forward) * v;
      float r = std::hypot(pan.x, pan.z);
      if (r > 500) {
        pan.x *= 500 / r;
        pan.z *= 500 / r;
      }
    } else {
      float v = (fast ? 8.f : 4.8f) * dt;
      player.x += (std::sin(yaw) * forward + std::cos(yaw) * side) * v;
      player.z += (std::cos(yaw) * forward - std::sin(yaw) * side) * v;
      float r = std::hypot(player.x, player.z);
      if (r > CORE_RADIUS - .7f) {
        player.x *= (CORE_RADIUS - .7f) / r;
        player.z *= (CORE_RADIUS - .7f) / r;
      }
      if (r < 2.8f) {
        if (r < .001f)
          player.z = 2.8f;
        else {
          player.x *= 2.8f / r;
          player.z *= 2.8f / r;
        }
      }
    }
  }
  bool pick(const Camera& c, float mouseX, float mouseY, float width, float height) {
    Vec3 forward = normalize(c.target - c.eye);
    float tanHalf = std::tan(c.fov * PI / 360);
    Vec3 ray = normalize(forward + c.right * ((mouseX / width * 2 - 1) * width / height * tanHalf) +
                         c.up * ((1 - mouseY / height * 2) * tanHalf));
    if (ray.y >= -.0001f) {
      placement = false;
      return false;
    }
    Vec3 p = c.eye + ray * (-c.eye.y / ray.y);
    placeX = std::round(p.x / 2) * 2;
    placeZ = std::round(p.z / 2) * 2;
    placement = std::hypot(placeX, placeZ) < WORLD_RADIUS;
    return placement;
  }
};

class GameScene {
public:
  struct OpaquePart {
    Mesh* mesh;
    Instance data;
    bool plain = false;
  };
  std::vector<OpaquePart> extraOpaque;
  std::vector<std::pair<ID3D11ShaderResourceView*, const std::vector<Instance>*>> extraSprites;
  std::array<Com<ID3D11ShaderResourceView>, SpeciesCount> textures;
  std::array<float, SpeciesCount> aspect{1, 1, 1.5f, 1, 1.5f, 1, 1, 1, 1, 1};
  std::array<std::vector<Instance>, SpeciesCount> enemies;
  std::vector<Instance> boxes, tubes, glowBoxes, glowTubes, glowRocks, beams, hot, smoke, ringData,
      shadows, rocks, crystals, worldLetters, bossHealth, gunLetters;
  explicit GameScene(Renderer& r, const std::filesystem::path& assets) {
    extraOpaque.reserve(8);
    r.makeArsenalMeshes();
    r.groundRadius = WORLD_RADIUS;
    r.fogDensity = .0014f;
    for (int i = 0; i < SpeciesCount; ++i) {
      r.loadTexture(assets / SPECIES[i].texture, textures[i]);
      enemies[i].reserve(6000);
    }
    for (auto* v : {&boxes, &tubes, &glowBoxes, &glowTubes, &glowRocks})
      v->reserve(16000);
    beams.reserve(15000);
    hot.reserve(22000);
    smoke.reserve(22000);
    ringData.reserve(2000);
    shadows.reserve(MAX_FOES + MAX_TOWERS);
    worldLetters.reserve(1024);
    bossHealth.reserve(128);
    rocks.reserve(3500);
    crystals.reserve(300);
    Random random;
    for (int i = 0; i < 3000; ++i) {
      float a = random.unit() * 2 * PI;
      float rad = 270 + std::pow(random.unit(), 1.8f) * 2700;
      float s = i < 140 ? 30 + random.unit() * 120 : 1 + random.unit() * 12;
      if (i < 140)
        rad = 1000 + random.unit() * 1900;
      rocks.push_back(instance({std::sin(a) * rad, s * .18f, std::cos(a) * rad},
                               {s * (1 + random.unit()), s * (.6f + random.unit()), s},
                               {.18f, .24f, .27f}, random.unit() * 2 * PI,
                               (random.unit() - .5f) * .3f));
    }
    for (int i = 0; i < 140; ++i) {
      float a = random.unit() * 2 * PI, rad = 280 + random.unit() * 900;
      crystals.push_back(instance({std::sin(a) * rad, 1, std::cos(a) * rad},
                                  {1 + random.unit(), 4 + random.unit() * 10, 1 + random.unit()},
                                  {.08f, .55f, .42f}, a, .2f));
    }
  }
  void box(Vec3 p, Vec3 scale, Color c, float yaw = 0, float pitch = 0, bool glow = false) {
    (glow ? glowBoxes : boxes).push_back(instance(p, scale, c, yaw, pitch));
  }
  void tube(Vec3 p, Vec3 scale, Color c, float yaw = 0, float pitch = 0, bool glow = false) {
    (glow ? glowTubes : tubes).push_back(instance(p, scale, c, yaw, pitch));
  }
  void rod(Vec3 a, Vec3 b, float radius, Color c, bool glow = false) {
    Vec3 d = b - a;
    float len = length(d);
    if (len < .001f)
      return;
    tube((a + b) * .5f, {radius, len, radius}, c, std::atan2(d.x, d.z),
         std::acos(std::clamp(d.y / len, -1.f, 1.f)), glow);
  }
  void makeTower(const Tower& t, float time, bool preview = false, bool valid = true) {
    const auto& w = WEAPONS[t.type];
    float h = w.height;
    Color steel{.60f, .69f, .73f}, dark{.16f, .22f, .25f}, accent = w.color;
    if (t.disabledUntil > time) {
      steel = {.28f, .24f, .33f};
      accent = {.14f, .06f, .2f};
    }
    if (preview) {
      steel = dark = accent = valid ? Color{.15f, .8f, .55f} : Color{.9f, .14f, .18f};
    }
    Vec3 base{t.x, 0, t.z};
    auto world = [&](Vec3 p) {
      return Vec3{t.x + p.x * std::cos(t.angle) + p.z * std::sin(t.angle), p.y,
                  t.z - p.x * std::sin(t.angle) + p.z * std::cos(t.angle)};
    };
    auto b = [&](Vec3 p, Vec3 size, Color c, float angle = 0, float pitch = 0, bool glow = false) {
      box(world(p), size, c, t.angle + angle, pitch, glow);
    };
    auto cy = [&](Vec3 p, Vec3 size, Color c, float pitch = 0, bool glow = false) {
      tube(world(p), size, c, t.angle, pitch, glow);
    };
    tube(base + Vec3{0, .24f, 0}, {1.7f, .48f, 1.7f}, dark);
    box(base + Vec3{0, (h - 1) / 2, 0}, {.7f, h - 1, .7f}, steel);
    tube(base + Vec3{0, h - 1, 0}, {.8f, .3f, .8f}, dark);
    tube(base + Vec3{0, .52f, 0}, {1.4f, .07f, 1.4f}, accent, 0, 0, true);
    if (!preview)
      shadows.push_back(instance(base + Vec3{.8f, .04f, .8f}, {5, 5, 1}, {0, 0, 0}, 0, 0, .4f));
    switch (t.type) {
    case RateRelay:
    case RangeRelay:
    case FrostTower: {
      cy({0, h - .4f, 0}, {1.3f, .5f, 1.3f}, dark);
      float pulse = .8f + .2f * std::cos(t.phase * 2 * PI / 3);
      for (int k = 0; k < 3; ++k)
        cy({0, h - 1.5f + k * .45f, 0}, {1.0f + k * .15f, .12f, 1.0f + k * .15f}, accent, 0, true);
      if (t.type == FrostTower) {
        glowRocks.push_back(
            instance(world({0, h + .5f, 0}), {.65f, 1.5f * pulse, .65f}, accent, t.spin, .2f));
        for (int k = 0; k < 6; ++k) {
          float a = t.spin + k * PI / 3;
          b({std::sin(a) * 1.15f, h + .1f, std::cos(a) * 1.15f}, {.25f, .18f, 1.5f}, steel, a,
            .25f);
        }
      } else {
        int arms = t.type == RateRelay ? 6 : 3;
        for (int k = 0; k < arms; ++k) {
          float a = t.spin * .3f + k * 2 * PI / arms;
          Vec3 end{std::sin(a) * 1.7f, h + .4f + std::sin(t.spin) * .2f, std::cos(a) * 1.7f};
          rod(world({0, h, 0}), world(end), .12f, steel);
          glowRocks.push_back(instance(world(end), {.3f * pulse, .5f, .3f * pulse}, accent));
        }
        b({0, h + .45f, 0}, {1.2f, .16f, .35f}, accent, -t.spin, 0, true);
      }
      break;
    }
    case Laser:
      b({0, h, 0}, {2.2f, 1.15f, 2.6f}, steel);
      b({0, h + .64f, 0}, {2.3f, .15f, 2.7f}, dark);
      cy({0, h, 1.7f - t.recoil * .15f}, {.22f, 1.4f, .22f}, dark, PI / 2);
      cy({0, h, 2.45f - t.recoil * .15f}, {.3f, .16f, .3f}, accent, PI / 2, true);
      for (int k = 0; k < 3; ++k)
        b({-.75f + k * .75f, h - .3f, -1.35f}, {.3f, .25f, .12f}, accent, 0, 0, true);
      cy({0, h + .8f, -.6f}, {.5f, .2f, .5f}, dark);
      b({0, h + .92f, -.6f}, {.85f, .1f, .16f}, accent, t.spin, 0, true);
      break;
    case Minigun:
      b({0, h, -.6f}, {1.9f, 1.55f, 1.8f}, steel);
      b({-1.2f, h - .2f, -.7f}, {.7f, 1.3f, 1.3f}, dark);
      b({1.14f, h - .2f + std::sin(t.spin) * .12f, -.5f}, {.4f, .5f, 1}, accent);
      for (int k = 0; k < 6; ++k) {
        float a = t.spin + k * 2 * PI / 6;
        float x = std::sin(a) * .5f, y = std::cos(a) * .5f;
        cy({x, h + y, 1.5f}, {.13f, 3, .13f}, steel, PI / 2);
        cy({x, h + y, 3.03f}, {.14f, .12f, .14f}, t.pulse > .05f ? accent : dark, PI / 2,
           t.pulse > .05f);
      }
      cy({0, h, .25f}, {.68f, .25f, .68f}, dark, PI / 2);
      cy({0, h, 2.1f}, {.62f, .17f, .62f}, dark, PI / 2);
      break;
    case Tesla:
      cy({0, h - 2, 0}, {.6f, 3.8f, .6f}, dark);
      for (int k = 0; k < 8; ++k)
        cy({0, h - 3 + k * .4f, 0}, {1.05f, .13f, 1.05f}, k % 2 ? steel : accent, 0, k % 2 == 0);
      glowRocks.push_back(instance(world({0, h + .3f, 0}), {.75f, .9f, .75f}, accent));
      for (int k = 0; k < 3; ++k) {
        float a = t.spin * .3f + k * 2 * PI / 3;
        Vec3 end = base + Vec3{std::sin(a) * (1.5f + t.pulse * .6f), h + .5f + t.pulse * .5f,
                               std::cos(a) * (1.5f + t.pulse * .6f)};
        rod(base + Vec3{0, h - 1, 0}, end, .14f, steel);
        glowRocks.push_back(instance(end, {.28f, .4f, .28f}, accent));
      }
      break;
    case Sniper:
      b({0, h, -.5f}, {1.4f, 1.05f, 2.8f}, steel);
      b({0, h, 2.3f - t.recoil * .9f}, {.5f, .5f, 5.4f}, dark);
      for (float sign : {-1.f, 1.f})
        b({sign * .4f, h + .08f, 2.25f - t.recoil * .9f}, {.12f, .2f, 4.8f}, steel);
      b({0, h + .8f, -.4f}, {.45f, .4f, 1.2f}, dark);
      b({0, h + .8f, .23f}, {.24f, .22f, .04f}, accent, 0, 0, true);
      b({0, h + .1f, 5 - t.recoil * .9f}, {.36f, .14f, .12f}, accent, 0, 0, true);
      b({.9f, h - .1f, -1.1f + t.recoil * .8f}, {.3f, .4f, .8f}, accent);
      break;
    case Lance: {
      float charge = t.state == 0 ? t.phase / 6 : t.state == 1 ? 1 : 0;
      float spread = .85f + charge * .65f;
      b({0, h, -.5f}, {2.1f, 1.8f, 2.8f}, dark);
      cy({0, h, 1}, {.48f, 3.5f, .48f}, steel, PI / 2);
      for (int k = 0; k < 4; ++k) {
        float a = t.spin * .35f + k * PI / 2;
        float x = std::sin(a) * spread, y = std::cos(a) * spread;
        b({x, h + y, 1.2f}, {.4f, .4f, 3.5f}, steel);
        b({x, h + y, 1.2f}, {.16f, .46f, 2.8f}, accent, 0, 0, true);
      }
      cy({0, h, 2.8f}, {.25f + charge * .4f, .2f, .25f + charge * .4f}, accent, PI / 2, true);
      break;
    }
    case Venom:
      b({0, h, 0}, {1.8f, .9f, 2.1f}, steel);
      for (float side : {-1.f, 1.f}) {
        cy({side * 1.15f, h - .2f, -.8f}, {.45f, 2, .45f}, dark);
        cy({side * 1.15f, h - .2f, -.8f}, {.37f, 1.6f, .37f}, accent, 0, true);
        cy({side * 1.15f, h + .9f + std::sin(t.spin) * .12f, -.8f}, {.18f, .4f, .18f}, steel);
      }
      cy({0, h, 1.55f}, {.3f, 2, .3f}, dark, PI / 2);
      cy({0, h, 2.55f}, {.35f, .25f, .35f}, accent, PI / 2, true);
      break;
    case Rocket:
      b({0, h - .7f, 0}, {3.3f, .35f, 1.6f}, dark);
      for (float side : {-1.f, 1.f}) {
        b({side * 1.05f, h, .1f}, {1.6f, 1.7f, 2.8f}, steel);
        b({side * 1.05f, h + .95f, .1f}, {1.7f, .16f, 2.9f}, dark, 0, -t.pulse * .4f);
        for (int k = 0; k < 4; ++k) {
          float x = side * 1.05f + (k % 2 - .5f) * .6f, y = h + (k / 2 - .5f) * .6f;
          cy({x, y, 1.57f - t.recoil * .3f}, {.22f, .3f, .22f}, accent, PI / 2, true);
        }
      }
      break;
    case Mortar: {
      float angle = .65f - t.recoil * .18f;
      cy({0, h - 1.6f, 0}, {1.4f, .45f, 1.4f}, dark);
      b({0, h - 1, 0}, {2.7f, .6f, 1.4f}, steel);
      Vec3 low = world({0, h - 1.2f, -.4f}),
           top = world({0, h - 1.2f + std::cos(angle) * 3.4f, std::sin(angle) * 3.4f - .4f});
      rod(low, top, .65f, steel);
      rod(top, top + normalize(top - low) * .18f, .7f, dark);
      for (float side : {-1.f, 1.f})
        cy({side * .95f, h - 1.8f + t.recoil * .35f, -.7f}, {.2f, 1.5f, .2f}, accent);
      b({1.3f, h - 1, 0}, {.16f, 1, .16f}, dark, t.spin);
      break;
    }
    case Flame:
      b({0, h, 0}, {1.6f, 1.05f, 2.4f}, steel);
      for (float side : {-1.f, 1.f}) {
        cy({side * 1.1f, h - .3f, -1}, {.45f, 2, .45f}, dark);
        cy({side * 1.1f, h - .3f, -1}, {.46f, .5f, .46f}, accent);
        b({side * .7f, h - .3f + std::sin(t.spin) * .18f, 0}, {.18f, .4f, 1.4f}, accent);
      }
      cy({0, h, 1.5f}, {.4f, 2.5f, .4f}, dark, PI / 2);
      cy({0, h, 2.7f}, {.48f, .3f, .48f}, accent, PI / 2, true);
      for (int k = 0; k < 4; ++k) {
        float a = t.spin + k * PI / 2;
        b({std::sin(a) * .48f, h + std::cos(a) * .48f, 1.3f}, {.12f, .12f, 1.4f}, steel);
      }
      break;
    default:
      break;
    }
  }
  void core(float time, bool commandDeck = false) {
    const Color steel{.48f, .59f, .65f}, dark{.12f, .18f, .21f}, mint{.2f, .86f, .65f};
    tube({0, CORE_HEIGHT / 2, 0}, {CORE_RADIUS, CORE_HEIGHT, CORE_RADIUS}, steel);
    tube({0, .5f, 0}, {CORE_RADIUS + 1, .8f, CORE_RADIUS + 1}, dark);
    tube({0, CORE_HEIGHT, 0}, {CORE_RADIUS + .3f, .35f, CORE_RADIUS + .3f}, dark);
    for (float h : {1.4f, 9.f, 17.6f})
      tube({0, h, 0}, {CORE_RADIUS + .08f, .09f, CORE_RADIUS + .08f}, mint, 0, 0, true);
    for (int i = 0; i < 24; ++i) {
      float a = i * 2 * PI / 24, x = std::sin(a), z = std::cos(a);
      box({x * 14, 9, z * 14}, {.6f, 15, .6f}, dark, a);
      box({x * 14.35f, 9, z * 14.35f}, {.15f, 5, .12f}, mint, a, 0, true);
      box({x * 13.2f, 18.25f, z * 13.2f}, {.35f, .12f, 1}, mint, a, 0, true);
    }
    if (!commandDeck) {
      tube({0, 18.5f, 0}, {2.6f, .7f, 2.6f}, steel);
      tube({0, 20.7f, 0}, {1.35f, 4, 1.35f}, mint, 0, 0, true);
      tube({0, 22.9f, 0}, {2, .4f, 2}, dark);
      for (int i = 0; i < 4; ++i) {
        float a = i * PI / 2;
        box({std::sin(a) * 1.75f, 20.7f, std::cos(a) * 1.75f}, {.25f, 4.5f, .25f}, dark);
      }
    }
    ringData.push_back(instance({0, 18.23f, 0}, {23, 23, 1}, mint, 0, 0, .45f));
    ringData.push_back(instance({0, .12f, 0}, {36, 36, 1}, mint, 0, 0, .25f));
    shadows.push_back(instance({3, .03f, 3}, {40, 40, 1}, {0, 0, 0}, 0, 0, .5f));
    if (!commandDeck)
      glowRocks.push_back(instance({0, 23.6f, 0}, {.3f, .6f, .3f}, {.4f, .9f, .7f}, time * .3f));
  }
  void prepare(const Game& g, const ViewState& view, bool commandDeck = false) {
    extraOpaque.clear();
    extraSprites.clear();
    for (auto& v : enemies)
      v.clear();
    for (auto* v : {&boxes, &tubes, &glowBoxes, &glowTubes, &glowRocks, &beams, &hot, &smoke,
                    &ringData, &shadows, &worldLetters, &bossHealth, &gunLetters})
      v->clear();
    core(g.time, commandDeck);
    for (const auto& t : g.towers) {
      makeTower(t, g.time);
      if (t.disabledUntil > g.time) {
        ringData.push_back(instance({t.x, .2f, t.z}, {8, 8, 1}, {.8f, .25f, 1}, 0, 0, .8f));
        smoke.push_back(
            instance({t.x, WEAPONS[t.type].height, t.z}, {4, 5, 1}, {.5f, .1f, .8f}, 0, 0, .35f));
      }
      if (t.amplifiedUntil > g.time) {
        float h = WEAPONS[t.type].height;
        for (int k = 0; k < 4; ++k) {
          float a = g.time * 8 + k * PI * .5f;
          Vec3 p{t.x + std::sin(a) * 1.2f, h * .4f, t.z + std::cos(a) * 1.2f};
          Vec3 q{t.x - std::sin(a) * .5f, h + .6f, t.z - std::cos(a) * .5f};
          beams.push_back(lineInstance(p, q, {.35f, .65f, 1}, .09f));
        }
      }
    }
    for (const auto& shot : g.enemyShots) {
      Vec3 pos = shot.position(shot.age / shot.duration);
      hot.push_back(instance(pos, {1.5f, 1.5f, 1}, {.4f, 1, .08f}));
      beams.push_back(lineInstance(shot.position(std::max(0.f, (shot.age - .08f) / shot.duration)),
                                   pos, {.3f, .9f, .08f}, .3f));
    }
    for (auto link : g.supportLinks) {
      const auto& source = g.towers[link.source];
      const auto& target = g.towers[link.target];
      Vec3 a{source.x, WEAPONS[source.type].height + .5f, source.z};
      Vec3 b{target.x, WEAPONS[target.type].height, target.z};
      Color color = WEAPONS[source.type].color;
      beams.push_back(lineInstance(a, b, color, .05f + source.pulse * .07f));
      float t = std::fmod(g.time / 3, 1.f);
      hot.push_back(instance(a + (b - a) * t, {.5f, .5f, 1}, color, 0, 0, .8f));
    }
    if (view.tactical && view.building && view.placement) {
      bool valid = !g.canBuild(view.placeX, view.placeZ);
      Tower ghost;
      ghost.type = view.selected;
      ghost.x = view.placeX;
      ghost.z = view.placeZ;
      ghost.angle = std::atan2(ghost.x, ghost.z);
      ghost.spin = g.time;
      makeTower(ghost, g.time, true, valid);
      Color color = valid ? Color{.3f, .95f, .65f} : Color{1, .2f, .25f};
      float range = WEAPONS[view.selected].range;
      ringData.push_back(
          instance({view.placeX, .12f, view.placeZ}, {range * 2, range * 2, 1}, color, 0, 0, .65f));
    } else if (view.tactical && view.hovered >= 0 && view.hovered < int(g.towers.size())) {
      auto& t = g.towers[view.hovered];
      float range = WEAPONS[t.type].range;
      ringData.push_back(
          instance({t.x, .12f, t.z}, {range * 2, range * 2, 1}, WEAPONS[t.type].color, 0, 0, .5f));
    }
    if (view.tactical)
      ringData.push_back(instance({0, .1f, 0}, {BUILD_RADIUS * 2, BUILD_RADIUS * 2, 1},
                                  {.2f, .65f, .5f}, 0, 0, .35f));
    for (int i : g.active) {
      auto& e = g.foes[i];
      if (e.hp <= 0)
        continue;
      float bob =
          (e.stun > 0 ? 0 : std::abs(std::sin(g.time * (e.type >= Titan ? 2 : 8) + e.phase))) *
          (e.type >= Titan ? .3f : .15f);
      float height = e.size, width = e.size * aspect[e.type];
      Color color{1, 1, 1};
      if (e.elite)
        color = {1.45f, .9f, .5f};
      if (e.poison > 0)
        color = {.7f, 1, .6f};
      if (e.stun > 0)
        color = {.45f, .7f, 1.5f};
      if (e.flash > 0)
        color = {1.5f, 1.3f, 1.3f};
      float altitude = e.altitude + (e.type == Floater ? std::sin(g.time * 2 + e.phase) * .6f : 0);
      enemies[e.type].push_back(
          instance({e.x, height * .48f + altitude + bob, e.z},
                   {width * (1 + std::sin(g.time * 5 + e.phase) * .015f), height, 1}, color));
      if (e.type >= Titan)
        bossHealth.push_back(instance({e.x, height + altitude + bob + 1.5f, e.z},
                                      {height * .85f, .75f, 1}, {1, .18f, .12f}, 0, 0,
                                      std::clamp(e.hp / e.maxHp, 0.f, 1.f)));
      if (e.elite)
        ringData.push_back(instance({e.x, .16f, e.z}, {e.size * 1.3f, e.size * 1.3f, 1},
                                    {1, .6f, .15f}, 0, 0, .65f));
      if (e.type == Saboteur && e.sabotage > 0) {
        hot.push_back(instance({e.x, 1.5f, e.z}, {2, 3, 1}, {1, .4f, .05f}, 0, 0,
                               .25f + e.sabotage / SABOTAGE_TIME * .5f));
        auto t = std::find_if(g.towers.begin(), g.towers.end(),
                              [&](const Tower& tower) { return tower.id == e.towerTarget; });
        if (t != g.towers.end())
          beams.push_back(lineInstance({e.x, 2, e.z}, {t->x, 2, t->z}, {1, .3f, .04f}, .2f));
      }
      if (e.type == Brain) {
        ringData.push_back(instance({e.x, .18f, e.z}, {BRAIN_RADIUS * 2, BRAIN_RADIUS * 2, 1},
                                    {.65f, .2f, 1}, 0, 0, .15f + .12f * std::sin(g.time * 3)));
      }
      if (e.stun > 0)
        ringData.push_back(
            instance({e.x, .18f, e.z}, {e.size, e.size, 1}, {.3f, .65f, 1}, 0, 0, .8f));
      if (e.glued > 0)
        shadows.push_back(
            instance({e.x, .12f, e.z}, {e.size * 1.5f, e.size, 1}, {.08f, .2f, .025f}, 0, 0, .7f));
      shadows.push_back(
          instance({e.x, .06f, e.z}, {width * .9f, height * .55f, 1}, {0, 0, 0}, 0, 0, .45f));
      if (e.burn > 0) {
        hot.push_back(instance({e.x, altitude + 1.2f, e.z}, {height * .6f, height * .7f, 1},
                               {1, .25f, .04f}, 0, 0, .2f));
      }
    }
    for (auto& b : g.beams)
      beams.push_back(
          lineInstance(b.a, b.b, b.color, b.width, std::min(1.f, b.life / b.maxLife * 2)));
    for (auto& p : g.shots) {
      Color color = WEAPONS[p.type].color;
      Vec3 direction = normalize(p.pos - p.previous);
      beams.push_back(lineInstance(p.pos - direction * (p.type == Rocket ? 1.4f : .7f), p.pos,
                                   color, p.type == Mortar ? .45f : .25f));
      glowRocks.push_back(instance(p.pos, {.25f, .25f, .25f}, color));
    }
    for (auto& p : g.particles) {
      float age = 1 - p.life / p.maxLife;
      bool snowflake = p.style == 2;
      float size = p.size * (snowflake ? 1.f : 1 + age * (p.style ? 3 : 2));
      (p.style ? smoke : hot)
          .push_back(instance(p.pos, {size, size, 1}, p.color, 0, 0,
                              (1 - age) * (snowflake ? .65f
                                           : p.style ? .28f
                                                     : .75f)));
    }
    for (auto& z : g.gas) {
      for (int i = 0; i < 5; ++i) {
        float a = i * 2 * PI / 5 + g.time * .2f;
        float radius = z.radius * .45f;
        Vec3 pos{z.x + std::sin(a) * radius, 1.8f + std::sin(a * 2) * .5f,
                 z.z + std::cos(a) * radius};
        smoke.push_back(instance(pos, {z.radius, z.radius * .6f, 1}, {.25f, .65f, .08f}, 0, 0,
                                 std::min(1.f, z.life) * .3f));
      }
    }
    for (auto& r : g.rings) {
      float age = 1 - r.life / r.maxLife;
      float rad = r.radius * (.2f + .8f * age);
      ringData.push_back(instance(r.pos, {rad * 2, rad * 2, 1}, r.color, 0, 0, (1 - age) * .8f));
    }
  }
  void render(Renderer& r) {
    Instance sky = instance({0, 0, 0}, {1, 1, 1}, {1, 1, 1});
    r.draw(r.quad, &sky, 1, r.white, 11);
    Instance floor =
        instance({0, -.05f, 0}, {WORLD_RADIUS * 2, 1, WORLD_RADIUS * 2}, {.15f, .2f, .225f});
    r.draw(r.ground, &floor, 1, r.white, 3);
    r.draw(r.quad, shadows, r.white, 12);
    r.draw(r.rock, rocks, r.white, 0);
    r.draw(r.rock, crystals, r.white, 6);
    r.draw(r.cube, boxes, r.metal, 0);
    r.draw(r.tube, tubes, r.metal, 0);
    for (auto& part : extraOpaque)
      r.draw(*part.mesh, &part.data, 1, part.plain ? r.white : r.metal, 0);
    r.draw(r.cube, glowBoxes, r.white, 6);
    r.draw(r.tube, glowTubes, r.white, 6);
    r.draw(r.rock, glowRocks, r.white, 6);
    for (int i = 0; i < SpeciesCount; ++i)
      r.draw(r.quad, enemies[i], textures[i], 1);
    r.draw(r.quad, bossHealth, r.white, 17);
    for (const auto& batch : extraSprites)
      r.draw(r.quad, *batch.second, batch.first, 1);
    r.draw(r.quad, ringData, r.white, 10);
    r.draw(r.quad, smoke, r.white, 9);
    r.draw(r.quad, beams, r.white, 7);
    r.draw(r.quad, hot, r.white, 8);
    r.draw(r.quad, worldLetters, r.font, 15);
    r.draw(r.quad, gunLetters, r.font, 18);
  }
};
} // namespace arsenal
