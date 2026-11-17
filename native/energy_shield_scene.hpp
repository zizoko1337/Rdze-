#pragma once
#include "deck_instruments.hpp"

namespace command {
class EnergyShieldScene {
  struct CompactFrame {
    InstrumentFrame base;
    Vec3 point(Vec3 p) const { return base.point(p * EnergyShield::CRATE_SCALE); }
    void box(Vec3 p, Vec3 size, Color c, bool glow = false) const {
      base.box(p * EnergyShield::CRATE_SCALE, size * EnergyShield::CRATE_SCALE, c, glow);
    }
    void text(Vec3 p, const std::string& value, float size, Color c) const {
      base.text(p * EnergyShield::CRATE_SCALE, value, size * EnergyShield::CRATE_SCALE, c);
    }
    void rod(Vec3 a, Vec3 b, float radius, Color c, bool glow = false) const {
      base.rod(a * EnergyShield::CRATE_SCALE, b * EnergyShield::CRATE_SCALE,
               radius * EnergyShield::CRATE_SCALE, c, glow);
    }
    void dial(Vec3 p, float value, float maximum, const std::string& label, Color needle) const {
      constexpr float radius = .69f;
      rod(p, p + Vec3{0, 0, -.13f}, radius, {.45f, .39f, .25f});
      rod(p + Vec3{0, 0, -.135f}, p + Vec3{0, 0, -.17f}, radius * .86f, {.72f, .76f, .67f});
      for (int n = 0; n <= 20; ++n) {
        float a = -2.3f + n * .23f;
        rod(p + Vec3{std::sin(a) * radius * (n % 5 == 0 ? .66f : .74f),
                     std::cos(a) * radius * (n % 5 == 0 ? .66f : .74f), -.18f},
            p + Vec3{std::sin(a) * radius * .82f, std::cos(a) * radius * .82f, -.18f},
            n % 5 == 0 ? .015f : .008f, {.035f, .055f, .055f});
      }
      for (int n = 0; n < 3; ++n) {
        float a = -2.3f + n * 2.3f;
        text(p + Vec3{std::sin(a) * radius * .5f - .08f, std::cos(a) * radius * .5f + .03f, -.186f},
             number(maximum * n / 2), .105f, {.025f, .04f, .045f});
      }
      float a = -2.3f + std::clamp(value / maximum, 0.f, 1.f) * 4.6f;
      rod(p + Vec3{0, 0, -.205f},
          p + Vec3{std::sin(a) * radius * .65f, std::cos(a) * radius * .65f, -.205f}, .018f,
          needle);
      rod(p + Vec3{0, 0, -.20f}, p + Vec3{0, 0, -.23f}, .055f, {.22f, .25f, .24f});
      text(p + Vec3{-.25f, -.23f, -.19f}, number(value), .12f, {.03f, .045f, .045f});
      text(p + Vec3{-.65f, -.83f, -.04f}, label, .105f, {.65f, .78f, .75f});
    }
    void gear(Vec3 p, float radius, float angle) const {
      rod(p, p + Vec3{.16f, 0, 0}, radius * .83f, {.35f, .4f, .4f});
      for (int n = 0; n < 12; ++n) {
        float a = angle + n * 2 * PI / 12;
        rod(p + Vec3{.11f, std::sin(a) * radius * .72f, std::cos(a) * radius * .72f},
            p + Vec3{.11f, std::sin(a) * radius * 1.10f, std::cos(a) * radius * 1.10f}, .065f,
            {.63f, .52f, .3f});
        if (n % 3 == 0)
          rod(p + Vec3{.19f, 0, 0},
              p + Vec3{.19f, std::sin(a) * radius * .6f, std::cos(a) * radius * .6f}, .025f,
              {.09f, .13f, .14f});
      }
      rod(p + Vec3{.17f, 0, 0}, p + Vec3{.24f, 0, 0}, .12f, {.18f, .24f, .25f});
    }
  };
  Com<ID3D11ShaderResourceView> logo;
  Mesh shell;
  Mesh* quad;

public:
  static constexpr float RADIUS = CORE_RADIUS + .8f, HEIGHT = Session::EAVE - .25f;
  EnergyShieldScene(Renderer& r, const std::filesystem::path& assets) : quad(&r.quad) {
    r.loadTexture(assets / "jan-kowalski-company.png", logo);
    std::vector<Vertex> vertices;
    constexpr int slices = 128;
    for (int i = 0; i < slices; ++i) {
      float a = float(i) / slices * 2 * PI, b = float(i + 1) / slices * 2 * PI;
      auto vertex = [](float angle, float y) {
        return Vertex{std::sin(angle) * RADIUS, y, std::cos(angle) * RADIUS,
                      std::sin(angle),          0, std::cos(angle),
                      angle * RADIUS,           y};
      };
      auto loA = vertex(a, .1f), hiA = vertex(a, HEIGHT);
      auto loB = vertex(b, .1f), hiB = vertex(b, HEIGHT);
      for (auto v : {loA, hiA, hiB, loA, hiB, loB})
        vertices.push_back(v);
    }
    r.uploadMesh(shell, vertices);
  }
  void append(GameScene& scene, const Session& s) {
    const auto& shield = s.expedition.shield;
    if (!shield.purchased || s.mode == Mode::Selection)
      return;
    const Color steel{.42f, .5f, .53f}, dark{.055f, .09f, .11f}, cyan{.2f, .8f, 1},
        amber{1, .56f, .12f};
    // Small floor cabinet in the free east-side bay, facing the operator.
    CompactFrame f{{scene, shield.cratePosition(), EnergyShield::yaw()}};
    f.box({0, 2, 0}, {3.9f, 4, 2.3f}, dark);
    f.box({0, .12f, 0}, {4.1f, .24f, 2.5f}, steel);
    f.box({0, 4, 0}, {4.1f, .18f, 2.5f}, steel);
    for (float x : {-1.82f, 1.82f}) {
      f.box({x, 2, -1.19f}, {.09f, 3.7f, .06f}, shield.online() ? cyan : amber, true);
      f.box({x, 2, 1.1f}, {.13f, 3.7f, .12f}, steel);
    }
    f.dial({-.92f, 3.25f, -1.22f}, 100 * shield.hp / shield.maxHp, 100, "INTEGRALNOŚĆ %",
           {.9f, .12f, .05f});
    f.dial({.92f, 3.25f, -1.22f}, std::ceil(shield.rebuildRemaining), shield.rebuildDuration,
           "ODBUDOWA / s", {.9f, .38f, .04f});
    float phase = shield.mechanismPhase;
    f.gear({2.02f, 1.1f, -.3f}, .48f, phase);
    f.gear({2.02f, 1.98f, -.3f}, .32f, -phase * 1.5f + .12f);
    for (float z : {-.6f, .6f}) {
      f.rod({-2.09f, .55f, z}, {-2.09f, 3.2f, z}, .09f, steel);
      f.rod({-2.09f, 3.2f, z}, {-1.7f, 3.2f, z}, .09f, steel);
      for (float y : {.7f, 1.4f, 2.1f, 2.8f})
        f.rod({-2.09f, y - .07f, z}, {-2.09f, y + .07f, z}, .14f, {.5f, .4f, .24f});
    }
    float stroke = .2f + .28f * std::sin(phase * 2);
    f.rod({-2.21f, 1, 0}, {-2.21f, 1.75f, 0}, .22f, dark);
    f.rod({-2.21f, 1.55f, 0}, {-2.21f, 2.35f + stroke, 0}, .08f, steel);
    f.rod({-2.21f, 2.3f + stroke, 0}, {-2.21f, 2.45f + stroke, 0}, .23f, {.6f, .48f, .26f});
    float extension = shield.antennaExtension();
    std::array<Vec3, 2> tips;
    for (int n = 0; n < 2; ++n) {
      float side = n == 0 ? -1.f : 1.f;
      Vec3 base{side * 2.04f, 2.55f, .1f};
      tips[n] = {side * (2.04f + .45f * extension), 2.8f + 2.25f * extension, .1f};
      f.box(base, {.35f, .65f, .65f}, steel);
      f.rod(base, tips[n], .075f, steel);
      for (int coil = 0; coil < 3; ++coil) {
        auto p = tips[n] + Vec3{0, -.2f - coil * .14f, 0};
        f.rod(p, p + Vec3{0, .07f, 0}, .16f, {.2f, .28f, .3f});
      }
      f.rod(tips[n], tips[n] + Vec3{0, .13f, 0}, .13f, cyan, shield.arcStrength() > 0);
    }
    float arc = shield.arcStrength();
    if (arc > 0) {
      Vec3 previous = f.point(tips[0] + Vec3{0, .13f, 0});
      for (int n = 1; n <= 18; ++n) {
        float t = n / 18.f;
        Vec3 p = tips[0] * (1 - t) + tips[1] * t + Vec3{0, .13f, 0};
        if (n < 18)
          p = p + Vec3{0, std::sin(n * 8.37f + phase * 43) * .24f,
                       std::cos(n * 6.1f - phase * 31) * .14f} *
                      arc;
        Vec3 point = f.point(p);
        scene.beams.push_back(lineInstance(previous, point, {.15f, .65f, 1}, .027f * arc));
        scene.beams.push_back(lineInstance(previous, point, {.8f, .95f, 1}, .008f * arc));
        previous = point;
      }
    }
    // Fixed decal on the front face, rather than a camera-facing floating sign.
    scene.extraDraws.push_back(
        {quad,
         instance(f.point({0, 1.25f, -1.25f}), Vec3{3.0f, 2.0f, 1} * EnergyShield::CRATE_SCALE,
                  {1, 1, 1}, EnergyShield::yaw()),
         logo, 28});
    if (shield.installed) {
      auto p = EnergyShield::DOCK;
      scene.rod({p.x, CORE_HEIGHT + .15f, p.z}, f.point({0, .1f, .9f}), .045f, steel);
    }
    if (shield.droneVisible()) {
      Vec3 p = shield.dronePosition();
      constexpr float scale = EnergyShield::DRONE_SCALE;
      scene.box(p, Vec3{2.6f, .9f, 2} * scale, steel);
      scene.box(p + Vec3{0, .5f, 0} * scale, Vec3{1.7f, .12f, 1.5f} * scale, cyan, 0, 0, true);
      for (float x : {-1.65f, 1.65f})
        for (float z : {-1.3f, 1.3f}) {
          Vec3 rotor = p + Vec3{x, .15f, z} * scale;
          scene.rod(p, rotor, .12f * scale, dark);
          scene.tube(rotor, Vec3{1.05f, .12f, 1.05f} * scale, dark);
          scene.box(rotor + Vec3{0, .1f, 0} * scale, Vec3{1.5f, .045f, .1f} * scale, steel,
                    s.deckClock * 36 + x * z);
          if (!shield.installed)
            scene.rod(p + Vec3{x * .6f, -.4f, z * .6f} * scale,
                      f.point({(x < 0 ? -1.f : 1.f) * 1.3f, 4.1f, (z < 0 ? -1.f : 1.f) * .7f}),
                      .012f, steel);
        }
    }
    if (shield.online())
      scene.extraDraws.push_back(
          {&shell,
           instance({}, {1, 1, 1}, {shield.hp / shield.maxHp, shield.hitFlash / .35f, 0}, 0,
                    s.deckClock),
           nullptr, 27});
  }
};
} // namespace command
