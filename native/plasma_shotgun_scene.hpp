#pragma once
#include "manual_scene.hpp"
namespace command {
class PlasmaShotgunScene {
  Com<ID3D11ShaderResourceView> walnut, steel;
  Mesh barrel;
  Mesh* cube;

public:
  PlasmaShotgunScene(Renderer& r, const std::filesystem::path& assets) : cube(&r.cube) {
    r.loadTexture(assets / "shotgun-walnut.png", walnut);
    r.loadTexture(assets / "shotgun-steel.png", steel);
    std::vector<Vertex> vertices;
    auto face = [&](Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal) {
      int order[] = {0, 1, 2, 0, 2, 3};
      Vec3 p[] = {a, b, c, d};
      float uv[][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
      for (int n : order)
        vertices.push_back(
            {p[n].x, p[n].y, p[n].z, normal.x, normal.y, normal.z, uv[n][0], uv[n][1]});
    };
    for (int n = 0; n < 48; ++n) {
      float a = n * 2 * PI / 48, b = (n + 1) * 2 * PI / 48;
      Vec3 p{std::sin(a), std::cos(a), 0}, q{std::sin(b), std::cos(b), 0}, end{0, 0, 1};
      face(p, q, q + end, p + end, normalize(p + q));
      face(p * .70f, p * .70f + end, q * .70f + end, q * .70f, normalize(p + q) * -1);
      face(p, q, q * .70f, p * .70f, {0, 0, -1});
      face(p + end, p * .70f + end, q * .70f + end, q + end, {0, 0, 1});
    }
    r.uploadMesh(barrel, vertices);
  }
  void append(GameScene& scene, const Session& s) {
    if (s.module.kind != Gun::Shotgun || s.mode == Mode::Selection)
      return;
    const auto& cycle = s.shotgun;
    Color dark{.035f, .045f, .06f}, silver{.8f, .88f, .95f}, brass{.65f, .42f, .16f},
        plasma{.28f, .8f, 1};
    float lift = cycle.elevation();
    float extension = cycle.extension();
    float loaderDrop = .55f * (1 - PlasmaShotgun::ease(s.lift));
    Vec3 assemblyOffset{};
    auto local = [&](Vec3 p, bool moving = false) {
      if (moving)
        p = cycle.barrelPoint(p);
      p = p + assemblyOffset;
      p.z -= s.recoil * .25f;
      return p;
    };
    auto point = [&](Vec3 p, bool moving = false) { return s.gunPoint(local(p, moving)); };
    auto box = [&](Vec3 p, Vec3 size, Color c, bool wood = false, bool moving = false) {
      scene.extraDraws.push_back({cube,
                                  instance(point(p, moving), size, c, s.gunYaw,
                                           -s.gunPitch + (moving ? cycle.breakAngle() : 0)),
                                  wood ? walnut.p : steel.p, 0});
    };
    auto rod = [&](Vec3 a, Vec3 b, float radius, Color c, bool glow = false, bool moving = false) {
      scene.rod(point(a, moving), point(b, moving), radius, c, glow);
    };
    // The low fixed receiver leaves the raised breech and batteries in view.
    box({0, -.36f, -.22f}, {1.6f, .4f, 1.45f}, silver);
    box({0, -.4f, -1.13f}, {1.0f, .45f, .8f}, {1, 1, 1}, true);
    box({0, -.62f, -.8f}, {.55f, .55f, .6f}, {1, 1, 1}, true);
    // Keep the central battery corridor open with a low sliding U-shaped carriage.
    box({0, -.82f + lift, .7f + extension}, {2.18f, .12f, .5f}, silver);
    for (float side : {-1.f, 1.f}) {
      box({side * .81f, -.36f, -.22f}, {.045f, .24f, 1.1f}, brass);
      for (int n = 0; n < 5; ++n)
        box({side * .84f, -.36f, -.65f + n * .23f}, {.025f, .05f, .08f}, silver);
      // Two telescopic rams lift the barrel carriage before the hinge opens.
      rod({side * 1.03f, -.95f, .7f}, {side * 1.03f, -.78f, .7f}, .14f, dark);
      rod({side * 1.03f, -.8f, .7f}, {side * 1.03f, -.78f + lift, .7f}, .085f, silver);
      rod({side * 1.03f, -.76f + lift, .48f}, {side * 1.03f, -.76f + lift, 1.48f}, .065f, silver);
      rod({side * 1.03f, -.78f + lift, .7f + extension},
          {side * 1.03f, -.28f + lift, .7f + extension}, .09f, silver);
      box({side * 1.03f, -.28f + lift, .7f + extension}, {.16f, .22f, .34f}, brass);
      rod({side * .87f, -.24f + lift, .7f + extension},
          {side * 1.03f, -.28f + lift, .7f + extension}, .09f, silver);
      scene.extraDraws.push_back({&barrel,
                                  instance(point({side * .5f, 0, .7f}, true), {.42f, .42f, 4.1f},
                                           silver, s.gunYaw, -s.gunPitch + cycle.breakAngle()),
                                  steel, 0});
      for (float z : {.76f, 1.1f, 3.85f, 4.65f}) {
        for (int n = 0; n < 24; ++n) {
          float a = n * PI / 12, b = (n + 1) * PI / 12;
          rod({side * .5f + std::cos(a) * .435f, std::sin(a) * .435f, z},
              {side * .5f + std::cos(b) * .435f, std::sin(b) * .435f, z}, .027f, brass, false,
              true);
        }
      }
      rod({side * .5f, 0, 4.65f}, {side * .5f, 0, 4.7f}, .265f, dark, false, true);
      rod({side * .5f, 0, 4.70f}, {side * .5f, 0, 4.72f}, .16f, plasma, true, true);
    }
    box({0, -.39f, 2.3f}, {1.65f, .3f, 2.0f}, {1, 1, 1}, true, true);
    box({0, .41f, 2.6f}, {.16f, .1f, 3.7f}, brass, false, true);
    // Rear saddle joins the sight rib to both barrels and moves with their hinge.
    box({0, .4f, .88f}, {1.2f, .16f, .24f}, silver, false, true);
    box({0, .45f, .79f}, {.25f, .16f, .16f}, brass, false, true);
    for (float side : {-1.f, 1.f}) {
      box({side * .5f, .50f, .88f}, {.09f, .06f, .09f}, dark, false, true);
      rod({side * .5f, .36f, .88f}, {side * .72f, .27f, .88f}, .055f, brass, false, true);
    }
    box({0, .49f, 4.5f}, {.07f, .14f, .12f}, plasma, false, true);
    auto cell = [&](int side, bool fresh) {
      Vec3 axis{0, -std::sin(cycle.breakAngle()), std::cos(cycle.breakAngle())};
      if (cycle.reloading &&
          ((!fresh && cycle.progress() >= .36f) || (fresh && cycle.progress() < .56f)))
        axis = cycle.loadingAxis();
      Vec3 p = local(cycle.batteryPoint(side, fresh));
      if (cycle.reloading &&
          ((!fresh && cycle.progress() >= .36f) || (fresh && cycle.progress() < .56f)))
        p.y -= loaderDrop;
      Color body = fresh ? Color{.36f, .53f, .58f} : Color{.10f, .105f, .11f};
      auto emit = [&](Vec3 a, Vec3 b, float radius, Color c, bool glow = false) {
        scene.rod(s.gunPoint(a), s.gunPoint(b), radius, c, glow);
      };
      emit(p - axis * .55f, p + axis * .12f, .26f, body);
      emit(p - axis * .58f, p - axis * .5f, .3f, fresh ? brass : Color{.28f, .21f, .13f});
      emit(p + axis * .12f, p + axis * .20f, .22f, silver);
      for (int ring = 0; ring < 4; ++ring) {
        auto a = p - axis * (ring * .12f);
        emit(a, a + axis * .025f, .268f, fresh ? plasma : Color{.17f, .1f, .045f}, fresh);
      }
    };
    for (int side = 0; side < 2; ++side) {
      if (cycle.spentVisible(side))
        cell(side, false);
      if (cycle.freshVisible(side))
        cell(side, true);
    }
    // A permanent overhead cassette behind the seat supplies both chambers.
    // Its gantry, carriage and two sockets stay attached throughout both trips.
    // Stow the assembly lower under the closed canopy; unfold it with the main lift.
    assemblyOffset.y = -loaderDrop;
    Vec3 head = cycle.loaderHead();
    float doors = cycle.loaderDoors();
    for (float side : {-1.f, 1.f}) {
      rod({side * .85f, -.8f, -.2f}, {side * 1.02f, -.8f, -2.6f}, .11f, dark);
      rod({side * 1.02f, -.8f, -2.6f}, {side * 1.02f, 2.95f, -2.6f}, .11f, silver);
      box({side * 1.02f, 2.5f, -2.55f}, {.16f, 1.6f, 1.65f}, silver);
      box({side * 1.02f, 1.3f, -2.6f}, {.2f, .13f, .35f}, brass);
      box({side * (.53f + doors * 1.04f), 2.5f, -1.7f}, {1.04f, 1.55f, .1f}, silver);
      box({side * (.08f + doors * 1.04f), 2.5f, -1.635f}, {.035f, 1.25f, .035f},
          doors > .1f ? plasma : brass);
      rod({side * .82f, 2.95f, -2.6f}, {side * .82f, 2.95f, -1.8f}, .12f, dark);
      rod({side * .82f, 2.95f, -1.82f}, {side * .82f, 2.95f, head.z}, .065f, silver);
      rod({side * .67f, 2.95f, head.z}, {side * .67f, 2.3f, head.z}, .10f, dark);
      rod({side * .67f, 2.3f, head.z}, {side * .67f, head.y, head.z}, .055f, silver);
    }
    box({0, 3.32f, -2.55f}, {2.2f, .12f, 1.8f}, silver);
    box({0, 1.69f, -2.55f}, {2.2f, .12f, 1.8f}, dark);
    box({0, 2.5f, -3.4f}, {2.2f, 1.75f, .12f}, {1, 1, 1}, true);
    rod({-.82f, 2.95f, head.z}, {.82f, 2.95f, head.z}, .075f, brass);
    scene.extraDraws.push_back(
        {cube, instance(point(head), {1.6f, .12f, .22f}, silver, s.gunYaw, -s.gunPitch + .95f),
         steel, 0});
    Vec3 axis = cycle.loadingAxis(), up{0, std::cos(.95f), std::sin(.95f)};
    float jawRadius = .38f - cycle.loaderGrip() * .095f;
    for (float side : {-1.f, 1.f}) {
      Vec3 socket = head + Vec3{side * .5f, 0, 0};
      rod(socket, socket + axis * .15f, .13f, brass);
      for (int n = 0; n < 16; ++n) {
        float a = n * PI / 8, b = (n + 1) * PI / 8;
        auto rim = [&](float angle) {
          return socket + axis * .25f +
                 (Vec3{std::cos(angle), 0, 0} + up * std::sin(angle)) * jawRadius;
        };
        rod(rim(a), rim(b), .035f, brass);
        if (n % 4 == 0)
          rod(rim(a) - axis * .12f, rim(a) + axis * .14f, .045f, dark);
      }
    }
    assemblyOffset = {};
    float lever = cycle.lever(), rotation = lever * 2.4f;
    // Top-mounted charging handle: exposed rails, sliding crossbar and rack drive.
    float handleZ = .32f - lever * .65f;
    float handleY = .48f + lever * .32f;
    for (float side : {-1.f, 1.f}) {
      rod({side * .35f, -.08f, -.72f}, {side * .35f, -.08f, .5f}, .045f, silver);
      box({side * .35f, .04f, handleZ}, {.15f, .18f, .24f}, brass);
      rod({side * .35f, .04f, handleZ}, {side * .26f, handleY, handleZ}, .05f, silver);
      for (int n = 0; n < 12; ++n) {
        float a = rotation + n * PI / 6;
        Vec3 p{side * .58f, -.02f + std::sin(a) * .19f, -.2f + std::cos(a) * .19f};
        rod(p, p + Vec3{side * .09f, 0, 0}, .05f, brass);
        rod({side * .58f, -.02f, -.2f}, p, .018f, silver);
      }
    }
    rod({-.26f, handleY, handleZ}, {.26f, handleY, handleZ}, .065f, dark);
    box({0, .02f, handleZ}, {.16f, .1f, .35f}, silver);
    for (int n = 0; n < 8; ++n)
      box({0, -.06f, handleZ - .4f + n * .1f}, {.22f, .045f, .035f}, brass);
    rod({.18f, -.02f, .45f}, {.18f, -.02f, .05f}, .10f, dark);
    rod({.18f, -.02f, .05f}, {.18f, -.02f, handleZ - .23f}, .045f, silver);
    if (s.shotFlash > 0)
      scene.hot.push_back(instance(s.shotgunMuzzle(cycle.lastBarrel), {1.4f, 1.4f, 1}, plasma, 0, 0,
                                   s.shotFlash * 7));
  }
};
} // namespace command
