#include "command_scene.hpp"
#include <SDL3/SDL_main.h>
#include <fstream>
#include <iostream>
#include <memory>

using namespace command;
struct FrameProbe {
  Com<ID3D11Texture2D> copy;
  UINT width = 0, height = 0;
  std::vector<uint8_t> sampledRgb;
  float blankFraction(Renderer& r) {
    Com<ID3D11Texture2D> back;
    check(r.swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)back.put()), "Probe backbuffer");
    D3D11_TEXTURE2D_DESC d{};
    back->GetDesc(&d);
    if (!copy.p || width != d.Width || height != d.Height) {
      width = d.Width;
      height = d.Height;
      d.Usage = D3D11_USAGE_STAGING;
      d.BindFlags = 0;
      d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
      d.MiscFlags = 0;
      check(r.device->CreateTexture2D(&d, nullptr, copy.put()), "Probe staging texture");
    }
    r.context->CopyResource(copy, back);
    D3D11_MAPPED_SUBRESOURCE map{};
    check(r.context->Map(copy, 0, D3D11_MAP_READ, 0, &map), "Probe pixels");
    int blank = 0, sampled = 0;
    sampledRgb.clear();
    for (UINT y = height / 5; y < height * 4 / 5; y += 8) {
      auto* row = static_cast<const uint8_t*>(map.pData) + y * map.RowPitch;
      for (UINT x = width / 8; x < width * 7 / 8; x += 8) {
        auto* p = row + x * 4;
        sampledRgb.insert(sampledRgb.end(), p, p + 3);
        bool black = p[0] < 4 && p[1] < 4 && p[2] < 4;
        bool clearOnly = std::abs(int(p[0]) - 9) <= 1 && std::abs(int(p[1]) - 19) <= 1 &&
                         std::abs(int(p[2]) - 24) <= 1;
        blank += black || clearOnly;
        ++sampled;
      }
    }
    r.context->Unmap(copy, 0);
    return float(blank) / sampled;
  }
  double meanLuma() const {
    double sum = 0;
    for (size_t i = 0; i + 2 < sampledRgb.size(); i += 3)
      sum += (sampledRgb[i] + sampledRgb[i + 1] + sampledRgb[i + 2]) / 3.;
    return sum / (sampledRgb.size() / 3);
  }
  double meanDelta(const std::vector<uint8_t>& previous) const {
    if (previous.size() != sampledRgb.size())
      return 0;
    double sum = 0;
    for (size_t i = 0; i < sampledRgb.size(); ++i)
      sum += std::abs(int(sampledRgb[i]) - int(previous[i]));
    return sum / sampledRgb.size();
  }
};

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
        SDL_CreateWindow("RDZEN - kontrola migotania", 1280, 720, SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);
    if (!window)
      throw std::runtime_error(SDL_GetError());
    auto artifacts = std::filesystem::current_path() / "artifacts" / "render";
    std::filesystem::create_directories(artifacts);
    Renderer r(window.get(), 1280, 720, std::filesystem::current_path() / "public" / "assets");
    GameScene scene(r, std::filesystem::current_path() / "public" / "assets");
    CommandRoof roof(r);
    CommandUi ui;
    FrameProbe probe;
    // A subpixel camera shift should not cause distant ground grain to change abruptly.
    // Isolate terrain, excluding enemies, particles, UI and gun animation from the metric.
    std::vector<uint8_t> previousTerrain;
    double terrainJitter = 0;
    for (int frame = 0; frame < 32; ++frame) {
      Camera camera;
      camera.eye = {0, 30, 0};
      camera.target = {frame % 2 ? .0375f : 0.f, 0, 250};
      camera.farClip = 5000;
      r.begin(camera, 0, -1);
      auto sky = instance({0, 0, 0}, {1, 1, 1}, {1, 1, 1});
      auto floor = instance({0, -.05f, 0}, {6400, 1, 6400}, {.15f, .2f, .225f});
      r.draw(r.quad, &sky, 1, r.white, 11);
      r.draw(r.ground, &floor, 1, r.white, 3);
      r.end();
      probe.blankFraction(r);
      if (!previousTerrain.empty()) {
        double delta = 0;
        for (size_t i = 0; i < previousTerrain.size(); ++i)
          delta += std::abs(int(previousTerrain[i]) - int(probe.sampledRgb[i]));
        terrainJitter += delta / previousTerrain.size();
      }
      previousTerrain = probe.sampledRgb;
      if (frame == 0)
        r.screenshot(artifacts / "terrain.png");
      r.present(false);
    }
    terrainJitter /= 31;
    ViewState passive;
    passive.building = false;
    // Render real ground mark batches: visible heat, cooled scar, then exact restoration.
    double groundHeatDelta = 0, groundScarDelta = 0, groundExpiredDelta = 0;
    {
      Game groundFx(false);
      groundFx.autoWaves = false;
      Camera camera;
      camera.eye = {7, 12, 30};
      camera.target = {0, 0, 45};
      camera.farClip = 5000;
      std::vector<uint8_t> baseline;
      for (int phase = 0; phase < 4; ++phase) {
        if (phase == 1) {
          for (int n = 0; n < 12; ++n) {
            float x = float(n)-6;
            groundFx.groundMarks.stroke({x,0,45+std::sin(x*.5f)*2},
                                       {x+1,0,45+std::sin((x+1)*.5f)*2},.34f,4.5f,1.6f);
          }
          groundFx.groundMarks.spot({-5,0,40},.32f,3.5f,.35f);
          groundFx.groundMarks.spot({-3,0,40},.16f,2.5f);
          groundFx.groundMarks.spot({-1,0,40},.5f,4,.2f);
          groundFx.groundMarks.spot({4,0,46},2,6,.8f);
        } else if (phase == 2) groundFx.groundMarks.tick(2);
        else if (phase == 3) groundFx.groundMarks.tick(5);
        scene.prepare(groundFx,passive,true);
        r.begin(camera,0,-1);
        scene.render(r);
        r.end();
        probe.blankFraction(r);
        if (phase == 0) baseline = probe.sampledRgb;
        else if (phase == 1) groundHeatDelta = probe.meanDelta(baseline);
        else if (phase == 2) groundScarDelta = probe.meanDelta(baseline);
        else groundExpiredDelta = probe.meanDelta(baseline);
        const char* names[] = {"clean", "hot", "cooled", "expired"};
        r.screenshot(artifacts / (std::string("ground-")+names[phase]+".png"));
        r.present(false);
      }
    }
    // Reproduce the reported case: mounted gun, aimed down, rotating extremely slowly.
    int slowFrames = 0, slowBlackSpikes = 0;
    double slowMaxLumaJump = 0, slowMaxPixelDelta = 0;
    std::vector<uint8_t> previousSlow;
    double previousLuma = 0;
    for (int weapon = 0; weapon < 2; ++weapon) {
      Session slow;
      slow.module.kind = Gun(weapon);
      slow.mode = Mode::Mounted;
      slow.lift = slow.hatch = 1;
      slow.spawnTimer = 100000;
      slow.pitch = slow.gunPitch = -PI / 6; // 30 degrees below neutral.
      previousSlow.clear();
      previousLuma = 0;
      for (int frame = 0; frame < 600; ++frame) {
        slow.look(.001f, 0); // About 3.4 degrees per second at 60 Hz.
        slow.updateAim(1.f / 60);
        slow.step(1.f / 60, {0, 0, false, true});
        Camera camera;
        camera.eye = slow.eye();
        camera.target = camera.eye + slow.forward();
        applyManualCamera(camera, slow.feedback.pose);
        camera.farClip = 5000;
        scene.prepare(slow.world, passive, true);
        buildCommandDeck(scene, slow);
        roof.append(scene, slow);
        r.begin(camera, slow.world.time, -1);
        scene.render(r);
        r.end();
        float blank = probe.blankFraction(r);
        double luma = probe.meanLuma();
        if (frame > 0) {
          slowMaxLumaJump = std::max(slowMaxLumaJump, std::abs(luma - previousLuma));
          slowMaxPixelDelta = std::max(slowMaxPixelDelta, probe.meanDelta(previousSlow));
        }
        if (blank > .2 || (frame > 0 && luma < previousLuma - 20)) {
          ++slowBlackSpikes;
          if (slowBlackSpikes <= 3)
            r.screenshot(artifacts / ("slow-black-" + std::to_string(slowBlackSpikes) + ".png"));
        }
        if (frame == 0 || frame == 300 || frame == 599)
          r.screenshot(artifacts / ("slow-down-" + std::to_string(weapon) + "-" +
                                    std::to_string(frame) + ".png"));
        previousLuma = luma;
        previousSlow = probe.sampledRgb;
        ++slowFrames;
        r.present(false);
      }
    }
    int frames = 0, blankFrames = 0, resizeChecks = 0;
    float worst = 0;
    std::ofstream csv(artifacts / "frames.csv");
    csv << "weapon,frame,pitch,yaw,blank_fraction\n";
    for (int weapon = 0; weapon < 2; ++weapon) {
      Session s;
      s.start(Gun(weapon));
      s.mode = Mode::Mounted;
      s.lift = s.hatch = 1;
      for (int frame = 0; frame < 360; ++frame) {
        if (frame == 120)
          SDL_SetWindowFullscreen(window.get(), true);
        if (frame == 240)
          SDL_SetWindowFullscreen(window.get(), false);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
          if (event.type == SDL_EVENT_QUIT)
            throw std::runtime_error("Render test interrupted");
          if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            r.resize(event.window.data1, event.window.data2);
        }
        if (frame % 30 == 0) {
          auto* before = r.target.p;
          r.resize(r.width, r.height);
          if (before != r.target.p)
            throw std::runtime_error("Duplicate resize recreates backbuffer");
          ++resizeChecks;
        }
        float pitch = -.235f + .885f * std::sin(frame * .045f);
        s.look(.035f, pitch - s.pitch - s.pendingPitch);
        s.updateAim(1.f / 60);
        s.step(1.f / 60, {0, 0, false, true});
        Camera camera;
        camera.eye = s.eye();
        camera.target = camera.eye + s.forward();
        applyManualCamera(camera, s.feedback.pose);
        camera.farClip = 5000;
        if (!std::isfinite(camera.eye.x + camera.eye.y + camera.eye.z + s.yaw + s.pitch))
          throw std::runtime_error("Nonfinite camera transform");
        scene.prepare(s.world, passive, true);
        buildCommandDeck(scene, s);
        roof.append(scene, s);
        // Exercise the degenerate ribbon cases: camera on the beam axis and zero length.
        if (frame % 3 == 0) {
          scene.beams.push_back(lineInstance(
              camera.eye + s.forward() * 3, camera.eye + s.forward() * 300, {.1f, .4f, .5f}, .03f));
          scene.beams.push_back(lineInstance(camera.eye, camera.eye, {.1f, .4f, .5f}, .03f));
        }
        r.begin(camera, s.world.time, -1);
        scene.render(r);
        ui.drawCommand(r, s, 60, true, false, false);
        r.end();
        float fraction = probe.blankFraction(r);
        worst = std::max(worst, fraction);
        ++frames;
        csv << weapon << ',' << frame << ',' << s.pitch << ',' << s.yaw << ',' << fraction << '\n';
        if (fraction > .98f) {
          ++blankFrames;
          if (blankFrames <= 3)
            r.screenshot(artifacts / ("blank-" + std::to_string(blankFrames) + ".png"));
        }
        if (frame == 75 || frame == 175 || frame == 275)
          r.screenshot(artifacts / (std::to_string(weapon) + "-" + std::to_string(frame) + ".png"));
        r.present(frame < 180);
        r.completed.clear();
      }
    }
    bool passed =
        blankFrames == 0 && terrainJitter < .1 && slowBlackSpikes == 0 && slowMaxLumaJump < 10 &&
        groundHeatDelta > .05 && groundScarDelta > .01 && groundExpiredDelta < .001;
    std::ofstream out(artifacts / "result.json");
    out << "{\n  \"passed\": " << (passed ? "true" : "false")
        << ",\n  \"frames_checked\": " << frames << ",\n  \"blank_frames\": " << blankFrames
        << ",\n  \"worst_blank_fraction\": " << worst
        << ",\n  \"terrain_subpixel_delta\": " << terrainJitter
        << ",\n  \"ground_heat_delta\": " << groundHeatDelta
        << ",\n  \"ground_scar_delta\": " << groundScarDelta
        << ",\n  \"ground_expired_delta\": " << groundExpiredDelta
        << ",\n  \"slow_down_frames\": " << slowFrames
        << ",\n  \"slow_down_black_spikes\": " << slowBlackSpikes
        << ",\n  \"slow_down_max_luma_jump\": " << slowMaxLumaJump
        << ",\n  \"slow_down_max_pixel_delta\": " << slowMaxPixelDelta
        << ",\n  \"duplicate_resize_checks\": " << resizeChecks << "\n}\n";
    std::cout << "RENDER " << (passed ? "PASS" : "FAIL") << " frames=" << frames
              << " blank=" << blankFrames << " duplicate_resize=" << resizeChecks
              << " worst=" << worst << '\n';
    std::cout << "TERRAIN subpixel_delta=" << terrainJitter << '\n';
    std::cout << "SLOW_DOWN frames=" << slowFrames << " black_spikes=" << slowBlackSpikes
              << " max_luma_jump=" << slowMaxLumaJump << " max_pixel_delta=" << slowMaxPixelDelta
              << '\n';
    return passed ? 0 : 1;
  } catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
  }
}
