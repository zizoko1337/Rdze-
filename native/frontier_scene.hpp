#pragma once
#include "command_game.hpp"
#include "game_scene.hpp"

namespace command {
inline uint64_t pickCrystal(const Session& s, const Mat& matrix, float width, float height,
                            float px, float py, float scale) {
  uint64_t found = 0;
  float best = 25 * 25 * scale * scale;
  const auto& m = matrix.m;
  for (const auto& c : s.frontier.crystals) {
    if (c.state != CrystalState::Ground)
      continue;
    Vec3 p = c.position + Vec3{0, 1.8f, 0};
    float w = p.x * m[3] + p.y * m[7] + p.z * m[11] + m[15];
    if (w <= 0)
      continue;
    float x = p.x * m[0] + p.y * m[4] + p.z * m[8] + m[12];
    float y = p.x * m[1] + p.y * m[5] + p.z * m[9] + m[13];
    float dx = (x / w + 1) * width * .5f - px, dy = (1 - y / w) * height * .5f - py;
    float distance = dx * dx + dy * dy;
    if (distance < best) {
      best = distance;
      found = c.id;
    }
  }
  return found;
}
class FrontierScene {
  Com<ID3D11ShaderResourceView> roverTexture;
  std::vector<Instance> roverSprites;

public:
  explicit FrontierScene(Renderer& r, const std::filesystem::path& assets) {
    r.loadTexture(assets / "rover-rear.png", roverTexture);
    roverSprites.reserve(Frontier::MAX_ROVERS);
  }
  void append(GameScene& scene, const Session& s, uint64_t hovered = 0) {
    if (!s.expedition.enabled || s.mode == Mode::Selection)
      return;
    const auto& f = s.frontier;
    roverSprites.clear();
    for (const auto& rover : f.rovers) {
      Vec3 p = rover.position + Vec3{0, .3f + std::sin(s.world.time * 18) * .02f, 0};
      roverSprites.push_back(instance(p, {1.75f, 1.75f, 1}, {1, 1, 1}));
      scene.shadows.push_back(instance({p.x, .08f, p.z}, {1.5f, 1.1f, 1}, {0, 0, 0}, 0, 0, .45f));
    }
    scene.extraSprites.push_back({roverTexture, &roverSprites});
    if (s.expedition.levels[size_t(Upgrade::RoverFactory)] > 0) {
      for (int i = 0; i < 4; ++i) {
        float a = i * PI / 2;
        Vec3 p{std::sin(a) * 14.2f, 1.1f, std::cos(a) * 14.2f};
        scene.box(p, {4.2f, 2.1f, .6f}, {.1f, .2f, .23f}, a);
        scene.box(p + Vec3{std::sin(a) * .4f, .8f, std::cos(a) * .4f}, {3.8f, .08f, .08f},
                  {.2f, .9f, .7f}, a, 0, true);
      }
    }
    for (const auto& c : f.crystals) {
      Color color = c.boss ? Color{1, .65f, .2f} : Color{.4f, .6f, 1};
      Vec3 p = c.state == CrystalState::Carried
                   ? f.collectorPosition + Vec3{0, -2.8f, 0}
                   : c.position + Vec3{0, 1.6f + std::sin(s.world.time * 2 + c.id) * .3f, 0};
      float size = c.boss ? 2.2f : 1.6f;
      scene.glowRocks.push_back(
          instance(p, {size * .6f, size * 1.5f, size * .6f}, color, s.world.time * .6f, .2f));
      if (c.state != CrystalState::Carried) {
        scene.ringData.push_back(instance(c.position, {7, 7, 1}, color, 0, 0, .75f));
        scene.beams.push_back(lineInstance(p + Vec3{0, 1, 0}, p + Vec3{0, 8, 0}, color, .07f));
        if (c.id == hovered)
          scene.ringData.push_back(instance(c.position, {10, 10, 1}, {1, 1, 1}, 0, 0, 1));
      }
    }
    const Vec3 dock = Frontier::COLLECTOR_DOCK;
    scene.box(dock + Vec3{0, -3.7f, 0}, {5, .35f, 5}, {.1f, .22f, .25f});
    scene.rod({-12, dock.y - 3.7f, -6}, dock + Vec3{0, -3.7f, 0}, .25f, {.15f, .25f, .28f});
    Vec3 drone = f.collectorPosition;
    scene.box(drone, {2.7f, 1.1f, 3.7f}, {.5f, .63f, .67f});
    scene.box(drone + Vec3{0, .65f, 0}, {1.6f, .25f, 1.8f}, {.15f, .28f, .3f});
    scene.box(drone + Vec3{0, 0, -1.9f}, {1.8f, .25f, .05f}, {.25f, .9f, 1}, 0, 0, true);
    for (float x : {-2.3f, 2.3f})
      for (float z : {-1.9f, 1.9f}) {
        Vec3 rotor = drone + Vec3{x, .2f, z};
        scene.rod(drone, rotor, .13f, {.15f, .23f, .26f});
        scene.tube(rotor, {1.1f, .2f, 1.1f}, {.2f, .3f, .33f});
        scene.box(rotor + Vec3{0, .15f, 0}, {2.1f, .06f, .18f}, {.65f, .75f, .78f},
                  s.world.time * 18);
      }
    if (f.collectorState == CollectorState::Collecting)
      for (const auto& crystal : f.crystals)
        if (crystal.id == f.collectorTarget)
          scene.beams.push_back(lineInstance(drone + Vec3{0, -.6f, 0},
                                             crystal.position + Vec3{0, 1, 0}, {.4f, .8f, 1}, .3f));
  }
};
} // namespace command
