#pragma once
#include "command_scene.hpp"
namespace command {
class JukeboxScene {
  std::vector<Instance> letters;
  float bass = 0, energy = 0, high = 0;
  void label(Vec3 origin, const std::string& text, float size, Color color, int wrap = 35) {
    int column = 0, row = 0;
    for (unsigned char ch : text) {
      if (ch == '\n' || column >= wrap) {
        ++row;
        column = 0;
        if (ch == '\n')
          continue;
      }
      letters.push_back(instance(origin + Vec3{column * size * .56f, -row * size * 1.7f, 0},
                                 {size, size * 1.5f, 1}, color, 0, float(ch)));
      ++column;
    }
  }

public:
  JukeboxScene() { letters.reserve(256); }
  void append(GameScene& scene, const Session& s, const RadioSnapshot& music, float dt) {
    letters.clear();
    if (!s.expedition.enabled || !s.expedition.miningStarted || s.mode == Mode::Selection)
      return;
    float lift = RadioState::deployment(s.buttonAge);
    if (lift <= 0)
      return;
    float gain = s.radio.volume * (music.playing ? 1 : 0);
    float response = 1 - std::exp(-std::min(dt, .1f) * 18);
    bass += (music.bass * gain - bass) * response;
    energy += (music.rms * gain - energy) * response;
    high += (music.treble * gain - high) * response;
    Vec3 base{Session::START_BUTTON.x, CORE_HEIGHT + .28f - (1 - lift) * 2.7f,
              Session::START_BUTTON.z};
    Color dark{.055f, .085f, .10f}, steel{.28f, .37f, .4f}, cyan{.15f, .9f, .85f},
        amber{1, .5f, .12f};
    auto box = [&](Vec3 p, Vec3 size, Color c, bool glow = false) {
      scene.box(base + p, size, c, 0, 0, glow);
    };
    box({0, .10f, 0}, {1.85f, .2f, 1.45f}, steel);
    box({0, 1.25f, 0}, {1.78f, 2.3f, 1.3f}, dark);
    box({0, 2.43f, 0}, {1.94f, .18f, 1.42f}, steel);
    for (float side : {-1.f, 1.f}) {
      box({side * .84f, 1.27f, -.69f}, {.06f, 2.15f, .06f}, cyan, true);
      box({side * .94f, .3f, 0}, {.12f, .28f, 1.15f}, steel);
      box({side * .9f, 2.2f, .2f}, {.18f, .45f, .5f}, steel);
      scene.rod(base + Vec3{side * .64f, 2.5f, .4f}, base + Vec3{side * .72f, 2.78f, .4f}, .035f,
                steel);
      Vec3 p = base + Vec3{side * .46f, .84f, -.71f};
      scene.rod(p, p + Vec3{0, 0, -.05f}, .35f, steel);
      scene.rod(p + Vec3{0, 0, -.055f}, p + Vec3{0, 0, -.08f - bass * .075f}, .29f,
                {.025f, .04f, .05f});
      scene.rod(p + Vec3{0, 0, -.09f - bass * .075f}, p + Vec3{0, 0, -.11f - bass * .075f},
                .11f + bass * .012f, cyan, true);
      for (int i = 0; i < 4; ++i)
        box({side * .46f, .45f + i * .24f, -.98f}, {.72f, .025f, .025f}, steel);
    }
    box({0, 1.98f, -.676f}, {1.57f, .72f, .055f}, steel);
    box({0, 1.98f, -.713f}, {1.45f, .61f, .025f}, {.008f, .035f, .04f}, true);
    box({0, 1.48f, -.71f}, {1.5f, .2f, .045f}, {.01f, .025f, .03f}, true);
    for (int i = 0; i < 10; ++i) {
      float value = (i < 3 ? bass : i < 7 ? energy : high) * (1.2f + (i % 3) * .3f);
      float height = .02f + std::min(.15f, value * .4f);
      box({-.65f + i * .144f, 1.4f + height / 2, -.746f}, {.085f, height, .015f},
          i % 3 ? cyan : amber, true);
    }
    for (float x : {-.56f, .56f}) {
      Vec3 knob = base + Vec3{x, .25f, -.73f};
      scene.rod(knob, knob + Vec3{0, 0, -.13f}, .21f, steel);
      float angle = x < 0 ? -.7f : -2.3f + s.radio.volume * 4.6f;
      scene.rod(knob + Vec3{0, 0, -.14f},
                knob + Vec3{std::sin(angle) * .075f, std::cos(angle) * .075f, -.14f}, .024f, amber,
                true);
    }
    if (lift > .8f) {
      label(base + Vec3{-.67f, 2.19f, -.733f}, RADIO_STATION, .088f, cyan);
      label(base + Vec3{-.67f, 1.98f, -.733f}, RADIO_TRACKS[music.track].title, .075f,
            {.8f, .95f, .9f}, 35);
      label(base + Vec3{-.16f, .28f, -.755f}, "FM 01", .065f, amber);
      label(base + Vec3{-.72f, .08f, -.755f}, "STATION", .054f, cyan);
      label(base + Vec3{.38f, .08f, -.755f}, "VOLUME", .054f, cyan);
    }
  }
  void draw(Renderer& r) { r.draw(r.quad, letters, r.font, 15); }
};
inline Rect radioButton(const GameUi& ui, int i) {
  return {ui.width / 2 - 330 + (i % 3) * 225.f, ui.height / 2 + 50 + (i / 3) * 70.f, 210, 52};
}
inline int radioClick(const GameUi& ui, float px, float py) {
  for (int i = 0; i < 6; ++i)
    if (radioButton(ui, i).contains(px / ui.scale, py / ui.scale))
      return i;
  return -1;
}
inline void radioAction(RadioState& state, const RadioSnapshot& music, int action) {
  if (action == 0)
    state.selectTrack(music.track - 1);
  if (action == 1)
    state.playing = !state.playing;
  if (action == 2)
    state.selectTrack(music.track + 1);
  if (action == 3)
    state.changeVolume(-.05f);
  if (action == 5)
    state.changeVolume(.05f);
}
inline void drawRadioPanel(GameUi& ui, Renderer& r, const Session& s, const RadioSnapshot& music) {
  ui.resize(r.width, r.height);
  float x = ui.width / 2, y = ui.height / 2;
  ui.rect(r, {0, 0, ui.width, ui.height}, {.005f, .02f, .025f}, .78f);
  ui.panel(r, {x - 365, y - 210, 730, 440});
  ui.text(r, x - 330, y - 180, RADIO_STATION, 29, GameUi::mint);
  ui.text(r, x - 330, y - 137, "STACJA 1 / 1    |    ORBITAL MINING RADIO", 14, GameUi::muted);
  ui.paragraph(r, x - 330, y - 85, RADIO_TRACKS[music.track].title, 48, 23, GameUi::white);
  ui.text(r, x - 330, y - 15,
          music.failed ? "Brak odtwarzania / sprawdz pliki muzyki"
                       : std::string(s.radio.playing ? "ODTWARZANIE  " : "PAUZA  ") +
                             number(music.position) + " / " + number(music.duration) + " s",
          14, GameUi::muted);
  const std::string labels[] = {"[LEWO] Poprzedni",
                                s.radio.playing ? "[SPACJA] Pauza" : "[SPACJA] Odtwarzaj",
                                "[PRAWO] Nastepny",
                                "[-] Ciszej",
                                "Stacja 1/1 | " + number(s.radio.volume * 100) + "%",
                                "[+] Glosniej"};
  for (int i = 0; i < 6; ++i) {
    auto b = radioButton(ui, i);
    ui.panel(r, b);
    ui.text(r, b.x + 12, b.y + 17, labels[i], 15, i == 4 ? GameUi::muted : GameUi::mint);
  }
  ui.text(r, x - 330, y + 205, "E / Esc: wroc    |    Walka wstrzymana, radio gra dalej", 13,
          GameUi::muted);
  r.flushText();
}
} // namespace command
