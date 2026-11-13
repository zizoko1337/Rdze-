#pragma once
#include "deck_instruments.hpp"

namespace command {
inline constexpr Color COFFEE_COLORS[] = {
    {.55f, .8f, .3f}, {1, .48f, .10f}, {.1f, .85f, 1}, {.7f, 1, .06f}, {.8f, .3f, 1}};
class CoffeeScene {
  std::array<Com<ID3D11ShaderResourceView>, 5> packets;
  Mesh ceramicCup;
  Mesh* liquidDisk = nullptr;
  Instance packet{};
  int tier = 0;
  bool visible = false;

public:
  explicit CoffeeScene(Renderer& r, const std::filesystem::path& assets) {
    for (int i = 0; i < 5; ++i)
      r.loadTexture(assets / ("coffee-tier-" + std::to_string(i + 1) + ".png"), packets[i]);
    liquidDisk = &r.cylinder;
    std::vector<Vertex> vertices;
    // Revolved ceramic profile includes the inside wall, rounded lip and solid floor.
    const float profile[][2] = {{0, .02f},      {.16f, .02f},   {.173f, .045f},
                                {.193f, .265f}, {.195f, .283f}, {.187f, .295f},
                                {.175f, .288f}, {.153f, .07f},  {0, .07f}};
    auto quad = [&](const Vertex(&v)[4]) {
      for (int index : {0, 1, 2, 0, 2, 3})
        vertices.push_back(v[index]);
    };
    for (int j = 0; j < int(std::size(profile)) - 1; ++j) {
      float r0 = profile[j][0], y0 = profile[j][1];
      float r1 = profile[j + 1][0], y1 = profile[j + 1][1];
      float dr = r1 - r0, dy = y1 - y0, norm = std::hypot(dr, dy);
      auto vertex = [&](float angle, float radius, float y) {
        float c = std::cos(angle), sn = std::sin(angle);
        return Vertex{c * radius, y, sn * radius, c * dy / norm, -dr / norm, sn * dy / norm, 0, 0};
      };
      for (int n = 0; n < 64; ++n) {
        float a = n * 2 * PI / 64, b = (n + 1) * 2 * PI / 64;
        quad({vertex(a, r0, y0), vertex(a, r1, y1), vertex(b, r1, y1), vertex(b, r0, y0)});
      }
    }
    auto handleVertex = [](float a, float b) {
      Vec3 normal{std::cos(a) * std::cos(b), std::sin(a) * std::cos(b), std::sin(b)};
      Vec3 p = Vec3{.185f + std::cos(a) * .13f, .16f + std::sin(a) * .105f, 0} + normal * .025f;
      return Vertex{p.x, p.y, p.z, normal.x, normal.y, normal.z, 0, 0};
    };
    for (int n = 0; n < 48; ++n)
      for (int m = 0; m < 12; ++m) {
        float a = -PI / 2 + n * PI / 48, aa = -PI / 2 + (n + 1) * PI / 48;
        float b = m * 2 * PI / 12, bb = (m + 1) * 2 * PI / 12;
        quad({handleVertex(a, b), handleVertex(aa, b), handleVertex(aa, bb), handleVertex(a, bb)});
      }
    r.uploadMesh(ceramicCup, vertices);
  }
  void append(GameScene& scene, const Session& s) {
    visible = s.expedition.enabled;
    if (!visible)
      return;
    tier = (s.coffee.active ? s.coffee.servingTier : s.expedition.coffeeTier) - 1;
    const auto& coffee = s.coffee;
    int machine = s.expedition.coffeeMachineTier;
    float time = s.deckClock;
    Color dark{.035f, .065f, .085f}, steel{.32f, .42f, .46f}, copper{.60f, .26f, .10f},
        ivory{.82f, .87f, .82f}, cyan{.08f, .9f, .85f}, accent = COFFEE_COLORS[tier];
    InstrumentFrame f{scene, Session::COFFEE_MACHINE, Session::coffeeYaw()};
    // Freestanding industrial espresso station, with an open brewing bay.
    for (float x : {-1.02f, 1.02f}) {
      f.box({x, .08f, 0}, {.38f, .16f, 1.35f}, dark);
      f.box({x, .47f, .15f}, {.18f, .8f, .7f}, steel);
    }
    f.box({0, .87f, -.02f}, {2.65f, .18f, 1.55f}, steel);
    f.box({0, 1.52f, .42f}, {2.4f, 1.15f, .44f}, dark);
    f.box({0, 2.06f, 0}, {2.5f, .67f, 1.25f}, dark);
    f.box({0, 2.44f, .02f}, {2.65f, .13f, 1.4f}, steel);
    f.box({0, 2.52f, .02f}, {2.5f, .035f, 1.28f}, cyan, true);
    for (float x : {-1.18f, 1.18f}) {
      f.box({x, 1.37f, .02f}, {.14f, .97f, 1.25f}, steel);
      f.box({x, 1.7f, -.64f}, {.035f, 1.32f, .025f}, cyan, true);
    }
    f.box({-.33f, .99f, -.56f}, {1.2f, .06f, .77f}, dark);
    for (int i = 0; i < 12; ++i)
      f.box({-.89f + i * .10f, 1.027f, -.56f}, {.025f, .018f, .64f}, steel);
    f.text({-.93f, .72f, -.82f}, "ORBITAL / COFFEE DIVISION", .072f, cyan);
    for (int i = 0; i < 7; ++i)
      f.box({-.9f + i * .30f, .88f, -.81f}, {.15f, .055f, .015f}, i % 2 ? copper : dark, true);

    float pressure = coffee.pouring() ? .68f + .13f * std::sin(time * 27) : .24f;
    f.gauge({-.85f, 2.1f, -.65f}, .26f, pressure, copper);
    f.text({-1.05f, 1.81f, -.69f}, "BAR / 2137", .06f, ivory);
    f.box({.4f, 2.08f, -.65f}, {1.64f, .57f, .10f}, steel);
    f.box({.4f, 2.08f, -.714f}, {1.5f, .47f, .025f}, {.006f, .025f, .025f}, true);
    const char* rows[] = {"TERRAN ARABICA", "FRONTIER ROAST", "LUNA NITRO", "REACTOR BLEND",
                          "PURE CAFFEINE"};
    f.text({-.28f, 2.24f, -.735f}, rows[tier], .105f, accent);
    f.text({-.28f, 2.06f, -.735f},
           tier == 4 ? "STAR DUST / TIER V"
                     : "TIER " + std::to_string(tier + 1) + " / " +
                           std::to_string(COFFEE_MINUTES[tier]) + " MIN",
           .074f, ivory);
    f.text({-.28f, 1.92f, -.735f},
           coffee.drinking()  ? "SMACZNEGO, OPERATORZE"
           : coffee.pouring() ? "EKSTRAKCJA KOFEINY..."
                              : "GOTOWY / NACISNIJ [E]",
           .062f, cyan);
    for (int i = 0; i < 14; ++i)
      f.box({.4f, 1.86f + i * .032f, -.732f}, {1.47f, .005f, .002f}, {.012f, .05f, .045f}, true);
    f.box({.82f, 1.37f, -.25f}, {.67f, .74f, .5f}, dark);
    packet = instance(f.point({.82f, 1.4f, -.512f}), {.80f, .80f, 1}, {1, 1, 1}, f.yaw);
    f.text({.51f, 1.02f, -.55f}, "ZASOBNIK", .065f, accent);

    // Upgrade II adds a second pressure circuit, gauge and dispensing tap.
    if (machine >= 2) {
      f.gauge({.86f, 2.71f, -.14f}, .21f, pressure * .92f, cyan);
      f.rod({.86f, 2.54f, .02f}, {.86f, 2.71f, .02f}, .09f, steel);
      f.rod({-1.31f, 1.2f, .2f}, {-1.31f, 2.68f, .2f}, .10f, copper);
      f.rod({-1.31f, 2.68f, .2f}, {-.28f, 2.68f, .2f}, .10f, copper);
      f.rod({-.28f, 2.68f, .2f}, {-.28f, 2.4f, .2f}, .10f, copper);
      for (float y : {1.3f, 1.8f, 2.3f})
        f.box({-1.31f, y, .2f}, {.25f, .06f, .26f}, steel);
    }
    if (machine == 3) {
      // Rotating six-barrel coffee injector, supplied by a luminous pressure reactor.
      f.rod({-.32f, 1.72f, -.57f}, {-.32f, 1.84f, -.57f}, .27f, steel);
      f.rod({-.32f, 1.68f, -.57f}, {-.32f, 1.72f, -.57f}, .30f, copper);
      float spin = coffee.pouring() ? time * 45 : time * .9f;
      for (int n = 0; n < 6; ++n) {
        float a = spin + n * PI / 3;
        Vec3 top{-.32f + std::cos(a) * .16f, 1.72f, -.57f + std::sin(a) * .16f};
        f.rod(top, top - Vec3{0, .22f, 0}, .055f, steel);
        f.rod(top - Vec3{0, .21f, 0}, top - Vec3{0, .235f, 0}, .037f, dark);
        if (coffee.pouring())
          f.rod(top - Vec3{0, .235f, 0},
                {-.32f + std::cos(a) * .09f, 1.12f + coffee.cupFill() * .18f,
                 -.57f + std::sin(a) * .09f},
                .018f, {.24f, .085f, .018f});
      }
      for (int n = 0; n < 4; ++n) {
        float a = n * PI / 2;
        float x = .05f + std::cos(a) * .18f, z = .25f + std::sin(a) * .18f;
        f.rod({x, 2.54f, z}, {x, 3.03f, z}, .025f, steel);
      }
      f.rod({.05f, 2.57f, .25f}, {.05f, 3.0f, .25f}, .13f, accent, true);
      for (float y : {2.54f, 2.78f, 3.03f})
        f.rod({.05f, y, .25f}, {.05f, y + .045f, .25f}, .24f, steel);
      f.rod({.05f, 2.57f, .25f}, {-.6f, 2.57f, .25f}, .08f, copper);
      f.text({-.75f, 2.45f, -.72f}, "CAFFEINE GATLING", .075f, {1, .7f, .2f});
    } else {
      for (int n = 0; n < machine; ++n) {
        float x = -.32f + (machine == 2 ? (n ? .12f : -.12f) : 0);
        f.rod({x, 1.98f, -.36f}, {x, 1.98f, -.57f}, .10f, copper);
        f.rod({x, 1.98f, -.57f}, {x, 1.64f, -.57f}, .075f, steel);
        f.rod({x, 1.65f, -.57f}, {x, 1.62f, -.57f}, .055f, dark);
        if (coffee.pouring())
          f.rod({x, 1.61f, -.57f}, {x, 1.12f + coffee.cupFill() * .18f, -.57f}, .022f,
                {.24f, .085f, .018f});
      }
    }
    // A genuinely hollow cup: ceramic walls, visible rising liquid, rim and handle.
    Vec3 resting = f.point({-.32f, 1.045f, -.57f});
    Vec3 forward = s.forward(), right = normalize(cross({0, 1, 0}, forward)),
         up = cross(forward, right);
    Vec3 mouth = s.eye() + forward * .65f - up * .33f;
    float lift = coffee.cupLift();
    Vec3 cup = resting * (1 - lift) + mouth * lift;
    float tilt = -coffee.cupTilt();
    auto cp = [&](Vec3 p) {
      float y = p.y * std::cos(tilt) - p.z * std::sin(tilt);
      float z = p.y * std::sin(tilt) + p.z * std::cos(tilt);
      return cup + Vec3{p.x * std::cos(f.yaw) + z * std::sin(f.yaw), y,
                        -p.x * std::sin(f.yaw) + z * std::cos(f.yaw)};
    };
    scene.extraOpaque.push_back({&ceramicCup, instance(cup, {1, 1, 1}, ivory, f.yaw, tilt), true});
    float fill = coffee.cupFill();
    if (fill > .001f) {
      float y = .075f + fill * .19f;
      float radius = .153f + (y - .07f) / .218f * .022f;
      scene.extraOpaque.push_back(
          {liquidDisk,
           instance(cp({0, y, 0}), {radius, .004f, radius}, {.10f, .033f, .006f}, f.yaw, tilt),
           true});
      for (int n = 0; n < 7; ++n) {
        float a = n * .9f + time * .7f;
        scene.extraOpaque.push_back(
            {liquidDisk,
             instance(cp({std::sin(a) * .12f, y + .003f, std::cos(a) * .12f}),
                      {.014f, .002f, .009f}, {.46f, .23f, .06f}, f.yaw, tilt),
             true});
      }
      for (int n = 0; n < 5; ++n) {
        float p = std::fmod(time * .8f + n * .2f, 1.f);
        scene.smoke.push_back(instance(cp({std::sin(time * 2 + n) * .06f, .32f + p * .35f, 0}),
                                       {.09f + p * .13f, .14f + p * .16f, 1}, {.8f, .85f, .85f}, 0,
                                       0, (1 - p) * .15f));
      }
    }
  }
  void draw(Renderer& r) {
    if (visible)
      r.draw(r.quad, &packet, 1, packets[tier], 22);
  }
};
} // namespace command
