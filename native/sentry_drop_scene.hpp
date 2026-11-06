#pragma once
#include "command_game.hpp"
#include "game_scene.hpp"

namespace command {
class SentryDropScene {
  Mesh balloon;

public:
  explicit SentryDropScene(Renderer& r) {
    std::vector<Vertex> vertices;
    auto vertex = [](int lat, int lon) {
      float a = lat * PI / 16, b = lon * 2 * PI / 32;
      Vec3 n{std::sin(a) * std::cos(b), std::cos(a), std::sin(a) * std::sin(b)};
      return Vertex{n.x, n.y, n.z, n.x, n.y, n.z, lon / 32.f, lat / 16.f};
    };
    for (int lat = 0; lat < 16; ++lat)
      for (int lon = 0; lon < 32; ++lon) {
        auto a = vertex(lat, lon), b = vertex(lat + 1, lon), c = vertex(lat + 1, lon + 1),
             d = vertex(lat, lon + 1);
        for (auto v : {a, b, c, a, c, d})
          vertices.push_back(v);
      }
    r.uploadMesh(balloon, vertices);
  }
  void append(GameScene& scene, const Session& s) {
    for (const auto& drop : s.sentryDrops) {
      const auto& slot = TURRET_SLOTS[drop.slot];
      float h = drop.cargoHeight(), y = drop.balloonHeight();
      if (!drop.installed) {
        std::array<std::vector<Instance>*, 5> groups{&scene.boxes, &scene.tubes, &scene.glowBoxes,
                                                     &scene.glowTubes, &scene.glowRocks};
        std::array<size_t, 5> sizes{};
        for (size_t i = 0; i < groups.size(); ++i)
          sizes[i] = groups[i]->size();
        Tower cargo;
        cargo.type = drop.type;
        cargo.x = slot.x;
        cargo.z = slot.z;
        cargo.angle = std::atan2(slot.x, slot.z);
        scene.makeTower(cargo, s.world.time);
        for (size_t i = 0; i < groups.size(); ++i)
          for (size_t j = sizes[i]; j < groups[i]->size(); ++j)
            (*groups[i])[j].y += h;
      }
      // Fabric balloon, ribbed canopy, four suspension lines and a powered return module.
      scene.extraOpaque.push_back(
          {&balloon, instance({slot.x, y, slot.z}, {6.2f, 8, 6.2f}, {.9f, .76f, .47f}), true});
      for (int rib = 0; rib < 8; ++rib) {
        float a = rib * PI / 4;
        for (int k = 0; k < 10; ++k) {
          float p = .15f + k * (PI - .3f) / 10, q = .15f + (k + 1) * (PI - .3f) / 10;
          Vec3 first{slot.x + 6.24f * std::sin(p) * std::cos(a), y + 8.04f * std::cos(p),
                     slot.z + 6.24f * std::sin(p) * std::sin(a)};
          Vec3 next{slot.x + 6.24f * std::sin(q) * std::cos(a), y + 8.04f * std::cos(q),
                    slot.z + 6.24f * std::sin(q) * std::sin(a)};
          scene.rod(first, next, .045f, {.3f, .22f, .13f});
        }
      }
      scene.box({slot.x, y - 8.4f, slot.z}, {2.6f, .8f, 2.6f}, {.18f, .23f, .26f});
      for (int line = 0; line < 4; ++line) {
        float a = PI / 4 + line * PI / 2;
        Vec3 top{slot.x + std::cos(a) * 2.7f, y - 7.2f, slot.z + std::sin(a) * 2.7f};
        Vec3 end = drop.installed
                       ? Vec3{slot.x + std::cos(a), y - 9.7f, slot.z + std::sin(a)}
                       : Vec3{slot.x + std::cos(a) * 1.1f, h + 4.5f, slot.z + std::sin(a) * 1.1f};
        scene.rod(top, end, .055f, {.25f, .29f, .3f});
      }
      if (drop.installed) {
        scene.glowTubes.push_back(
            instance({slot.x, y - 9, slot.z}, {.7f, .3f, .7f}, {.2f, .7f, 1}));
        scene.hot.push_back(
            instance({slot.x, y - 11.1f, slot.z}, {2.1f, 5, 1}, {.2f, .6f, 1}, 0, 0, .8f));
      }
    }
  }
};
inline void buildSlotOverlay(GameScene& scene, const Session& s, int hovered) {
  if (!s.tactical)
    return;
  for (int i = 0; i < SLOT_COUNT; ++i) {
    const auto& p = TURRET_SLOTS[i];
    bool available = s.slotAvailable(i);
    Color color = available ? Color{.2f, .65f, .6f} : Color{.24f, .36f, .4f};
    for (const auto& d : s.sentryDrops)
      if (d.slot == i && !d.installed)
        color = {1, .6f, .18f};
    if (i == hovered)
      color = available && s.sentryStock > 0 ? Color{.6f, 1, .8f} : Color{1, .3f, .25f};
    for (int side = 0; side < 6; ++side) {
      float a = PI / 6 + side * PI / 3, b = a + PI / 3;
      scene.rod({p.x + std::cos(a) * SLOT_RADIUS, .12f, p.z + std::sin(a) * SLOT_RADIUS},
                {p.x + std::cos(b) * SLOT_RADIUS, .12f, p.z + std::sin(b) * SLOT_RADIUS}, .075f,
                color, true);
    }
  }
  if (hovered < 0 || s.serviceTarget != Service::None)
    return;
  const auto& p = TURRET_SLOTS[hovered];
  if (s.sentryStock > 0) {
    Tower ghost;
    ghost.type = s.pendingSentry();
    ghost.x = p.x;
    ghost.z = p.z;
    scene.makeTower(ghost, s.world.time, true, s.slotAvailable(hovered));
  }
  Weapon type = s.pendingSentry();
  bool showRange = s.sentryStock > 0;
  if (s.sentryStock == 0) {
    for (const auto& tower : s.world.towers)
      if (std::hypot(tower.x - p.x, tower.z - p.z) < 1) {
        type = tower.type;
        showRange = true;
      }
    for (const auto& drop : s.sentryDrops)
      if (drop.slot == hovered) {
        type = drop.type;
        showRange = true;
      }
  }
  if (!showRange)
    return;
  float diameter = s.world.previewWeaponInfo(type, p.x, p.z).range * 2;
  if (type == RateRelay || type == RangeRelay)
    for (const auto& target : s.world.towers)
      if (!isSupport(target.type) && supportNeighbor(p.x, p.z, target.x, target.z))
        scene.beams.push_back(lineInstance({p.x, WEAPONS[type].height, p.z},
                                           {target.x, WEAPONS[target.type].height, target.z},
                                           WEAPONS[type].color, .14f));
  scene.ringData.push_back(
      instance({p.x, .15f, p.z}, {diameter, diameter, 1}, WEAPONS[type].color, 0, 0, .6f));
}
} // namespace command
