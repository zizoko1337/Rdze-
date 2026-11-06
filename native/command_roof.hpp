#pragma once
#include "game_scene.hpp"
#include "command_game.hpp"

namespace command {
// Immutable meshes, uploaded once. Two semicircular hatch leaves slide clear of the lift.
class CommandRoof {
  Mesh canopy, hatch;
  Mesh* deck;

public:
  explicit CommandRoof(Renderer& r) : deck(&r.cylinder) {
    std::vector<Vertex> vertices;
    auto triangle = [&](Vec3 a, Vec3 b, Vec3 c, Vec3 normal) {
      for (auto p : {a, b, c})
        vertices.push_back({p.x, p.y, p.z, normal.x, normal.y, normal.z,
                            .5f + p.x / (2 * Session::HATCH_RADIUS),
                            .5f + p.z / (2 * Session::HATCH_RADIUS)});
    };
    auto quad = [&](Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal) {
      triangle(a, b, c, normal);
      triangle(a, c, d, normal);
    };
    constexpr int segments = 64;
    constexpr float thickness = .16f;
    // The metal sampler clamps UVs. Give each panel its own bounded, metric UVs
    // rather than projecting the whole roof beyond the texture's [0, 1] range.
    auto panel = [&](Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal) {
      Vec3 center = (a + b + c + d) * .25f;
      Vec3 across = normalize(b - a);
      Vec3 along = normalize((c + d) - (a + b));
      constexpr float tile = 1.6f; // Same metres per texture on every roof panel.
      for (auto p : {a, b, c, a, c, d}) {
        Vec3 delta = p - center;
        vertices.push_back({p.x, p.y, p.z, normal.x, normal.y, normal.z,
                            .5f + dot(delta, across) / tile,
                            .5f + dot(delta, along) / tile});
      }
    };
    for (int i = 0; i < segments; ++i) {
      float a = i * 2 * PI / segments, b = (i + 1) * 2 * PI / segments;
      Vec3 innerA{std::sin(a) * Session::HATCH_RADIUS, Session::ROOF,
                  std::cos(a) * Session::HATCH_RADIUS};
      Vec3 innerB{std::sin(b) * Session::HATCH_RADIUS, Session::ROOF,
                  std::cos(b) * Session::HATCH_RADIUS};
      Vec3 outerA{std::sin(a) * Session::ROOF_RADIUS, Session::EAVE,
                  std::cos(a) * Session::ROOF_RADIUS};
      Vec3 outerB{std::sin(b) * Session::ROOF_RADIUS, Session::EAVE,
                  std::cos(b) * Session::ROOF_RADIUS};
      Vec3 normal = normalize(Vec3{std::sin((a + b) / 2) * (Session::ROOF - Session::EAVE),
                                   Session::ROOF_RADIUS - Session::HATCH_RADIUS,
                                   std::cos((a + b) / 2) * (Session::ROOF - Session::EAVE)});
      Vec3 down{0, -thickness, 0};
      constexpr int bands = 8;
      for (int ring = 0; ring < bands; ++ring) {
        float t0 = float(ring) / bands, t1 = float(ring + 1) / bands;
        Vec3 a0 = innerA + (outerA - innerA) * t0;
        Vec3 b0 = innerB + (outerB - innerB) * t0;
        Vec3 a1 = innerA + (outerA - innerA) * t1;
        Vec3 b1 = innerB + (outerB - innerB) * t1;
        panel(a0, b0, b1, a1, normal);
        panel(b0 + down, a0 + down, a1 + down, b1 + down, normal * -1);
      }
      panel(outerA, outerB, outerB + down, outerA + down,
            normalize(Vec3{outerA.x, 0, outerA.z}));
      panel(innerB, innerA, innerA + down, innerB + down,
            normalize(Vec3{-innerA.x, 0, -innerA.z}));
    }
    r.uploadMesh(canopy, vertices);
    vertices.clear();
    for (int i = 0; i < segments / 2; ++i) {
      float a = i * 2 * PI / segments, b = (i + 1) * 2 * PI / segments;
      Vec3 p{std::sin(a) * Session::HATCH_RADIUS, 0, std::cos(a) * Session::HATCH_RADIUS};
      Vec3 q{std::sin(b) * Session::HATCH_RADIUS, 0, std::cos(b) * Session::HATCH_RADIUS};
      Vec3 down{0, -.12f, 0};
      triangle({0, 0, 0}, p, q, {0, 1, 0});
      triangle(down, q + down, p + down, {0, -1, 0});
      panel(p, q, q + down, p + down, normalize(Vec3{p.x, 0, p.z}));
    }
    quad({0, 0, -Session::HATCH_RADIUS}, {0, -.12f, -Session::HATCH_RADIUS},
         {0, -.12f, Session::HATCH_RADIUS}, {0, 0, Session::HATCH_RADIUS}, {-1, 0, 0});
    r.uploadMesh(hatch, vertices);
  }
  void append(GameScene& scene, const Session& s) {
    scene.extraOpaque.push_back({&canopy, instance({0, 0, 0}, {1, 1, 1}, {.32f, .42f, .47f})});
    for (float sign : {-1.f, 1.f}) {
      scene.extraOpaque.push_back(
          {&hatch, instance({sign * s.hatch * 4.1f, Session::ROOF + .03f, 0}, {1, 1, 1},
                            {.38f, .48f, .52f}, sign > 0 ? 0 : PI)});
    }
    // Circular deck lip matches the outer rail instead of leaving rails inboard on an octagon.
    scene.extraOpaque.push_back(
        {deck, instance({0, CORE_HEIGHT + .15f, 0},
                        {Session::DECK_RADIUS, .12f, Session::DECK_RADIUS}, {.23f, .31f, .35f})});
  }
};
} // namespace command
