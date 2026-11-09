#pragma once
#include "game_ui.hpp"
#include "command_game.hpp"

namespace command {
struct InstrumentFrame {
  GameScene& scene;
  Vec3 origin;
  float yaw;
  Vec3 point(Vec3 p) const {
    return origin + Vec3{p.x * std::cos(yaw) + p.z * std::sin(yaw), p.y,
                         -p.x * std::sin(yaw) + p.z * std::cos(yaw)};
  }
  void box(Vec3 p, Vec3 size, Color c, bool glow = false, float pitch = 0) const {
    scene.box(point(p), size, c, yaw, pitch, glow);
  }
  void rod(Vec3 a, Vec3 b, float radius, Color c, bool glow = false) const {
    scene.rod(point(a), point(b), radius, c, glow);
  }
  // Existing font atlas, a single batch for all instruments, no render-to-texture camera.
  void text(Vec3 p, const std::string& text, float size, Color color) const {
    std::wstring wide(text.size(), 0);
    int n = MultiByteToWideChar(CP_UTF8, 0, text.data(), int(text.size()), wide.data(),
                                int(wide.size()));
    const std::wstring polish = L"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ";
    Vec3 start = p;
    for (int i = 0; i < n; ++i) {
      wchar_t ch = wide[i];
      if (ch == '\n') {
        p.x = start.x;
        p.y -= size * 1.7f;
        continue;
      }
      int glyph = int(ch);
      if (ch < 32 || ch >= 128) {
        size_t k = polish.find(ch);
        glyph = k == std::wstring::npos ? '?' : 128 + int(k);
      }
      scene.worldLetters.push_back(
          instance(point(p), {size, size * 1.5f, 1}, color, yaw, float(glyph)));
      p.x += size * .56f;
    }
  }
  void meter(Vec3 p, float width, float fraction, Color color) const {
    box(p, {width, .09f, .018f}, {.025f, .07f, .075f}, true);
    float w = width * std::clamp(fraction, 0.f, 1.f);
    if (w > .001f)
      box(p + Vec3{(w - width) / 2, 0, -.014f}, {w, .054f, .012f}, color, true);
  }
  void gauge(Vec3 p, float radius, float value, Color color) const {
    Color steel{.4f, .5f, .52f};
    rod(p, p + Vec3{0, 0, -.06f}, radius, steel);
    rod(p + Vec3{0, 0, -.065f}, p + Vec3{0, 0, -.08f}, radius * .85f, {.012f, .028f, .03f});
    for (int i = 0; i <= 10; ++i) {
      float a = -2.3f + i * .46f;
      rod(p + Vec3{std::sin(a) * radius * .65f, std::cos(a) * radius * .65f, -.095f},
          p + Vec3{std::sin(a) * radius * .79f, std::cos(a) * radius * .79f, -.095f}, .012f,
          i > 7 ? Color{1, .35f, .12f} : steel, true);
    }
    float a = -2.3f + std::clamp(value, 0.f, 1.f) * 4.6f;
    rod(p + Vec3{0, 0, -.11f},
        p + Vec3{std::sin(a) * radius * .61f, std::cos(a) * radius * .61f, -.11f}, .019f, color,
        true);
    rod(p + Vec3{0, 0, -.11f}, p + Vec3{0, 0, -.13f}, .04f, steel);
  }
};

inline float consoleYaw(float side) {
  return std::atan2(side * 6.8f, -3.5f);
}
inline void appendCommandConsoles(GameScene& scene, const Session& s) {
  const Color dark{.055f, .09f, .11f}, steel{.32f, .41f, .44f}, mint{.2f, 1, .5f},
      cyan{.25f, .72f, 1}, amber{1, .58f, .13f};
  for (float side : {-1.f, 1.f}) {
    InstrumentFrame f{scene, {side * 6.8f, CORE_HEIGHT + .2f, -3.5f}, consoleYaw(side)};
    bool upgrades = side < 0;
    int count = upgrades ? int(s.expedition.rewards.size()) : s.expedition.servicePoints;
    Color accent = upgrades ? mint : cyan;
    if (upgrades) {
      // Pale research station: twin feet, shallow keyboard, wide suspended monitor.
      for (float x : {-1.f, 1.f}) {
        f.box({x, .12f, 0}, {.42f, .2f, 1.5f}, steel);
        f.box({x, .65f, .18f}, {.22f, 1, .65f}, {.56f, .63f, .62f});
      }
      f.box({0, 1.09f, -.12f}, {2.85f, .18f, 1.55f}, {.52f, .62f, .6f});
      f.box({0, 1.24f, .35f}, {2.95f, .14f, .5f}, dark);
      f.box({0, 1.78f, .4f}, {.32f, 1.2f, .26f}, steel);
      f.box({0, 2.04f, .28f}, {2.8f, 1.4f, .22f}, steel);
      f.box({0, 2.04f, .143f}, {2.54f, 1.17f, .04f}, {.006f, .025f, .027f}, true);
      for (float x : {-1.36f, 1.36f})
        f.box({x, 2.04f, .13f}, {.035f, .9f, .02f}, mint, true);
      f.text({-1.13f, 2.49f, .11f}, "RDZEŃ / ULEPSZENIA", .115f, mint);
      f.text({-1.13f, 2.16f, .11f}, "NAGRODY: " + number(count), .15f, {.8f, 1, .88f});
      f.text({-1.13f, 1.88f, .11f},
             count > 0 ? "WYBIERZ MODYFIKACJĘ [E]" : "OCZEKIWANIE NA WYPŁATĘ", .095f,
             count > 0 ? mint : steel);
      f.text({-1.13f, 1.63f, .11f}, "NANOBOTY " + number(s.expedition.regeneration, 1) + " HP/s",
             .09f, mint);
      // Softly breathing green beacon: metal collar, luminous glass, inner emitter and halo.
      float strength =
          count > 0 ? .38f + .62f * std::pow(.5f + .5f * std::sin(s.deckClock * 5), 2) : .025f;
      Vec3 lamp{1.03f, 1.4f, -.48f};
      f.rod(lamp, lamp + Vec3{0, 0, -.08f}, .19f, steel);
      Color light{.08f * strength, strength, .3f * strength};
      f.rod(lamp + Vec3{0, 0, -.085f}, lamp + Vec3{0, 0, -.15f}, .145f, light, true);
      f.rod(lamp + Vec3{0, 0, -.16f}, lamp + Vec3{0, 0, -.18f}, .07f,
            {.45f * strength, strength, .6f * strength}, true);
      if (count > 0)
        scene.hot.push_back(instance(f.point(lamp + Vec3{0, 0, -.16f}), {.8f, .8f, 1}, light, 0, 0,
                                     .15f * strength));
    } else {
      // Armored service rack: tall side cabinet, inset screen and amber guarded switches.
      f.box({0, .64f, .08f}, {2.8f, 1.2f, 1.45f}, dark);
      f.box({0, .1f, .05f}, {3, .2f, 1.65f}, steel);
      f.box({0, 1.29f, -.18f}, {2.95f, .19f, 1.7f}, steel);
      f.box({-.3f, 1.98f, .25f}, {2.05f, 1.32f, .32f}, {.12f, .21f, .29f});
      f.box({-.3f, 1.98f, .064f}, {1.83f, 1.09f, .04f}, {.008f, .018f, .035f}, true);
      f.box({1.12f, 1.97f, .28f}, {.48f, 1.48f, .55f}, dark);
      for (int i = 0; i < 3; ++i) {
        f.box({1.12f, 1.56f + i * .4f, -.03f}, {.3f, .23f, .1f}, steel);
        f.rod({1.05f, 1.56f + i * .4f, -.1f}, {1.16f, 1.61f + i * .4f, -.14f}, .035f, amber, true);
      }
      f.text({-1.1f, 2.38f, .03f}, "ORBITAL / USŁUGI", .105f, cyan);
      f.text({-1.1f, 2.09f, .03f}, "PUNKTY: " + number(count), .13f, {.7f, .85f, 1});
      f.text({-1.1f, 1.83f, .03f}, "01 BRIMSTONE\n02 IMPULS\n03 PRZECIĄŻENIE", .09f, cyan);
      f.box({0, 1.43f, -.64f}, {2.6f, .16f, .06f}, dark);
      f.text({-1.12f, 1.45f, -.68f}, count > 0 ? "WSPARCIE GOTOWE [E]" : "BRAK PUNKTÓW USŁUG", .1f,
             count > 0 ? amber : steel);
      for (int i = 0; i < 5; ++i)
        f.box({-.9f + i * .43f, .7f, -.66f}, {.28f, .035f, .035f}, cyan, true);
    }
    // Real raised keys, shared only on the control deck; housings and screens differ.
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 8; ++col)
        f.box({-.93f + col * .24f, upgrades ? 1.21f : 1.41f, -.73f + row * .15f},
              {.17f, .035f, .095f}, col == 7 ? accent : Color{.12f, .17f, .2f});
    for (int i = 0; i < 5; ++i)
      f.box({-1.2f + i * .12f, .48f, .77f}, {.05f, .32f, .025f}, dark);
  }
}

inline void appendUraniumPump(GameScene& scene, const Session& s) {
  if (!s.expedition.enabled || s.mode == Mode::Selection)
    return;
  InstrumentFrame f{scene, Session::URANIUM_PUMP, PI};
  Color dark{.07f, .12f, .14f}, steel{.39f, .49f, .5f}, green{.38f, 1, .3f}, amber{1, .62f, .16f};
  float clock = s.expedition.miningStarted
                    ? s.world.time * std::clamp(s.expedition.rate() * .8f, 1.f, 4.f)
                    : 0;
  float progress = s.expedition.uranium / Expedition::CAPACITY;
  float stability = s.expedition.hp / s.expedition.maxHp;
  f.box({0, .13f, 0}, {4.8f, .26f, 1.8f}, dark);
  f.box({0, .36f, .15f}, {4.45f, .2f, 1.4f}, steel);
  // Two opposed pistons, crank rods and compressing helical return springs.
  for (int side : {-1, 1}) {
    float x = side * 1.65f, phase = clock + (side > 0 ? PI : 0), stroke = .28f * std::sin(phase);
    f.rod({x, .48f, .2f}, {x, .94f, .2f}, .31f, dark);
    f.rod({x, .65f, .2f}, {x, 1.63f + stroke, .2f}, .10f, steel);
    f.rod({x, .98f, .2f}, {x, 1.03f, .2f}, .29f, steel);
    f.box({x, 1.67f + stroke, .2f}, {.76f, .16f, .68f}, steel);
    for (float guide : {-.38f, .38f})
      f.rod({x + guide, .98f, .2f}, {x + guide, 2.18f, .2f}, .035f, steel);
    f.rod({x, 2.12f, .2f}, {x, 2.48f, .2f}, .37f, dark);
    for (int k = 0; k < 5; ++k)
      f.rod({x, 2.12f + k * .075f, .2f}, {x, 2.14f + k * .075f, .2f}, .4f, steel);
    // Keep the entire coil above the cylinder, between a fixed seat and the moving cap.
    // The former smaller coil was mostly hidden inside the lower housing.
    Vec3 previous{x + .24f, 1.03f, .2f};
    for (int i = 1; i <= 48; ++i) {
      float t = i / 48.f, a = t * PI * 8;
      Vec3 p{x + std::cos(a) * .24f, 1.03f + t * (.56f + stroke), .2f + std::sin(a) * .24f};
      f.rod(previous, p, .024f, amber);
      previous = p;
    }
    // Front crank wheel with teeth and rotating spokes.
    Vec3 hub{x, .91f, -.55f};
    f.rod(hub, hub + Vec3{0, 0, -.1f}, .32f, steel);
    for (int i = 0; i < 14; ++i) {
      float a = phase + i * 2 * PI / 14;
      f.rod(hub + Vec3{std::cos(a) * .29f, std::sin(a) * .29f, -.12f},
            hub + Vec3{std::cos(a) * .39f, std::sin(a) * .39f, -.12f}, .055f, steel);
    }
    for (int i = 0; i < 4; ++i) {
      float a = phase + i * PI / 2;
      f.rod(hub + Vec3{0, 0, -.18f}, hub + Vec3{std::cos(a) * .27f, std::sin(a) * .27f, -.18f},
            .04f, dark);
    }
    f.rod(hub + Vec3{std::cos(phase) * .23f, std::sin(phase) * .23f, -.2f},
          {x, 1.67f + stroke, -.2f}, .055f, amber);
    f.rod({x, 2.45f, .2f}, {x * .55f, 2.45f, .2f}, .09f, steel);
    f.rod({x * .55f, 2.45f, .2f}, {x * .55f, .55f, .2f}, .09f, green, true);
  }
  // A central telemetry panel with actual extraction and core integrity, plus analog gauges.
  f.box({0, 1.51f, -.25f}, {2.35f, 2.08f, .38f}, dark);
  f.box({0, 1.79f, -.46f}, {2.13f, 1.21f, .035f}, {.006f, .026f, .028f}, true);
  f.text({-.94f, 2.29f, -.49f}, "POMPA URANU / U-2137", .105f, green);
  f.text({-.94f, 2.03f, -.49f}, "WYDOBYCIE " + number(progress * 100) + "%", .105f, green);
  f.meter({0, 1.84f, -.5f}, 1.87f, progress, green);
  Color stable = stability > .5f ? Color{.3f, .85f, 1} : Color{1, .3f, .1f};
  f.text({-.94f, 1.63f, -.49f}, "STABILNOŚĆ " + number(stability * 100) + "%", .105f, stable);
  f.meter({0, 1.44f, -.5f}, 1.87f, stability, stable);
  f.text({-.94f, 1.22f, -.49f},
         number(s.expedition.rate(), 1) + " U/s   +" + number(s.expedition.regeneration, 1) +
             " HP/s",
         .085f, steel);
  f.gauge({-.54f, .87f, -.49f}, .24f,
          s.expedition.miningStarted ? .45f + .09f * std::sin(clock) : 0, amber);
  f.gauge({.54f, .87f, -.49f}, .24f, stability, stable);
  f.text({-.85f, .52f, -.51f}, "CIŚNIENIE    INTEGRALNOŚĆ", .075f, steel);
  for (int i = 0; i < 4; ++i)
    f.box({-1.8f + i * 1.2f, .25f, -.91f}, {.65f, .07f, .025f}, amber, true);
}
inline void appendWarningLamps(GameScene& scene, const Session& s) {
  if (!s.expedition.enabled || s.mode == Mode::Selection)
    return;
  int level = s.coreWarningLevel();
  Color color = level == 0   ? Color{.12f, 1, .3f}
                : level == 1 ? Color{1, .75f, .08f}
                             : Color{1, .055f, .025f};
  float intensity = level == 2 ? .65f + .35f * std::sin(s.deckClock * 6) : .9f;
  color = {color.r * intensity, color.g * intensity, color.b * intensity};
  for (int i = 0; i < 8; ++i) {
    float a = i * PI / 4;
    Vec3 p{std::sin(a) * 11.5f, CORE_HEIGHT + 2.15f, std::cos(a) * 11.5f};
    scene.tube(p, {.28f, .12f, .28f}, {.12f, .17f, .2f});
    scene.tube(p + Vec3{0, .19f, 0}, {.19f, .3f, .19f}, color, 0, 0, true);
    scene.tube(p + Vec3{0, .37f, 0}, {.25f, .06f, .25f}, {.3f, .4f, .42f});
    scene.hot.push_back(instance(p + Vec3{0, .2f, 0}, {1.15f, 1.15f, 1}, color, 0, 0, .17f));
  }
}
} // namespace command
