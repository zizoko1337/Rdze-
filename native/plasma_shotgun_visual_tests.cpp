#include "plasma_shotgun_scene.hpp"
#include "command_scene.hpp"
#include "manual_sound.hpp"
#include <SDL3/SDL_main.h>
#include <fstream>
#include <iostream>
#include <memory>
using namespace command;
int main(int, char**) {
  try {
    check(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "COM");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
      throw std::runtime_error(SDL_GetError());
    struct Runtime {
      ~Runtime() {
        SDL_Quit();
        CoUninitialize();
      }
    } runtime;
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("Plasma shotgun", 1600, 900, SDL_WINDOW_HIDDEN), SDL_DestroyWindow);
    if (!window)
      throw std::runtime_error(SDL_GetError());
    auto assets = std::filesystem::current_path() / "public" / "assets";
    auto output = std::filesystem::current_path() / "artifacts" / "command" / "plasma-shotgun";
    std::filesystem::create_directories(output);
    Renderer r(window.get(), 1600, 900, assets);
    GameScene scene(r, assets);
    PlasmaShotgunScene model(r, assets);
    CommandRoof roof(r);
    auto session = std::make_unique<Session>();
    auto& s = *session;
    s.mode = Mode::Mounted;
    s.module.kind = Gun::Shotgun;
    s.lift = s.hatch = 1;
    s.pitch = s.gunPitch = -.08f;
    s.spawnTimer = 100000;
    s.world.autoWaves = false;
    ViewState view;
    view.building = false;
    CommandUi ui;
    ui.resize(1600, 900);
    auto render = [&](std::string file, bool firstPerson = false, bool withRoof = false) {
      Camera camera;
      camera.farClip = 5000;
      if (firstPerson) {
        camera.eye = s.eye();
        camera.target = camera.eye + s.forward() * 100;
      } else if (withRoof) {
        camera.eye = {5.4f, CORE_HEIGHT + 2.4f, -5.8f};
        camera.target = {0, CORE_HEIGHT + 3, -1.6f};
      } else {
        camera.eye = s.gunPoint({5.8f, 3.2f, -4.8f});
        camera.target = s.gunPoint({0, -.6f, 1.5f});
      }
      r.begin(camera, s.world.time, -1);
      scene.prepare(s.world, view, true);
      buildCommandDeck(scene, s);
      model.append(scene, s);
      if (withRoof)
        roof.append(scene, s);
      scene.render(r);
      if (firstPerson)
        drawManualSight(ui, r, s);
      r.flushText();
      r.end();
      r.screenshot(output / file);
      r.present(false);
    };
    render("01-ready.png");
    render("02-operator-view.png", true);
    s.fire();
    bool first = s.manualProjectiles.size() == 10 && s.shotgun.rounds == 1 && !s.shotgun.reloading;
    render("03-first-shot.png");
    s.cooldown = 0;
    s.shotgun.triggerLatched = false;
    s.fire();
    bool second = s.manualProjectiles.size() == 20 && s.shotgun.reloading;
    s.pitch = s.gunPitch = -.08f;
    s.recoil = s.shotFlash = 0;
    s.manualProjectiles.clear();
    constexpr float stages[] = {.04f, .12f,  .20f, .265f, .325f, .395f, .465f, .52f,
                                .60f, .685f, .72f, .78f,  .90f,  .94f,  .98f};
    int frame = 4;
    for (float progress : stages) {
      s.shotgun.age = s.shotgun.duration * progress;
      render(std::to_string(frame++) + "-reload.png");
      render("operator-" + std::to_string(frame - 1) + ".png", true);
    }
    s.shotgun.age = s.shotgun.duration * .60f;
    render("12-insertion-operator.png", true);
    s.shotgun.step(30);
    render("13-reloaded.png");
    s.mode = Mode::Walking;
    s.lift = s.hatch = 0;
    s.gunPitch = -.2f;
    render("19-base-roof-clearance.png", false, true);
    auto card = ui.choice(int(Gun::Shotgun));
    bool fits =
        card.y + card.h < ui.launch().y &&
        ui.selectionClick((card.x + 10) * ui.scale, (card.y + 10) * ui.scale) == int(Gun::Shotgun);
    s.mode = Mode::Selection;
    Camera menu;
    r.begin(menu, 0, -1);
    ui.drawCommand(r, s, 60, false, false, false);
    r.end();
    r.screenshot(output / "14-selection.png");
    r.present(false);
    ManualSound synth;
    ManualAudioState state;
    state.active = true;
    state.gun = Gun::Shotgun;
    synth.set(state);
    state.shots = 1;
    synth.set(state);
    std::ofstream wav(output / "plasma-shotgun.wav", std::ios::binary);
    auto u16 = [&](uint16_t v) { wav.write(reinterpret_cast<const char*>(&v), 2); };
    auto u32 = [&](uint32_t v) { wav.write(reinterpret_cast<const char*>(&v), 4); };
    wav.write("RIFF", 4);
    u32(36 + 96000);
    wav.write("WAVEfmt ", 8);
    u32(16);
    u16(1);
    u16(1);
    u32(48000);
    u32(96000);
    u16(2);
    u16(16);
    wav.write("data", 4);
    u32(96000);
    for (int n = 0; n < 48000; ++n) {
      int16_t sample = int16_t(synth.sample() * 32767);
      wav.write(reinterpret_cast<const char*>(&sample), 2);
    }
    bool pass = first && second && fits && s.shotgun.rounds == 2;
    std::ofstream report(output / "result.json");
    report << std::boolalpha << "{\n  \"passed\": " << pass
           << ",\n  \"two_separate_shots\": " << (first && second)
           << ",\n  \"selection_fits\": " << fits << ",\n  \"reload_frames\": 15\n}\n";
    std::cout << "PLASMA SHOTGUN VISUAL " << (pass ? "PASS" : "FAIL") << '\n';
    return pass ? 0 : 1;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
