#include "energy_shield_scene.hpp"
#include "command_scene.hpp"
#include <SDL3/SDL_main.h>
#include <fstream>
#include <iostream>
#include <memory>
using namespace command;
uint64_t frameHash(Renderer& r, bool aimingRegion = false) {
  Com<ID3D11Texture2D> back, staging;
  check(r.swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)back.put()), "Probe buffer");
  D3D11_TEXTURE2D_DESC desc{};
  back->GetDesc(&desc);
  desc.Usage = D3D11_USAGE_STAGING;
  desc.BindFlags = 0;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  desc.MiscFlags = 0;
  check(r.device->CreateTexture2D(&desc, nullptr, staging.put()), "Probe staging");
  r.context->CopyResource(staging, back);
  D3D11_MAPPED_SUBRESOURCE map{};
  check(r.context->Map(staging, 0, D3D11_MAP_READ, 0, &map), "Probe map");
  uint64_t hash = 1469598103934665603ull;
  UINT y0 = aimingRegion ? desc.Height / 4 : 0,
       y1 = aimingRegion ? desc.Height * 3 / 4 : desc.Height;
  UINT x0 = aimingRegion ? desc.Width / 4 : 0, x1 = aimingRegion ? desc.Width * 3 / 4 : desc.Width;
  for (UINT y = y0; y < y1; ++y) {
    auto* row = static_cast<const uint8_t*>(map.pData) + map.RowPitch * y;
    for (UINT x = x0 * 4; x < x1 * 4; ++x) {
      hash ^= row[x];
      hash *= 1099511628211ull;
    }
  }
  r.context->Unmap(staging, 0);
  return hash;
}
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
        SDL_CreateWindow("Energy shield test", 1600, 900, SDL_WINDOW_HIDDEN), SDL_DestroyWindow);
    if (!window)
      throw std::runtime_error(SDL_GetError());
    auto assets = std::filesystem::current_path() / "public" / "assets";
    auto output = std::filesystem::current_path() / "artifacts" / "command" / "energy-shield";
    std::filesystem::create_directories(output);
    Renderer r(window.get(), 1600, 900, assets);
    GameScene scene(r, assets);
    CommandRoof roof(r);
    EnergyShieldScene shieldScene(r, assets);
    auto session = std::make_unique<Session>(true, true);
    auto& s = *session;
    s.mode = Mode::Walking;
    s.spawnTimer = s.frontier.nextBoss = s.frontier.nextElite = s.frontier.nextHorde =
        s.frontier.mutations.nextMutation = 100000;
    if (!s.debugGrant(Upgrade::EnergyShield, Rarity::Epic))
      throw std::runtime_error("Unlock failed");
    auto tick = [&](float seconds) {
      for (int n = 0; n < int(std::round(seconds * 60)); ++n)
        s.step(1.f / 60);
    };
    ViewState view;
    view.building = false;
    Camera outside;
    outside.eye = {44, 31, -40};
    outside.target = {0, 11, 0};
    outside.farClip = 5000;
    Camera box;
    box.eye = {6.3f, CORE_HEIGHT + 1.72f, 2.2f};
    box.target = EnergyShield::DOCK + Vec3{0, .9f, 0};
    box.farClip = 5000;
    auto render = [&](const std::string& file, Camera camera, bool effect = true,
                      bool aimingRegion = false) {
      r.begin(camera, s.world.time, -1);
      scene.prepare(s.world, view, true);
      buildCommandDeck(scene, s);
      roof.append(scene, s);
      shieldScene.append(scene, s);
      if (!effect)
        scene.extraDraws.erase(std::remove_if(scene.extraDraws.begin(), scene.extraDraws.end(),
                                              [](const auto& draw) { return draw.mode == 27; }),
                               scene.extraDraws.end());
      scene.render(r);
      r.end();
      auto hash = frameHash(r, aimingRegion);
      if (!file.empty())
        r.screenshot(output / file);
      r.present(false);
      return hash;
    };
    tick(4);
    Camera incoming;
    incoming.eye = {75, 70, -70};
    incoming.target = {22, 32, 6};
    render("01-delivery.png", incoming);
    tick(3);
    render("02-lowering.png", outside);
    tick(1.1f);
    bool landed = s.expedition.shield.installed && !s.expedition.shield.online();
    render("11-antennas-start.png", box);
    tick(.6f);
    render("12-antennas-extending.png", box);
    tick(.8f);
    bool arcBeforeShield = s.expedition.shield.arcStrength() > 0 && !s.expedition.shield.online();
    render("13-arc-before-shield.png", box);
    tick(.8f);
    bool startupComplete =
        s.expedition.shield.online() && s.expedition.shield.antennaExtension() == 1;
    auto external = render("03-active.png", outside);
    bool effectVisible = external != render("", outside, false);
    render("04-generator.png", box);
    Camera side = box;
    side.eye = EnergyShield::DOCK + Vec3{.5f, 1.9f, -2.8f};
    side.target = EnergyShield::DOCK + Vec3{0, .9f, 0};
    auto moving = render("14-side-machinery.png", side);
    tick(.2f);
    bool animated = moving != render("15-side-machinery-moving.png", side);
    s.expedition.shield.absorb(720);
    render("05-damaged.png", outside);
    render("06-integrity.png", box);
    s.expedition.shield.absorb(280);
    render("07-rebuild.png", box);
    tick(60.1f);
    render("08-restored.png", outside);
    Camera inside;
    inside.eye = {0, CORE_HEIGHT + 12, 0};
    inside.target = {0, 2, 150};
    // Peripheral hexcells are intentionally visible now; the central aiming region stays clear.
    auto shielded = render("09-firing-view.png", inside, true, true);
    bool viewClear = shielded == render("", inside, false, true);
    Camera walking;
    walking.eye = {0, CORE_HEIGHT + 1.72f, -6};
    walking.target = {0, CORE_HEIGHT + 1.72f, 40};
    bool innerVisible = render("10-shield-inside.png", walking) != render("", walking, false);
    bool roofClear = true;
    for (int n = 0; n <= 1200; ++n) {
      EnergyShield route;
      route.deliveryAge = float(n) / 100;
      auto p = route.dronePosition();
      float radius = std::hypot(p.x, p.z);
      if (radius < Session::ROOF_RADIUS + .9f)
        roofClear &=
            p.y + .2f < Session::roofHeight(std::min(Session::ROOF_RADIUS, radius + .9f)) - .18f;
    }
    bool pass = landed && arcBeforeShield && startupComplete && animated && effectVisible &&
                innerVisible && viewClear && roofClear && s.expedition.shield.online() &&
                EnergyShieldScene::HEIGHT < Session::EAVE;
    std::ofstream report(output / "result.json");
    report << std::boolalpha << "{\n  \"passed\": " << pass
           << ",\n  \"delivery_installs\": " << landed
           << ",\n  \"arc_precedes_activation\": " << arcBeforeShield
           << ",\n  \"startup_complete\": " << startupComplete
           << ",\n  \"moving_machinery\": " << animated
           << ",\n  \"hex_effect_visible_outside\": " << effectVisible
           << ",\n  \"central_aiming_view_clear\": " << viewClear
           << ",\n  \"shield_visible_inside\": " << innerVisible
           << ",\n  \"drone_clears_roof\": " << roofClear << "\n}\n";
    std::cout << "ENERGY SHIELD VISUAL " << (pass ? "PASS" : "FAIL") << '\n';
    return pass ? 0 : 1;
  } catch (const std::exception& e) {
    std::cerr << "FAIL " << e.what() << '\n';
    return 1;
  }
}
