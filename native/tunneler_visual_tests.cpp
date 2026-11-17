#include "bestiary.hpp"
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
        SDL_CreateWindow("Tunneler visual check", 1600, 900, SDL_WINDOW_HIDDEN), SDL_DestroyWindow);
    if (!window)
      throw std::runtime_error(SDL_GetError());
    auto assets = std::filesystem::current_path() / "public" / "assets";
    auto output = std::filesystem::current_path() / "artifacts" / "command" / "tunneler";
    std::filesystem::create_directories(output);
    Renderer renderer(window.get(), 1600, 900, assets);
    GameScene scene(renderer, assets);
    auto session = std::make_unique<Session>();
    auto& s = *session;
    s.start(Gun::Pulse);
    s.world.autoWaves = false;
    for (int id : s.world.active)
      s.world.foes[id].hp = 0;
    s.world.step(.01f);
    int id = s.world.spawn(Tunneler, 0, 190);
    ViewState view;
    view.building = false;
    bool headAbove = false, headBelow = false, mixedBody = true, matchingBatches = true;
    for (int frame = 0; frame < 6; ++frame) {
      if (frame > 0)
        for (int n = 0; n < 54; ++n)
          s.world.step(1.f / 60);
      if (frame == 5)
        s.world.hurt(id, s.world.foes[id].maxHp * .3f, Venom);
      const auto& body = *s.world.tunneler(id);
      int exposed = 0;
      Vec3 centre = s.world.headPoint(id);
      for (int part = 0; part < body.SEGMENTS; ++part) {
        centre = centre + body.segments[part];
        exposed += s.world.partExposed(id, part);
      }
      centre = centre * (1.f / (body.SEGMENTS + 1));
      centre.y = 4;
      bool head = s.world.partExposed(id, -1);
      headAbove |= head;
      headBelow |= !head;
      mixedBody &= exposed > 0 && exposed < body.SEGMENTS;
      Camera camera;
      camera.eye = centre + Vec3{65, 135, -150};
      camera.target = centre;
      camera.farClip = 5000;
      if (frame == 0) {
        auto p = s.world.headPoint(id);
        camera.eye = p + Vec3{20, 20, -40};
        camera.target = p;
      }
      renderer.begin(camera, s.world.time, -1);
      scene.prepare(s.world, view, true);
      matchingBatches &= scene.tunnelerSegments.size() == size_t(exposed) &&
                         scene.enemies[Tunneler].size() == size_t(head);
      scene.render(renderer);
      renderer.end();
      renderer.screenshot(output / ("tunneler-" + std::to_string(frame) + ".png"));
      renderer.present(false);
    }
    CommandUi ui;
    ui.resize(1600, 900);
    Bestiary menu;
    auto card = Bestiary::card(ui, Tunneler);
    bool fits = card.x >= 0 && card.y >= 0 && card.x + card.w <= ui.width && card.y + card.h <= 800;
    Camera camera;
    renderer.begin(camera, s.world.time, -1);
    menu.selected = Tunneler;
    menu.draw(ui, renderer, scene, s);
    renderer.end();
    renderer.screenshot(output / "bestiary.png");
    renderer.present(false);
    bool pass = headAbove && headBelow && mixedBody && matchingBatches && fits &&
                s.world.tunneler(id)->reversals == 1;
    std::ofstream report(output / "result.json");
    report << std::boolalpha << "{\n  \"passed\": " << pass
           << ",\n  \"head_emerges_and_dives\": " << (headAbove && headBelow)
           << ",\n  \"mixed_above_below_body\": " << mixedBody
           << ",\n  \"only_exposed_parts_rendered\": " << matchingBatches
           << ",\n  \"bestiary_card_fits\": " << fits << "\n}\n";
    std::cout << "TUNNELER VISUAL " << (pass ? "PASS" : "FAIL") << '\n';
    return pass ? 0 : 1;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
