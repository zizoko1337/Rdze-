#pragma once
#include "command_game.hpp"
#include "game_scene.hpp"
namespace command {
inline void serviceHex(GameScene& scene, Vec3 p, Color color, float width) {
  for (int i = 0; i < 6; ++i) {
    float a = PI / 6 + i * PI / 3, b = a + PI / 3;
    scene.rod(p + Vec3{std::cos(a) * SLOT_RADIUS, .3f, std::sin(a) * SLOT_RADIUS},
              p + Vec3{std::cos(b) * SLOT_RADIUS, .3f, std::sin(b) * SLOT_RADIUS}, width, color,
              true);
  }
}
class ServiceScene {
  Com<ID3D11ShaderResourceView> pigTexture;
  std::vector<Instance> pigSprites;
  void drone(GameScene& scene, Vec3 p, float time) {
    Color steel{.35f, .45f, .5f}, dark{.08f, .12f, .15f}, cyan{.2f, .85f, 1};
    scene.box(p, {1.6f, .65f, 1.2f}, steel);
    scene.box(p + Vec3{0, .15f, -.63f}, {.7f, .18f, .1f}, cyan, 0, 0, true);
    for (float x : {-1.f, 1.f})
      for (float z : {-1.f, 1.f}) {
        Vec3 rotor = p + Vec3{x * 1.25f, .28f, z};
        scene.rod(p, rotor, .1f, dark);
        scene.tube(rotor, {1.05f, .12f, 1.05f}, dark);
        scene.box(rotor + Vec3{0, .13f, 0}, {1.25f, .035f, .11f}, steel, time * 40 + x * z);
        scene.hot.push_back(instance(rotor + Vec3{0, -.35f, 0}, {.45f, .8f, 1}, cyan, 0, 0, .3f));
      }
  }
  void cage(GameScene& scene, const ServiceCast& cast) {
    float age = cast.age, t = std::clamp(age / ServiceSystem::LAND_TIME, 0.f, 1.f);
    float height = 62 * (1 - t) * (1 - t) * (1 + 2 * t);
    Vec3 p = cast.position + Vec3{0, height, 0};
    Color steel{.36f, .44f, .47f}, dark{.11f, .15f, .18f}, cyan{.2f, .85f, 1};
    if (age < ServiceSystem::RELEASE_TIME) {
      scene.box(p + Vec3{0, .3f, 0}, {6.6f, .6f, 5.2f}, dark);
      scene.box(p + Vec3{0, 4.2f, 0}, {6.6f, .35f, 5.2f}, steel);
      for (float z : {-2.4f, 2.4f})
        for (int i = 0; i < 8; ++i) {
          float x = -3.f + i * 6.f / 7;
          scene.rod(p + Vec3{x, .6f, z}, p + Vec3{x, 4.1f, z}, .09f, steel);
        }
      for (float x : {-3.1f, 3.1f})
        for (int i = 0; i < 6; ++i) {
          float z = -2.4f + i * .96f;
          scene.rod(p + Vec3{x, .6f, z}, p + Vec3{x, 4.1f, z}, .09f, steel);
        }
      for (float z : {-2.45f, 2.45f})
        scene.box(p + Vec3{0, 2.1f, z}, {6.2f, .17f, .12f}, dark);
      scene.box(p + Vec3{0, 2, -2.58f}, {.6f, .85f, .12f},
                age >= ServiceSystem::LAND_TIME ? Color{1, .25f, .1f} : cyan, 0, 0, true);
      scene.shadows.push_back(
          instance(cast.position + Vec3{0, .05f, 0}, {7, 6, 1}, {0, 0, 0}, 0, 0, .35f));
      if (age >= ServiceSystem::LAND_TIME) {
        float remaining = ServiceSystem::RELEASE_TIME - age;
        scene.ringData.push_back(instance(cast.position + Vec3{0, .18f, 0},
                                          {7 + remaining * 3, 7 + remaining * 3, 1}, {1, .3f, .08f},
                                          0, 0, .8f));
      }
    } else if (age < ServiceSystem::RELEASE_TIME + 1.8f) {
      float blast = age - ServiceSystem::RELEASE_TIME;
      for (int i = 0; i < 4; ++i) {
        float a = i * PI / 2;
        Vec3 offset{std::cos(a) * (3 + blast * 5), 1 + blast * 7 - blast * blast * 3,
                    std::sin(a) * (3 + blast * 5)};
        scene.box(cast.position + offset, {5, .15f, 3}, steel, a + blast * 2, blast * 1.5f);
      }
    }
    if (age < ServiceSystem::LAND_TIME + 3.5f) {
      float depart = std::max(0.f, age - ServiceSystem::LAND_TIME);
      for (int i = 0; i < 4; ++i) {
        float x = i & 1 ? 1.f : -1.f, z = i & 2 ? 1.f : -1.f;
        Vec3 anchor = cast.position + Vec3{x * 2.7f, height + 4.4f, z * 2};
        Vec3 carrier =
            cast.position + Vec3{x * (4.5f + depart * 8), height + 12 + depart * depart * 6,
                                 z * (3.8f + depart * 8)};
        drone(scene, carrier, age + i * .3f);
        if (depart == 0)
          scene.rod(anchor, carrier, .045f, {.18f, .23f, .25f});
      }
    }
  }

public:
  explicit ServiceScene(Renderer& r, const std::filesystem::path& assets) {
    r.loadTexture(assets / "cyber-pig-profile.png", pigTexture);
    pigSprites.reserve(CyberPigSystem::MAX_PIGS + 2 * ServiceSystem::MAX_CASTS);
  }
  void append(GameScene& scene, const Session& s, Vec3 hover = {}, int slot = -1,
              Vec3 cameraRight = {1, 0, 0}) {
    pigSprites.clear();
    if (s.serviceTarget != Service::None) {
      Color color = SERVICES[int(s.serviceTarget)].color;
      if (s.serviceTarget == Service::CyberPigs && slot >= 0 && slot < SLOT_COUNT) {
        const auto& p = TURRET_SLOTS[slot];
        serviceHex(scene, {p.x, 0, p.z}, s.slotAvailable(slot) ? color : Color{.5f, .12f, .12f},
                   .18f);
        float radius = std::hypot(p.x, p.z) * 2;
        scene.ringData.push_back(instance({0, .22f, 0}, {radius, radius, 1}, color, 0, 0, .65f));
      } else if (s.serviceTarget == Service::Brimstone)
        scene.ringData.push_back(instance({hover.x, .2f, hover.z}, {20, 20, 1}, color, 0, 0, .85f));
    }
    for (const auto& cast : s.services.casts) {
      Color color = SERVICES[int(cast.type)].color;
      if (cast.type == Service::CyberPigs) {
        cage(scene, cast);
        if (cast.age < ServiceSystem::RELEASE_TIME) {
          float t = std::clamp(cast.age / ServiceSystem::LAND_TIME, 0.f, 1.f);
          float height = 62 * (1 - t) * (1 - t) * (1 + 2 * t);
          for (float side : {-1.f, 1.f})
            pigSprites.push_back(instance(cast.position + Vec3{side * .15f, height + 2.35f, side * 1.1f},
                                          {side * 6.4f, 4.27f, 1}, {.65f, .7f, .75f}));
        }
      } else if (cast.type == Service::Brimstone) {
        scene.ringData.push_back(
            instance(cast.position + Vec3{0, .25f, 0}, {20, 20, 1}, color, 0, 0, .9f));
        if (cast.age < 3) {
          float size = 2 + (3 - cast.age) * 3;
          scene.ringData.push_back(
              instance(cast.position + Vec3{0, .3f, 0}, {size, size, 1}, {1, .7f, .3f}, 0, 0, .8f));
        } else {
          float punch = std::exp(-std::fmod(cast.age - 3, .5f) * 12);
          scene.beams.push_back(
              lineInstance(cast.position + Vec3{0, 1000, 0}, cast.position, color, 10 + punch * 6));
          scene.beams.push_back(lineInstance(cast.position + Vec3{0, 1000, 0}, cast.position,
                                             {1, .95f, .8f}, 3 + punch * 2));
          scene.hot.push_back(instance(cast.position + Vec3{0, 2, 0},
                                       {20 + punch * 15, 20 + punch * 15, 1}, color, 0, 0, .8f));
          for (int i = 0; i < 5; ++i) {
            float a = cast.age * 5 + i * 2 * PI / 5;
            Vec3 offset{std::sin(a) * 5, 0, std::cos(a) * 5};
            scene.beams.push_back(lineInstance(cast.position + offset + Vec3{0, 180, 0},
                                               cast.position - offset, {1, .65f, .15f}, .5f));
          }
        }
      } else if (cast.age < 3) {
        float size = 4 + std::fmod(cast.age, 1.f) * 25;
        scene.ringData.push_back(
            instance({0, CORE_HEIGHT + .4f, 0}, {size, size, 1}, color, 0, 0, .8f));
      }
    }
    for (const auto& pig : s.services.cyberPigs.pigs) {
      Vec3 p = pig.position + Vec3{0, 1.75f + std::sin(s.world.time * 24 + pig.sign) * .045f, 0};
      pigSprites.push_back(
          instance(p, {cyberPigFacing(pig.direction, cameraRight) * 6.4f, 4.27f, 1}, {1, 1, 1}));
      scene.shadows.push_back(
          instance(pig.position + Vec3{0, .07f, 0}, {5, 2.8f, 1}, {0, 0, 0}, 0, 0, .4f));
    }
    scene.extraSprites.push_back({pigTexture, &pigSprites});
  }
};
} // namespace command
