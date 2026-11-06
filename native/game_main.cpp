#include "game_ui.hpp"
#include "game_audio.hpp"
#include "command_app.hpp"
#include <SDL3/SDL_main.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>

using namespace arsenal;
using Clock = std::chrono::steady_clock;
double elapsed(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double>(b - a).count();
}
double percentile(std::vector<double> v, double p) {
  if (v.empty())
    return 0;
  std::sort(v.begin(), v.end());
  return v[std::min(v.size() - 1, size_t((v.size() - 1) * p))];
}
void keyEvent(SDL_Scancode key) {
  SDL_Event e{};
  e.type = SDL_EVENT_KEY_DOWN;
  e.key.scancode = key;
  e.key.repeat = false;
  SDL_PushEvent(&e);
}
void clickEvent(float x, float y) {
  SDL_Event e{};
  e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
  e.button.button = SDL_BUTTON_LEFT;
  e.button.x = x;
  e.button.y = y;
  SDL_PushEvent(&e);
}
Vec3 screenPoint(Vec3 p, Camera& camera, int width, int height) {
  auto m = camera.matrix(float(width) / height);
  float w = p.x * m.m[3] + p.y * m.m[7] + p.z * m.m[11] + m.m[15];
  float x = p.x * m.m[0] + p.y * m.m[4] + p.z * m.m[8] + m.m[12];
  float y = p.x * m.m[1] + p.y * m.m[5] + p.z * m.m[9] + m.m[13];
  return {(x / w + 1) * width * .5f, (1 - y / w) * height * .5f, 0};
}

int main(int argc, char** argv) {
  bool smoke = false, stress = false, arsenalMode = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--smoke-test")
      smoke = true;
    if (arg == "--stress-test")
      stress = true;
    if (arg == "--arsenal")
      arsenalMode = true;
  }
  if (!smoke && !stress && !arsenalMode)
    return command::run(argc, argv);
  const bool automated = smoke || stress;
  try {
    check(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "COM initialization");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
      throw std::runtime_error(SDL_GetError());
    struct Runtime {
      ~Runtime() {
        SDL_Quit();
        CoUninitialize();
      }
    } runtime;
    auto root = std::filesystem::current_path(), assets = root / "public" / "assets",
         artifacts = root / "artifacts" / "arsenal";
    std::filesystem::create_directories(artifacts);
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("RDZEŃ - Poligon arsenału", 1920, 1080, SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);
    if (!window)
      throw std::runtime_error(SDL_GetError());
    if (automated) {
      SDL_SetWindowBordered(window.get(), false);
      SDL_SetWindowPosition(window.get(), 0, 0);
    } else
      SDL_SetWindowFullscreen(window.get(), true);
    int width, height;
    SDL_GetWindowSizeInPixels(window.get(), &width, &height);
    Renderer renderer(window.get(), width, height, assets);
    GameScene scene(renderer, assets);
    GameUi ui;
    ui.resize(width, height);
    GameAudio audio;
    if (automated)
      audio.enabled = false;
    auto game = std::make_unique<Game>();
    ViewState view;
    Camera camera;
    view.camera(camera, float(width) / height);
    bool running = true, help = false, vsync = !automated, focused = true, minimized = false;
    float mouseX = width * .5f, mouseY = height * .5f;
    double accumulator = 0;
    auto last = Clock::now(), start = last;
    float shownFps = 60, shownGpu = 0;
    int frameCounter = 0;
    double displayTimer = 0, frameSum = 0;
    uint64_t cameraSwitches = 0, buildCount = 0;
    std::vector<double> frameTimes, gpuTimes, simTimes;
    frameTimes.reserve(20000);
    gpuTimes.reserve(20000);
    simTimes.reserve(3000);
    size_t minEnemies = MAX_FOES, maxEnemies = 0;
    float stressRefill = 0;
    bool shotInitial = false, shotTactical = false, shotFps = false, shotSmall = false,
         pausedTest = false, pausePass = false;
    int smokeStage = 0, placed = 0;
    float frozen = 0;
    double nextPlacement = 1.3;
    if (smoke) {
      for (auto& t : game->towers) {
        float a = std::atan2(t.x, t.z);
        int id = game->spawn(Brute, t.x + std::sin(a) * 23, t.z + std::cos(a) * 23);
        game->foes[id].hp = game->foes[id].maxHp = 1000000;
        game->foes[id].speed = 0;
      }
      game->spawn(Titan, -85, 115);
      game->spawn(Queen, 80, 150);
    }
    if (stress) {
      view.tactical = true;
      view.building = false;
      view.zoom = 235;
      game->horde(4900);
      for (int i = 0; i < 90; ++i) {
        float a = i * 2.39996f, r = 65 + std::sqrt(float(i)) * 11;
        game->addTower(Weapon(i % 9), std::sin(a) * r, std::cos(a) * r);
      }
      game->boss();
      game->boss();
    }
    auto switchView = [&]() {
      view.tactical = !view.tactical;
      view.placement = false;
      ++cameraSwitches;
      SDL_SetWindowRelativeMouseMode(window.get(),
                                     !view.tactical && !game->paused && !help && !automated);
    };
    auto togglePause = [&]() {
      game->paused = !game->paused;
      if (game->paused)
        SDL_SetWindowRelativeMouseMode(window.get(), false);
    };
    auto action = [&](int code) {
      if (code >= 0 && code < WeaponCount) {
        view.selected = Weapon(code);
        view.building = true;
        return;
      }
      if (code == 100)
        switchView();
      if (code == 110)
        game->advanceWave();
      if (code == 111)
        game->boss();
      if (code == 112)
        game->horde();
      if (code == 113) {
        game->autoWaves = !game->autoWaves;
        game->announce(game->autoWaves ? "Fale automatyczne włączone"
                                       : "Fale ręczne - N wysyła kolejną falę");
      }
      if (code == 200)
        help = false;
    };
    std::cout << "RDZEN ARSENAL / " << renderer.adapter << " / " << width << 'x' << height << '\n'
              << std::flush;
    while (running) {
      auto frameStart = Clock::now();
      double dt = std::min(.1, elapsed(last, frameStart));
      last = frameStart;
      double wall = elapsed(start, frameStart);
      if (smoke) {
        if (smokeStage == 0 && wall >= 1) {
          keyEvent(SDL_SCANCODE_TAB);
          smokeStage = 1;
        }
        if (placed < 9 && wall >= nextPlacement && view.tactical) {
          keyEvent(SDL_Scancode(SDL_SCANCODE_1 + placed));
          Vec3 point =
              screenPoint({-64 + placed * 16.f, 0, 70}, camera, renderer.width, renderer.height);
          clickEvent(point.x, point.y);
          ++placed;
          nextPlacement += .16;
        }
        if (smokeStage == 1 && wall >= 3) {
          keyEvent(SDL_SCANCODE_P);
          smokeStage = 2;
        }
        if (smokeStage == 2 && wall >= 3.7) {
          pausePass = pausedTest && std::abs(game->time - frozen) < .0001f;
          keyEvent(SDL_SCANCODE_P);
          smokeStage = 3;
        }
        if (smokeStage == 3 && wall >= 4) {
          Rect b = ui.action(1);
          clickEvent((b.x + 30) * ui.scale, (b.y + 15) * ui.scale);
          keyEvent(SDL_SCANCODE_H);
          keyEvent(SDL_SCANCODE_N);
          smokeStage = 4;
        }
        if (smokeStage == 4 && wall >= 8.6) {
          keyEvent(SDL_SCANCODE_TAB);
          smokeStage = 5;
        }
        if (smokeStage == 5 && wall >= 9.9) {
          keyEvent(SDL_SCANCODE_TAB);
          SDL_SetWindowSize(window.get(), 1280, 720);
          smokeStage = 6;
        }
        if (wall >= 11.5)
          running = false;
      }
      if (stress && wall >= 18)
        running = false;
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
          running = false;
        if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
          renderer.resize(event.window.data1, event.window.data2);
          ui.resize(renderer.width, renderer.height);
        }
        if (event.type == SDL_EVENT_WINDOW_MINIMIZED)
          minimized = true;
        if (event.type == SDL_EVENT_WINDOW_RESTORED)
          minimized = false;
        if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
          focused = false;
          if (!automated) {
            game->paused = true;
            SDL_SetWindowRelativeMouseMode(window.get(), false);
          }
        }
        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
          focused = true;
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
          auto key = event.key.scancode;
          if (key == SDL_SCANCODE_Q && (event.key.mod & SDL_KMOD_CTRL))
            running = false;
          if (key == SDL_SCANCODE_ESCAPE) {
            if (help)
              help = false;
            else if (SDL_GetWindowRelativeMouseMode(window.get()))
              SDL_SetWindowRelativeMouseMode(window.get(), false);
            else
              togglePause();
          }
          if (key == SDL_SCANCODE_F1) {
            help = !help;
            if (help)
              SDL_SetWindowRelativeMouseMode(window.get(), false);
          }
          if (key == SDL_SCANCODE_TAB)
            switchView();
          if (key == SDL_SCANCODE_P)
            togglePause();
          if (key == SDL_SCANCODE_V)
            vsync = !vsync;
          if (key == SDL_SCANCODE_M)
            audio.enabled = !audio.enabled;
          if (key == SDL_SCANCODE_F11)
            SDL_SetWindowFullscreen(window.get(),
                                    !(SDL_GetWindowFlags(window.get()) & SDL_WINDOW_FULLSCREEN));
          if (key >= SDL_SCANCODE_1 && key <= SDL_SCANCODE_9) {
            view.selected = Weapon(key - SDL_SCANCODE_1);
            view.building = true;
          }
          if (key == SDL_SCANCODE_B)
            view.building = !view.building;
          if (key == SDL_SCANCODE_C)
            view.pan = {0, 0, 0};
          if (key == SDL_SCANCODE_N)
            action(110);
          if (key == SDL_SCANCODE_J)
            action(111);
          if (key == SDL_SCANCODE_H)
            action(112);
          if (key == SDL_SCANCODE_F)
            action(113);
          if (key == SDL_SCANCODE_X && view.tactical && view.placement) {
            if (game->removeTower(view.placeX, view.placeZ))
              game->announce("Wieżyczka usunięta");
          }
          if (key == SDL_SCANCODE_R && !automated) {
            game = std::make_unique<Game>();
            audio.reset();
            accumulator = 0;
            game->announce("Nowy poligon. Dziewięć wieżyczek gotowych.");
          }
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
          mouseX = event.motion.x;
          mouseY = event.motion.y;
          if (!view.tactical && SDL_GetWindowRelativeMouseMode(window.get())) {
            view.yaw += event.motion.xrel * .0024f;
            view.pitch = std::clamp(view.pitch - event.motion.yrel * .0024f, -1.35f, 1.35f);
          }
        }
        if (event.type == SDL_EVENT_MOUSE_WHEEL && view.tactical)
          view.zoom = std::clamp(view.zoom - event.wheel.y * view.zoom * .08f, 40.f, 500.f);
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
          mouseX = event.button.x;
          mouseY = event.button.y;
          if (event.button.button == SDL_BUTTON_RIGHT) {
            if (view.tactical)
              view.building = false;
            continue;
          }
          if (event.button.button != SDL_BUTTON_LEFT)
            continue;
          bool captured = SDL_GetWindowRelativeMouseMode(window.get());
          int clicked =
              (!captured || view.tactical) ? ui.click(mouseX, mouseY, view.tactical, help) : -1;
          if (clicked >= 0) {
            action(clicked);
            continue;
          }
          if (help || ui.covered(mouseX, mouseY, view.tactical, help))
            continue;
          if (!view.tactical) {
            if (!game->paused)
              SDL_SetWindowRelativeMouseMode(window.get(), true);
            continue;
          }
          view.camera(camera, float(renderer.width) / renderer.height);
          if (view.building &&
              view.pick(camera, mouseX, mouseY, float(renderer.width), float(renderer.height))) {
            if (game->addTower(view.selected, view.placeX, view.placeZ)) {
              ++buildCount;
              audio.build();
              game->announce(std::string(WEAPONS[view.selected].name) + " - wieżyczka aktywna");
            } else if (auto* why = game->canBuild(view.placeX, view.placeZ))
              game->announce(why);
          }
        }
      }
      if (!running)
        break;
      if (smoke && smokeStage == 2 && !pausedTest) {
        frozen = game->time;
        pausedTest = true;
      }
      if (minimized) {
        SDL_Delay(30);
        accumulator = 0;
        continue;
      }
      if (!help && focused) {
        const bool* keys = SDL_GetKeyboardState(nullptr);
        view.move(float(keys[SDL_SCANCODE_D]) - float(keys[SDL_SCANCODE_A]),
                  float(keys[SDL_SCANCODE_W]) - float(keys[SDL_SCANCODE_S]), float(dt),
                  keys[SDL_SCANCODE_LSHIFT]);
        if (view.tactical)
          view.orbit += (float(keys[SDL_SCANCODE_E]) - float(keys[SDL_SCANCODE_Q])) * float(dt);
      }
      if (!game->paused && !help) {
        accumulator += dt;
        int steps = 0;
        while (accumulator >= 1. / 60 && steps < 8) {
          auto tick = Clock::now();
          game->step(1.f / 60);
          if (stress && wall >= 3)
            simTimes.push_back(elapsed(tick, Clock::now()) * 1000);
          accumulator -= 1. / 60;
          ++steps;
        }
      } else
        accumulator = 0;
      if (stress && game->time >= stressRefill) {
        if (game->active.size() < 5000)
          game->horde(5000 - int(game->active.size()));
        stressRefill = game->time + 1;
      }
      view.camera(camera, float(renderer.width) / renderer.height);
      view.hovered = -1;
      view.placement = false;
      if (view.tactical && !ui.covered(mouseX, mouseY, true, help)) {
        view.pick(camera, mouseX, mouseY, float(renderer.width), float(renderer.height));
        for (size_t i = 0; i < game->towers.size(); ++i)
          if (std::hypot(game->towers[i].x - view.placeX, game->towers[i].z - view.placeZ) < 4) {
            view.hovered = int(i);
            break;
          }
      }
      audio.update(*game);
      scene.prepare(*game, view);
      renderer.begin(camera, game->time, stress && wall < 3 ? 1 : 0);
      scene.render(renderer);
      ui.draw(renderer, *game, view, shownFps, shownGpu, vsync, help, audio.enabled,
              SDL_GetWindowRelativeMouseMode(window.get()));
      renderer.end();
      bool capture = false;
      std::string screenshot;
      if (smoke) {
        if (!shotInitial && wall > .7) {
          shotInitial = true;
          capture = true;
          screenshot = "01-deck.png";
        } else if (!shotTactical && wall > 8.1) {
          shotTactical = true;
          capture = true;
          screenshot = "02-tactical-combat.png";
        } else if (!shotFps && wall > 9.3) {
          shotFps = true;
          capture = true;
          screenshot = "03-deck-combat.png";
        } else if (!shotSmall && wall > 10.8) {
          shotSmall = true;
          capture = true;
          screenshot = "04-small-window.png";
        }
      }
      if (stress && wall > 17.8 && !shotTactical) {
        shotTactical = true;
        capture = true;
        screenshot = "stress-5000.png";
      }
      if (capture)
        renderer.screenshot(artifacts / screenshot);
      renderer.present(vsync);
      double frameMs = elapsed(frameStart, Clock::now()) * 1000;
      if (stress && wall >= 3 && !capture) {
        frameTimes.push_back(frameMs);
        minEnemies = std::min(minEnemies, game->active.size());
        maxEnemies = std::max(maxEnemies, game->active.size());
      }
      for (auto sample : renderer.completed) {
        shownGpu = float(sample.ms);
        if (stress && sample.tag == 0)
          gpuTimes.push_back(sample.ms);
      }
      renderer.completed.clear();
      ++frameCounter;
      displayTimer += dt;
      frameSum += dt;
      if (displayTimer > .4) {
        shownFps = float(frameCounter / std::max(frameSum, .0001));
        displayTimer = frameSum = 0;
        frameCounter = 0;
      }
    }
    SDL_SetWindowRelativeMouseMode(window.get(), false);
    if (smoke) {
      bool allFired = true;
      for (int i = 0; i < WeaponCount; ++i)
        if (game->fired[i] == 0)
          allFired = false;
      bool pass = buildCount == 9 && cameraSwitches >= 3 && pausePass && allFired && shotSmall;
      std::ofstream out(artifacts / "smoke-result.json");
      out << "{\n  \"passed\": " << (pass ? "true" : "false")
          << ",\n  \"towers_built_through_input\": " << buildCount
          << ",\n  \"camera_switches\": " << cameraSwitches
          << ",\n  \"pause_froze_simulation\": " << (pausePass ? "true" : "false")
          << ",\n  \"all_nine_weapons_fired\": " << (allFired ? "true" : "false")
          << ",\n  \"resized_width\": " << renderer.width
          << ",\n  \"resized_height\": " << renderer.height << "\n}\n";
      std::cout << "SMOKE " << (pass ? "PASS" : "FAIL") << " built=" << buildCount
                << " views=" << cameraSwitches << " pause=" << pausePass << " weapons=" << allFired
                << '\n';
      return pass ? 0 : 1;
    }
    if (stress) {
      auto mean = [](const std::vector<double>& v) {
        return v.empty() ? 0. : std::accumulate(v.begin(), v.end(), 0.) / v.size();
      };
      std::ofstream out(artifacts / "stress-result.json");
      out << "{\n  \"resolution\": \"1920x1080\",\n  \"fps_mean\": " << 1000 / mean(frameTimes)
          << ",\n  \"frame_p99_ms\": " << percentile(frameTimes, .99)
          << ",\n  \"frame_max_ms\": " << percentile(frameTimes, 1)
          << ",\n  \"sim_tick_mean_ms\": " << mean(simTimes)
          << ",\n  \"gpu_mean_ms\": " << mean(gpuTimes) << ",\n  \"enemies_min\": " << minEnemies
          << ",\n  \"enemies_max\": " << maxEnemies << ",\n  \"towers\": " << game->towers.size()
          << ",\n  \"kills\": " << game->kills
          << ",\n  \"sound_enabled\": false,\n  \"vsync\": false\n}\n";
      std::ofstream raw(artifacts / "stress-frames.csv");
      raw << "frame_ms\n";
      for (double ms : frameTimes)
        raw << ms << '\n';
      std::cout << "STRESS " << number(1000 / mean(frameTimes), 1)
                << " FPS p99=" << number(percentile(frameTimes, .99), 2)
                << " ms enemies=" << minEnemies << ".." << maxEnemies
                << " towers=" << game->towers.size() << '\n';
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    if (!automated)
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RDZEŃ - błąd", e.what(), nullptr);
    return 1;
  }
}
