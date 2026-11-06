#pragma once
#include "command_scene.hpp"
#include "sentry_drop_scene.hpp"
#include "frontier_scene.hpp"
#include "service_scene.hpp"
#include "game_audio.hpp"
#include "manual_monitor.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

namespace command {
inline int run(int argc, char** argv) {
  bool test = false, testMini = false, progressionTest = false, deliveryTest = false;
  bool upgradeTest = false, progressionLab = false, supportTest = false;
  bool frontierTest = false, frontierLab = false;
  bool servicesTest = false, servicesLab = false;
  bool manualTest = false;
  for (int i = 1; i < argc; ++i) {
    test |= std::string(argv[i]) == "--command-test";
    testMini |= std::string(argv[i]) == "--minigun";
    progressionTest |= std::string(argv[i]) == "--expedition-test";
    deliveryTest |= std::string(argv[i]) == "--delivery-test";
    upgradeTest |= std::string(argv[i]) == "--upgrade-test";
    progressionLab |= std::string(argv[i]) == "--progression-lab";
    supportTest |= std::string(argv[i]) == "--support-test";
    frontierTest |= std::string(argv[i]) == "--frontier-test";
    frontierLab |= std::string(argv[i]) == "--frontier-lab";
    servicesTest |= std::string(argv[i]) == "--services-test";
    servicesLab |= std::string(argv[i]) == "--services-lab";
    manualTest |= std::string(argv[i]) == "--manual-test";
  }
  upgradeTest |= supportTest;
  deliveryTest |= upgradeTest;
  progressionTest |= deliveryTest;
  test |= progressionTest || frontierTest || servicesTest || manualTest;
  progressionLab |= frontierLab;
  const bool liveExpedition = !test || progressionTest || frontierTest || servicesTest;
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
    auto root = std::filesystem::current_path(), assets = root / "public" / "assets";
    auto artifacts = root / "artifacts" / "command" /
                     (manualTest     ? "manual"
                      : servicesTest ? "services"
                      : frontierTest ? "frontier"
                      : supportTest  ? "supports"
                      : upgradeTest  ? "upgrades"
                      : testMini     ? "minigun"
                                     : "laser");
    if (test)
      std::filesystem::create_directories(artifacts);
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("RDZEŃ - Centrum dowodzenia", 1600, 900, SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);
    if (!window)
      throw std::runtime_error(SDL_GetError());
    if (test)
      SDL_SetWindowPosition(window.get(), 0, 0);
    else
      SDL_SetWindowFullscreen(window.get(), true);
    int width, height;
    SDL_GetWindowSizeInPixels(window.get(), &width, &height);
    Renderer renderer(window.get(), width, height, assets);
    GameScene scene(renderer, assets);
    CommandRoof roof(renderer);
    SentryDropScene dropScene(renderer);
    FrontierScene frontierScene(renderer, assets);
    SniperMonitor sniperMonitor;
    CommandUi ui;
    ui.resize(width, height);
    GameAudio audio;
    if (test)
      audio.enabled = false;
    auto session = std::make_unique<Session>(liveExpedition);
    ViewState passive;
    passive.building = false;
    ViewState tacticalView;
    tacticalView.tactical = true;
    tacticalView.building = false;
    tacticalView.zoom = TACTICAL_ZOOM;
    int hoveredSlot = -1;
    uint64_t hoveredCrystal = 0;
    Camera camera;
    std::array<bool, SDL_SCANCODE_COUNT> held{};
    bool firing = false, running = true, help = false, vsync = !test, minimized = false;
    using Clock = std::chrono::steady_clock;
    auto begin = Clock::now(), last = begin;
    double accumulator = 0, fpsTime = 0;
    int frames = 0;
    float fps = 60;
    int stage = 0, fixture = -1;
    int manualIndex = 0;
    std::array<bool, GUN_COUNT> manualChecks{};
    bool manualSightShot = false, manualModelShot = false;
    double stageAt = 0;
    bool menuShot = false, liftShot = false, combatShot = false, roofShot = false;
    bool pausedPass = false, raisedPass = false, returnedPass = false, noBuildPass = true;
    float frozen = 0;
    uint64_t firedBefore = 0;
    Vec3 returnAt;
    bool deathHandled = false, rewardPass = false, defeatPass = false, resetPass = false;
    int delivered = 0, applied = 0;
    bool landedPass = false, dropPausePass = false, balloonPass = false, returnViewPass = false;
    bool earlyRewardPass = false, placementFreezePass = false;
    bool supportVisualPass = false;
    bool factoryPass = false, crystalPickPass = false, collectorPausePass = false,
         bossRarityPass = false;
    Vec3 collectorFrozen{};
    uint64_t testCrystal = 0;
    std::array<bool, 6> serviceChecks{};
    int serviceBoss = -1, serviceOutside = -1, serviceSlot = -1;
    float serviceHp = 0;
    int earnedLoads = 0;
    Upgrade expectedChoice = Upgrade::Damage;
    std::string capture;
    static constexpr Uint32 testDevice = 0xFFFFFFFEu;
    auto key = [](SDL_Scancode code, bool down = true) {
      SDL_Event e{};
      e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
      e.key.scancode = code;
      e.key.which = testDevice;
      e.key.down = down;
      SDL_PushEvent(&e);
    };
    auto click = [&](Rect r) {
      SDL_Event e{};
      e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
      e.button.button = SDL_BUTTON_LEFT;
      e.button.which = testDevice;
      e.button.x = (r.x + r.w * .5f) * ui.scale;
      e.button.y = (r.y + r.h * .5f) * ui.scale;
      SDL_PushEvent(&e);
      e.type = SDL_EVENT_MOUSE_BUTTON_UP;
      SDL_PushEvent(&e);
    };
    auto release = [&]() {
      session->cancelAim();
      firing = false;
      held.fill(false);
      SDL_SetWindowRelativeMouseMode(window.get(), false);
    };
    auto launch = [&]() {
      if (!test)
        session->expedition.rewardRandom.state =
            uint32_t(Clock::now().time_since_epoch().count()) | 1u;
      if (!test)
        session->frontier.lootRandom.state =
            (uint32_t(Clock::now().time_since_epoch().count()) ^ 0x9E3779B9u) | 1u;
      session->start(session->module.kind);
      // Legacy integration tests begin after the new physical start-button interaction.
      if (test && liveExpedition && !servicesTest)
        session->startExtraction();
      if (servicesLab) {
        session->expedition.hp = session->expedition.maxHp = 20000;
        session->expedition.servicePoints = 6;
        for (int i = 0; i < 3; ++i)
          session->expedition.rewards.push_back(
              {uint64_t(i + 1),
               {Upgrade::ServiceCredit, Upgrade::Mining, Upgrade::Hull},
               {Rarity::Rare, Rarity::Rare, Rarity::Rare}});
        session->expedition.rewardSerial = 3;
      }
      if (progressionLab) {
        // Explicit standalone sandbox; normal START.bat keeps the survival economy.
        session->expedition.hp = session->expedition.maxHp = 20000;
        session->expedition.miningMultiplier = 4;
        for (auto u :
             {Upgrade::Sentry, Upgrade::MinigunTower, Upgrade::TeslaTower, Upgrade::LanceTower}) {
          session->sentryInventory.push_back(towerWeapon(u));
          ++session->expedition.levels[size_t(u)];
          ++session->sentryStock;
        }
        for (int i = 0; i < 2; ++i)
          session->expedition.rewards.push_back(
              {uint64_t(i + 1),
               {Upgrade::RateRelayTower, Upgrade::RangeRelayTower, Upgrade::FrostSupportTower},
               {Rarity::Rare, Rarity::Rare, Rarity::Rare}});
        session->expedition.rewards.push_back({3,
                                               {Upgrade::Radar, Upgrade::Frost, Upgrade::Focus},
                                               {Rarity::Rare, Rarity::Rare, Rarity::Rare}});
        session->expedition.rewards.push_back({4,
                                               {Upgrade::Frost, Upgrade::Focus, Upgrade::Purge},
                                               {Rarity::Rare, Rarity::Rare, Rarity::Epic}});
        session->expedition.rewards.push_back(
            {5,
             {Upgrade::TowerDamage, Upgrade::TowerRate, Upgrade::TowerRange},
             {Rarity::Legendary, Rarity::Epic, Rarity::Rare}});
        session->expedition.earned = 5;
        session->expedition.rewardSerial = 5;
        session->world.announce(
            "POLIGON ROZWOJU: 4 bojowe [Tab]. Wybierz wsparcie i moduły przy komputerze [E].");
      }
      if (frontierLab) {
        session->world.time = 150;
        session->frontier.nextElite = 155;
        session->expedition.rewards.clear();
        std::array<std::array<Upgrade, 3>, 6> cards{
            {{Upgrade::RoverFactory, Upgrade::Radar, Upgrade::Hull},
             {Upgrade::Radar, Upgrade::Frost, Upgrade::Focus},
             {Upgrade::RoverBatch, Upgrade::RoverRate, Upgrade::Hull},
             {Upgrade::RateRelayTower, Upgrade::RangeRelayTower, Upgrade::FrostSupportTower},
             {Upgrade::Sentry, Upgrade::TeslaTower, Upgrade::LanceTower},
             {Upgrade::TowerDamage, Upgrade::TowerRange, Upgrade::Hull}}};
        for (size_t i = 0; i < cards.size(); ++i)
          session->expedition.rewards.push_back(
              {i + 1, cards[i], {Rarity::Epic, Rarity::Rare, Rarity::Rare}});
        session->expedition.earned = session->expedition.rewardSerial = 6;
        session->world.announce(
            "POLIGON EKSPEDYCJI: boss za 30 s. Fabryka i radar czekają przy komputerze.");
      }
      audio.reset();
      accumulator = 0;
      SDL_SetWindowRelativeMouseMode(window.get(), true);
    };
    auto pause = [&]() {
      session->world.paused = !session->world.paused;
      release();
      if (!session->world.paused && !session->tactical)
        SDL_SetWindowRelativeMouseMode(window.get(), true);
    };
    auto reset = [&]() {
      release();
      session = std::make_unique<Session>(liveExpedition);
      audio.reset();
      help = false;
      accumulator = 0;
      deathHandled = false;
      tacticalView = ViewState{};
      tacticalView.tactical = true;
      tacticalView.building = false;
      tacticalView.zoom = TACTICAL_ZOOM;
    };
    auto choose = [&](int index) {
      if (session->chooseUpgrade(index)) {
        release();
        SDL_SetWindowRelativeMouseMode(window.get(), !session->tactical);
        audio.build();
        accumulator = 0;
      }
    };
    auto toggleTactical = [&]() {
      if (session->serviceTarget != Service::None) {
        session->cancelService();
        release();
        return;
      }
      if (!session->expedition.enabled || session->terminalOpen || session->servicesOpen ||
          session->expedition.dead())
        return;
      session->tactical = !session->tactical;
      release();
      SDL_SetWindowRelativeMouseMode(window.get(), !session->tactical && !session->world.paused);
    };
    auto chooseService = [&](int choice) {
      if (session->chooseService(choice)) {
        release();
        SDL_SetWindowRelativeMouseMode(window.get(), !session->tactical && !session->servicesOpen);
        accumulator = 0;
        audio.build();
      }
    };
    auto pickSlot = [&](float x, float y) {
      if (!tacticalView.pick(camera, x, y, float(renderer.width), float(renderer.height)))
        return -1;
      return nearestTurretSlot(tacticalView.placeX, tacticalView.placeZ);
    };
    auto crystalAt = [&](float x, float y) {
      return pickCrystal(*session, camera.matrix(float(renderer.width) / renderer.height),
                         float(renderer.width), float(renderer.height), x, y, ui.scale);
    };
    auto collect = [&](uint64_t id) {
      if (!help && session->expedition.enabled && !session->expedition.dead() &&
          !session->terminalOpen && !session->servicesOpen &&
          session->serviceTarget == Service::None && session->frontier.requestCollection(id)) {
        session->world.announce("Dron przyjął zlecenie. Wypłata po dostarczeniu kryształu.");
        audio.build();
      }
    };
    auto clickWorld = [&](Vec3 p) {
      auto m = camera.matrix(float(renderer.width) / renderer.height);
      float w = p.x * m.m[3] + p.y * m.m[7] + p.z * m.m[11] + m.m[15];
      float x = p.x * m.m[0] + p.y * m.m[4] + p.z * m.m[8] + m.m[12];
      float y = p.x * m.m[1] + p.y * m.m[5] + p.z * m.m[9] + m.m[13];
      click({(x / w + 1) * renderer.width * .5f / ui.scale,
             (1 - y / w) * renderer.height * .5f / ui.scale, 1, 1});
    };
    std::cout << "RDZEN COMMAND / " << renderer.adapter << " / " << width << 'x' << height << '\n'
              << std::flush;
    while (running) {
      auto now = Clock::now();
      double dt = std::min(.1, std::chrono::duration<double>(now - last).count());
      last = now;
      double wall = std::chrono::duration<double>(now - begin).count();
      if (manualTest) {
        if (!menuShot) {
          capture = "manual-selection.png";
          menuShot = true;
        } else if (stage == 0 && wall - stageAt > .4) {
          click(ui.choice(manualIndex));
          click(ui.launch());
          stage = 1;
        } else if (stage == 1 && session->mode == Mode::Walking) {
          manualChecks[manualIndex] = int(session->module.kind) == manualIndex;
          session->world = Game(false);
          session->world.autoWaves = false;
          session->mode = Mode::Mounted;
          session->lift = session->hatch = 1;
          session->spawnTimer = 100000;
          for (int i = 0; i < 18; ++i) {
            float x = (i % 5 - 2) * 8.f, z = 85 + (i / 5) * 8.f;
            if (manualIndex == int(Gun::Flame)) {
              x *= .5f;
              z -= 45;
            }
            int id = session->world.spawn(Crawler, x, z);
            session->world.foes[id].hp = session->world.foes[id].maxHp = 10000;
            session->world.foes[id].speed = 0;
          }
          session->world.grid.rebuild(session->world.foes, session->world.active);
          Vec3 point{0, 1.8f, manualIndex == int(Gun::Flame) ? 40.f : 85.f};
          for (int i = 0; i < 4; ++i) {
            Vec3 direction = normalize(point - session->eye());
            session->yaw = session->gunYaw = std::atan2(direction.x, direction.z);
            session->pitch = session->gunPitch = std::asin(direction.y);
          }
          SDL_SetWindowRelativeMouseMode(window.get(), true);
          firing = true;
          manualSightShot = manualModelShot = false;
          stage = 2;
          stageAt = wall;
        } else if (stage == 2) {
          std::string name = "manual-" + std::to_string(manualIndex + 1);
          if (!manualSightShot && wall - stageAt > 2.6) {
            capture = name + "-sight.png";
            manualSightShot = true;
          } else if (!manualModelShot && wall - stageAt > 3.6) {
            capture = name + "-model.png";
            manualModelShot = true;
          } else if (wall - stageAt > 5.8) {
            manualChecks[manualIndex] =
                manualChecks[manualIndex] && session->shots > 0 && session->impacts > 0;
            std::cout << "MANUAL " << manualIndex + 1 << " shots=" << session->shots
                      << " impacts=" << session->impacts << '\n'
                      << std::flush;
            ++manualIndex;
            if (manualIndex == GUN_COUNT) {
              running = false;
              stage = 3;
            } else {
              release();
              session = std::make_unique<Session>(false);
              stage = 0;
              stageAt = wall;
            }
          }
        }
        if (wall > 90)
          running = false;
      }
      if (test && !progressionTest && !frontierTest && !servicesTest && !manualTest) {
        if (stage == 9 && wall - stageAt > .15 && !roofShot) {
          capture = "06-round-roof.png";
          roofShot = true;
        }
        if (stage == 6 && session->shots > 0 && !combatShot) {
          capture = "04-gunnery.png";
          combatShot = true;
        }
        if (!menuShot && wall > .25) {
          capture = "01-selection.png";
          menuShot = true;
        }
        if (stage == 0 && wall > .6) {
          click(ui.choice(testMini ? 1 : 0));
          click(ui.launch());
          stage = 1;
          stageAt = wall;
        } else if (stage == 1 && wall - stageAt > .25) {
          key(SDL_SCANCODE_W);
          stage = 2;
        } else if (stage == 2 && session->player.z >= -4.1f) {
          key(SDL_SCANCODE_W, false);
          capture = "02-seat-prompt.png";
          returnAt = session->player;
          stage = 3;
          stageAt = wall;
        } else if (stage == 3 && wall - stageAt > .2) {
          key(SDL_SCANCODE_E);
          stage = 4;
        } else if (stage == 4) {
          if (!liftShot && session->lift > .45f) {
            capture = "03-elevator.png";
            liftShot = true;
          }
          if (session->mode == Mode::Mounted) {
            raisedPass = session->eye().y > Session::ROOF + 2 && session->hatch == 1;
            fixture = session->world.spawn(Crawler, 0, 95);
            session->world.foes[fixture].speed = 0;
            session->world.grid.rebuild(session->world.foes, session->world.active);
            Vec3 d = normalize(session->world.targetPoint(fixture) - session->eye());
            SDL_Event e{};
            e.type = SDL_EVENT_MOUSE_MOTION;
            e.motion.which = testDevice;
            e.motion.xrel = (std::atan2(d.x, d.z) - session->yaw) / .0024f;
            e.motion.yrel = (session->pitch - std::asin(d.y)) / .0024f;
            SDL_PushEvent(&e);
            stage = 5;
            stageAt = wall;
          }
        } else if (stage == 5 && wall - stageAt > .2) {
          click({ui.width * .5f, ui.height * .5f, 1, 1});
          SDL_Event e{};
          e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
          e.button.which = testDevice;
          e.button.button = SDL_BUTTON_LEFT;
          SDL_PushEvent(&e);
          stage = 6;
          stageAt = wall;
        } else if (stage == 6 && wall - stageAt > (testMini ? 2.5 : 1.25)) {
          SDL_Event e{};
          e.type = SDL_EVENT_MOUSE_BUTTON_UP;
          e.button.which = testDevice;
          e.button.button = SDL_BUTTON_LEFT;
          SDL_PushEvent(&e);
          key(SDL_SCANCODE_P);
          stage = 7;
          stageAt = wall;
          frozen = session->world.time;
          firedBefore = session->shots;
        } else if (stage == 7 && wall - stageAt > .4) {
          pausedPass = session->world.time == frozen && session->shots == firedBefore;
          key(SDL_SCANCODE_P);
          key(SDL_SCANCODE_E);
          stage = 8;
        } else if (stage == 8 && session->mode == Mode::Walking) {
          returnedPass = length(session->player - returnAt) < .15f && session->lift == 0 &&
                         session->hatch == 0;
          // Previously active arsenal keys must not enable construction or spawn bosses.
          key(SDL_SCANCODE_TAB);
          key(SDL_SCANCODE_3);
          key(SDL_SCANCODE_B);
          key(SDL_SCANCODE_J);
          SDL_SetWindowSize(window.get(), 1280, 720);
          capture = "05-return.png";
          stage = 9;
          stageAt = wall;
        } else if (stage == 9 && wall - stageAt > .35) {
          noBuildPass = session->world.towers.empty() && session->world.bossSerial == 0;
          running = false;
        }
        if (wall > 25)
          running = false;
      }
      if (servicesTest) {
        if (stage == 0 && wall > .3) {
          click(ui.launch());
          stage = 1;
        } else if (stage == 1 && session->mode == Mode::Walking) {
          serviceChecks[0] = !session->expedition.miningStarted && session->world.active.empty();
          session->player = {0, CORE_HEIGHT + 1.72f, 5.8f};
          capture = "services-01-start-button.png";
          stage = 2;
          stageAt = wall;
        } else if (stage == 2 && wall - stageAt > .25) {
          key(SDL_SCANCODE_E);
          stage = 3;
        } else if (stage == 3 && session->buttonAge > .15f && session->expedition.miningStarted) {
          capture = "services-02-press.png";
          stage = 4;
        } else if (stage == 4 && session->buttonAge > .85f) {
          capture = "services-03-retract.png";
          stage = 5;
        } else if (stage == 5 && session->buttonAge > 2.4f) {
          serviceChecks[0] &= session->expedition.uranium > 0;
          for (int id : session->world.active)
            session->world.foes[id].hp = 0;
          session->world.step(1.f / 60);
          session->spawnTimer = session->frontier.nextBoss = session->frontier.nextElite = 100000;
          session->expedition.miningMultiplier = 0;
          session->player = {-6.8f, CORE_HEIGHT + 1.72f, -6.2f};
          session->expedition.rewards.push_back(
              {1,
               {Upgrade::ServiceCredit, Upgrade::Damage, Upgrade::Hull},
               {Rarity::Rare, Rarity::Rare, Rarity::Rare}});
          session->deckClock = .1f;
          capture = "services-04-upgrade-lamp.png";
          stage = 6;
          stageAt = wall;
        } else if (stage == 6 && wall - stageAt > .25) {
          key(SDL_SCANCODE_E);
          stage = 7;
          stageAt = wall;
        } else if (stage == 7 && session->terminalOpen && wall - stageAt > .2) {
          capture = "services-05-credit-card.png";
          stage = 8;
          stageAt = wall;
        } else if (stage == 8 && wall - stageAt > .2) {
          click(ui.rewardCard(0));
          stage = 9;
        } else if (stage == 9 && session->expedition.servicePoints == 1 && !session->terminalOpen) {
          serviceChecks[1] = session->expedition.rewards.empty();
          session->player.x = 6.8f;
          key(SDL_SCANCODE_E);
          stage = 10;
          stageAt = wall;
        } else if (stage == 10 && session->servicesOpen && wall - stageAt > .25) {
          capture = "services-06-service-menu.png";
          stage = 100;
          stageAt = wall;
        } else if (stage == 100 && wall - stageAt > .2) {
          click(ui.rewardCard(0));
          stage = 11;
          stageAt = wall;
          frozen = session->world.time;
        } else if (stage == 11 && session->serviceTarget == Service::Brimstone &&
                   wall - stageAt > .35) {
          serviceChecks[2] =
              session->world.time == frozen && session->expedition.servicePoints == 1;
          serviceBoss = session->world.spawn(Titan, 42, 36);
          serviceOutside = session->world.spawn(Crawler, 53, 36);
          for (int id : {serviceBoss, serviceOutside}) {
            session->world.foes[id].hp = session->world.foes[id].maxHp = 10000;
            session->world.foes[id].speed = 0;
          }
          session->world.grid.rebuild(session->world.foes, session->world.active);
          clickWorld({42, 0, 36});
          stage = 12;
        } else if (stage == 12 && !session->services.casts.empty() &&
                   session->services.casts.front().age > 2.5f) {
          serviceChecks[2] &= session->expedition.servicePoints == 0 &&
                              session->world.foes[serviceBoss].hp == 10000;
          capture = "services-07-brimstone-warning.png";
          stage = 13;
        } else if (stage == 13 && session->services.casts.front().age > 3.2f) {
          capture = "services-08-brimstone.png";
          key(SDL_SCANCODE_P);
          frozen = session->services.casts.front().age;
          serviceHp = session->world.foes[serviceBoss].hp;
          stage = 14;
          stageAt = wall;
        } else if (stage == 14 && wall - stageAt > .3) {
          serviceChecks[3] = session->services.casts.front().age == frozen &&
                             session->world.foes[serviceBoss].hp == serviceHp;
          key(SDL_SCANCODE_P);
          stage = 15;
        } else if (stage == 15 && session->services.casts.empty()) {
          serviceChecks[3] &= std::abs(session->world.foes[serviceBoss].hp - 1000) < 1 &&
                              session->world.foes[serviceOutside].hp == 10000;
          session->expedition.servicePoints = 1;
          key(SDL_SCANCODE_TAB);
          key(SDL_SCANCODE_E);
          stage = 16;
          stageAt = wall;
        } else if (stage == 16 && session->servicesOpen && wall - stageAt > .2) {
          key(SDL_SCANCODE_2);
          stage = 17;
        } else if (stage == 17 && !session->services.casts.empty() &&
                   session->services.casts.front().age > 3.1f) {
          serviceChecks[4] = session->world.foes[serviceBoss].stun > 4.7f &&
                             session->world.foes[serviceOutside].stun > 4.7f;
          capture = "services-09-stun.png";
          serviceSlot = nearestTurretSlot(42, 36);
          auto p = TURRET_SLOTS[serviceSlot];
          session->world.addTower(Laser, p.x, p.z);
          session->world.towers.back().cooldown = 1000;
          session->expedition.levels[size_t(Upgrade::Sentry)] = 1;
          session->expedition.servicePoints = 1;
          stage = 101;
          stageAt = wall;
        } else if (stage == 101 && wall - stageAt > .2) {
          key(SDL_SCANCODE_E);
          stage = 18;
          stageAt = wall;
        } else if (stage == 18 && session->servicesOpen && wall - stageAt > .2) {
          key(SDL_SCANCODE_3);
          stage = 19;
          stageAt = wall;
        } else if (stage == 19 && session->serviceTarget == Service::Overload &&
                   wall - stageAt > .2) {
          auto p = TURRET_SLOTS[serviceSlot];
          clickWorld({p.x, 0, p.z});
          stage = 20;
        } else if (stage == 20 && session->serviceTarget == Service::None) {
          capture = "services-10-overload.png";
          stage = 21;
        } else if (stage == 21 && session->world.towers.empty()) {
          serviceChecks[5] = session->slotAvailable(serviceSlot) &&
                             session->expedition.chosenTypes(false) == 1 &&
                             session->expedition.eligible(Upgrade::Sentry) &&
                             session->expedition.servicePoints == 0;
          capture = "services-11-cleared-hex.png";
          stage = 22;
          stageAt = wall;
        } else if (stage == 22 && wall - stageAt > .2) {
          key(SDL_SCANCODE_R);
          stage = 23;
        } else if (stage == 23 && session->mode == Mode::Selection) {
          serviceChecks[0] &=
              !session->expedition.miningStarted && session->expedition.servicePoints == 0;
          stage = 24;
          running = false;
        }
        if (wall > 50)
          running = false;
      }
      if (frontierTest) {
        if (stage == 0 && wall > .4) {
          click(ui.launch());
          stage = 1;
        } else if (stage == 1 && session->mode == Mode::Walking) {
          for (int id : session->world.active)
            session->world.foes[id].hp = 0;
          session->world.step(1.f / 60);
          session->world.time = 179.9f;
          session->spawnTimer = session->frontier.nextElite = 100000;
          session->expedition.hp = session->expedition.maxHp = 20000;
          session->expedition.miningMultiplier = 0;
          session->expedition.rewards.push_back(
              {1,
               {Upgrade::RoverFactory, Upgrade::Hull, Upgrade::Mining},
               {Rarity::Rare, Rarity::Rare, Rarity::Rare}});
          key(SDL_SCANCODE_A);
          stage = 2;
        } else if (stage == 2 && session->player.x <= -6.7f) {
          key(SDL_SCANCODE_A, false);
          key(SDL_SCANCODE_W);
          stage = 3;
        } else if (stage == 3 && session->player.z >= -6.3f) {
          key(SDL_SCANCODE_W, false);
          key(SDL_SCANCODE_E);
          stage = 4;
          stageAt = wall;
        } else if (stage == 4 && session->terminalOpen && wall - stageAt > .25) {
          capture = "frontier-01-factory-menu.png";
          stage = 5;
          stageAt = wall;
        } else if (stage == 5 && wall - stageAt > .2) {
          click(ui.rewardCard(0));
          stage = 6;
        } else if (stage == 6 && session->frontier.roversMade > 0) {
          factoryPass = session->expedition.levels[size_t(Upgrade::RoverFactory)] == 1;
          session->expedition.rewards.push_back(
              {2,
               {Upgrade::RoverBatch, Upgrade::RoverRate, Upgrade::Hull},
               {Rarity::Epic, Rarity::Rare, Rarity::Rare}});
          session->expedition.rewardSerial = 2;
          key(SDL_SCANCODE_E);
          stage = 7;
          stageAt = wall;
        } else if (stage == 7 && session->terminalOpen && wall - stageAt > .2) {
          click(ui.rewardCard(0));
          stage = 8;
          stageAt = wall;
        } else if (stage == 8 && !session->terminalOpen && wall - stageAt > .15) {
          factoryPass &= session->expedition.roverBatch == 5;
          capture = "frontier-02-rover.png";
          for (int id : session->world.active)
            if (session->world.foes[id].type == Titan)
              fixture = id;
          if (fixture >= 0) {
            auto& foe = session->world.foes[fixture];
            foe.x = 0;
            foe.z = 90;
            foe.speed = 0;
            foe.hp = 150;
            session->world.grid.rebuild(session->world.foes, session->world.active);
          }
          session->player = {0, CORE_HEIGHT + 1.72f, -4};
          session->yaw = 0;
          session->pitch = -.08f;
          key(SDL_SCANCODE_E);
          stage = 9;
        } else if (stage == 9 && session->mode == Mode::Mounted && fixture >= 0) {
          Vec3 d = normalize(session->world.targetPoint(fixture) - session->eye());
          SDL_Event e{};
          e.type = SDL_EVENT_MOUSE_MOTION;
          e.motion.which = testDevice;
          e.motion.xrel = (std::atan2(d.x, d.z) - session->yaw) / .0024f;
          e.motion.yrel = (session->pitch - std::asin(d.y)) / .0024f;
          SDL_PushEvent(&e);
          stage = 10;
          stageAt = wall;
        } else if (stage == 10 && wall - stageAt > .35) {
          SDL_Event e{};
          e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
          e.button.which = testDevice;
          e.button.button = SDL_BUTTON_LEFT;
          SDL_PushEvent(&e);
          stage = 11;
        } else if (stage == 11 && session->world.bossKills > 0 &&
                   !session->frontier.crystals.empty()) {
          SDL_Event e{};
          e.type = SDL_EVENT_MOUSE_BUTTON_UP;
          e.button.which = testDevice;
          e.button.button = SDL_BUTTON_LEFT;
          SDL_PushEvent(&e);
          testCrystal = session->frontier.crystals.front().id;
          for (int n = 0; n < 15; ++n)
            session->frontier.spawnFoe(session->world, Species(n % 5), -50 + n * 7,
                                       110 + (n % 3) * 20, n == 3);
          session->world.spawn(Queen, -85, 170);
          session->expedition.levels[size_t(Upgrade::Radar)] = 1;
          key(SDL_SCANCODE_TAB);
          stage = 12;
          stageAt = wall;
        } else if (stage == 12 && wall - stageAt > .25) {
          capture = "frontier-03-boss-crystal.png";
          stage = 13;
          stageAt = wall;
        } else if (stage == 13 && wall - stageAt > .2) {
          auto* c = session->frontier.crystal(testCrystal);
          if (c) {
            Vec3 p = c->position + Vec3{0, 1.8f, 0};
            auto m = camera.matrix(float(renderer.width) / renderer.height);
            float w = p.x * m.m[3] + p.y * m.m[7] + p.z * m.m[11] + m.m[15];
            float x = p.x * m.m[0] + p.y * m.m[4] + p.z * m.m[8] + m.m[12];
            float y = p.x * m.m[1] + p.y * m.m[5] + p.z * m.m[9] + m.m[13];
            click({(x / w + 1) * renderer.width * .5f / ui.scale,
                   (1 - y / w) * renderer.height * .5f / ui.scale, 1, 1});
          }
          stage = 14;
        } else if (stage == 14 && session->frontier.collectorState == CollectorState::Outbound &&
                   session->frontier.collectorAge > .5f) {
          crystalPickPass = session->expedition.rewards.empty();
          key(SDL_SCANCODE_P);
          collectorFrozen = session->frontier.collectorPosition;
          stage = 15;
          stageAt = wall;
        } else if (stage == 15 && wall - stageAt > .35) {
          collectorPausePass = length(session->frontier.collectorPosition - collectorFrozen) == 0;
          key(SDL_SCANCODE_P);
          stage = 16;
        } else if (stage == 16 && session->frontier.collectorState == CollectorState::Collecting) {
          capture = "frontier-04-collector.png";
          stage = 17;
        } else if (stage == 17 && session->frontier.collected == 1) {
          bossRarityPass = session->expedition.rewards.size() == 1 &&
                           session->expedition.rewards.front().source == RewardSource::BossCrystal;
          for (auto r : session->expedition.rewards.front().rarities)
            bossRarityPass &= r >= Rarity::Rare;
          key(SDL_SCANCODE_TAB);
          key(SDL_SCANCODE_E);
          stage = 18;
        } else if (stage == 18 && session->mode == Mode::Walking) {
          key(SDL_SCANCODE_S);
          stage = 19;
        } else if (stage == 19 && session->player.z <= -6.3f) {
          key(SDL_SCANCODE_S, false);
          key(SDL_SCANCODE_A);
          stage = 20;
        } else if (stage == 20 && session->player.x <= -6.7f) {
          key(SDL_SCANCODE_A, false);
          key(SDL_SCANCODE_E);
          stage = 21;
          stageAt = wall;
        } else if (stage == 21 && session->terminalOpen && wall - stageAt > .25) {
          capture = "frontier-05-boss-reward.png";
          stage = 22;
          stageAt = wall;
        } else if (stage == 22 && wall - stageAt > .2) {
          click(ui.rewardCard(0));
          stage = 23;
        } else if (stage == 23 && session->expedition.selected == 3) {
          stage = 24;
          running = false;
        }
        if (wall > 65)
          running = false;
      }
      if (progressionTest) {
        if (session->expedition.shipping && session->expedition.flightAge > 4 && !liftShot &&
            capture.empty()) {
          capture = "expedition-02-freighter.png";
          liftShot = true;
        }
        if (stage == 0 && wall > .4) {
          click(ui.launch());
          stage = 1;
        } else if (stage == 1 && session->mode == Mode::Walking) {
          // Start just below the real shipping threshold, without replacing the shipping logic.
          session->expedition.uranium = 99.9f;
          if (upgradeTest)
            session->expedition.levels[size_t(Upgrade::Radar)] = 1;
          key(SDL_SCANCODE_A);
          stage = 2;
        } else if (stage == 2 && session->player.x <= -6.7f) {
          key(SDL_SCANCODE_A, false);
          key(SDL_SCANCODE_W);
          stage = 3;
        } else if (stage == 3 && session->player.z >= -6.3f) {
          key(SDL_SCANCODE_W, false);
          key(SDL_SCANCODE_E);
          stage = 4;
          stageAt = wall;
          frozen = session->world.time;
        } else if (stage == 4 && wall - stageAt > .35) {
          pausedPass = session->terminalOpen && session->world.time == frozen;
          earlyRewardPass = session->expedition.earned == 1 && session->expedition.shipments == 0 &&
                            session->expedition.rewards.size() == 1;
          capture = "expedition-01-terminal.png";
          stage = 5;
          stageAt = wall;
        } else if (stage == 5 && wall - stageAt > .2) {
          key(SDL_SCANCODE_E);
          stage = 6;
        } else if (stage == 6) {
          if (session->expedition.shipping && session->expedition.flightAge > 4 && !liftShot) {
            capture = "expedition-02-freighter.png";
            liftShot = true;
          }
          if (!session->expedition.rewards.empty()) {
            key(SDL_SCANCODE_E);
            stage = 7;
            stageAt = wall;
          }
        } else if (stage == 7 && session->terminalOpen && wall - stageAt > .2) {
          if (upgradeTest) {
            session->expedition.rewards.front().choices = {Upgrade::TeslaTower, Upgrade::Purge,
                                                           Upgrade::TowerDamage};
            session->expedition.rewards.front().rarities = {Rarity::Rare, Rarity::Epic,
                                                            Rarity::Legendary};
            if (supportTest) {
              session->expedition.rewards.front().choices = {
                  Upgrade::RateRelayTower, Upgrade::RangeRelayTower, Upgrade::FrostSupportTower};
              session->expedition.rewards.front().rarities = {Rarity::Rare, Rarity::Rare,
                                                              Rarity::Rare};
            }
          } else if (deliveryTest)
            session->expedition.rewards.front().choices = {Upgrade::Sentry, Upgrade::Hull,
                                                           Upgrade::Mining};
          else
            session->expedition.rewards.front().choices = {Upgrade::Damage, Upgrade::Hull,
                                                           Upgrade::Mining};
          capture = "expedition-03-reward.png";
          stage = 8;
          stageAt = wall;
        } else if (stage == 8 && wall - stageAt > .2) {
          expectedChoice = session->expedition.rewards.front().choices[0];
          click(ui.rewardCard(0));
          stage = 9;
          stageAt = wall;
        } else if (stage == 9 && !session->terminalOpen && session->expedition.selected == 1) {
          rewardPass = session->expedition.levels[size_t(expectedChoice)] == 1 &&
                       session->expedition.rewards.empty();
          delivered = session->expedition.shipments;
          earnedLoads = session->expedition.earned;
          applied = session->expedition.selected;
          capture = deliveryTest ? "delivery-01-grid.png" : "expedition-04-upgraded.png";
          stage = deliveryTest ? 20 : 10;
          stageAt = wall;
          if (deliveryTest)
            frozen = session->world.time;
        } else if (stage == 10 && wall - stageAt > .25 && session->expedition.shipments > 0) {
          delivered = session->expedition.shipments;
          session->expedition.hp = .05f;
          int id = session->world.spawn(Crawler, 0, 15.9f);
          session->world.foes[id].speed = 0;
          stage = 11;
        } else if (stage == 11 && session->expedition.dead()) {
          capture = "expedition-05-defeat.png";
          stage = 12;
          stageAt = wall;
          frozen = session->world.time;
        } else if (stage == 12 && wall - stageAt > .3) {
          defeatPass = session->world.time == frozen;
          key(SDL_SCANCODE_R);
          stage = 13;
        } else if (stage == 13 && session->mode == Mode::Selection) {
          resetPass = session->expedition.hp == 1500 && session->expedition.rewards.empty() &&
                      session->expedition.selected == 0 && session->world.active.empty();
          capture = "expedition-06-restart.png";
          stage = 14;
          stageAt = wall;
        } else if (stage == 14 && wall - stageAt > .2)
          running = false;
        if (deliveryTest && stage == 20 && wall - stageAt > .35) {
          placementFreezePass = session->placingSentry() && session->world.time == frozen;
          const auto& slot = TURRET_SLOTS[nearestTurretSlot(36, 31)];
          auto m = camera.matrix(float(renderer.width) / renderer.height);
          float w = slot.x * m.m[3] + slot.z * m.m[11] + m.m[15];
          float px = slot.x * m.m[0] + slot.z * m.m[8] + m.m[12];
          float py = slot.x * m.m[1] + slot.z * m.m[9] + m.m[13];
          click({(px / w + 1) * renderer.width * .5f / ui.scale,
                 (1 - py / w) * renderer.height * .5f / ui.scale, 1, 1});
          stage = 21;
        } else if (deliveryTest && stage == 21 && !session->sentryDrops.empty() &&
                   session->sentryDrops.front().age > 2.2f) {
          capture = "delivery-02-descending.png";
          frozen = session->sentryDrops.front().age;
          key(SDL_SCANCODE_P);
          stage = 22;
          stageAt = wall;
        } else if (deliveryTest && stage == 22 && wall - stageAt > .35) {
          dropPausePass =
              !session->sentryDrops.empty() && session->sentryDrops.front().age == frozen;
          key(SDL_SCANCODE_P);
          stage = 23;
        } else if (deliveryTest && stage == 23 && !session->sentryDrops.empty() &&
                   session->sentryDrops.front().installed) {
          landedPass = session->world.towers.size() == 1 && session->sentryStock == 0;
          if (upgradeTest) {
            landedPass &= session->world.towers.front().type == (supportTest ? RateRelay : Tesla);
            const auto t = session->world.towers.front();
            for (int n = 0; n < 5; ++n) {
              int id = session->world.spawn(Species(n), t.x + 15 + n * 3, t.z + 10);
              session->world.foes[id].speed = 0;
            }
            if (supportTest) {
              for (int i = 0; i < 6; ++i) {
                float a = i * PI / 3;
                session->world.addTower(Laser, t.x + std::cos(a) * SUPPORT_SPACING,
                                        t.z + std::sin(a) * SUPPORT_SPACING);
              }
              session->world.addTower(FrostTower, t.x + 2 * SUPPORT_SPACING, t.z);
              session->expedition.levels[size_t(Upgrade::Sentry)] = 6;
              session->expedition.levels[size_t(Upgrade::FrostSupportTower)] = 1;
            }
          }
          capture = "delivery-03-landed.png";
          stage = 24;
        } else if (deliveryTest && stage == 24 &&
                   session->sentryDrops.front().age > (supportTest ? 9.2f : 7.f)) {
          balloonPass = session->sentryDrops.front().balloonHeight() > 50;
          capture = "delivery-04-return-boost.png";
          if (supportTest)
            supportVisualPass =
                session->world.supportLinks.size() == 6 &&
                std::any_of(session->world.particles.begin(), session->world.particles.end(),
                            [](const Particle& p) { return p.style == 2; });
          stage = 25;
        } else if (deliveryTest && stage == 25 && session->sentryDrops.empty()) {
          balloonPass = balloonPass && session->world.towers.size() == (supportTest ? 8u : 1u) &&
                        session->committedSlots() == (supportTest ? 8 : 1);
          key(SDL_SCANCODE_TAB);
          stage = 26;
          stageAt = wall;
        } else if (deliveryTest && stage == 26 && wall - stageAt > .25) {
          returnViewPass = !session->tactical && session->mode == Mode::Walking;
          capture = supportTest ? "support-radar-tv.png" : "delivery-05-deck.png";
          stage = 27;
          stageAt = wall;
        } else if (deliveryTest && stage == 27 && wall - stageAt > .25)
          running = false;
        if (wall > (deliveryTest ? 45 : 30))
          running = false;
      }
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        // Automated input must not mix with physical mouse movement or key presses.
        if (test && ((event.type == SDL_EVENT_MOUSE_MOTION && event.motion.which != testDevice) ||
                     ((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                       event.type == SDL_EVENT_MOUSE_BUTTON_UP) &&
                      event.button.which != testDevice) ||
                     ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) &&
                      event.key.which != testDevice)))
          continue;
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
        if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST && !test) {
          release();
          if (session->mode != Mode::Selection)
            session->world.paused = true;
        }
        if (event.type == SDL_EVENT_KEY_UP)
          held[event.key.scancode] = false;
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
          auto code = event.key.scancode;
          held[code] = true;
          if (code == SDL_SCANCODE_Q && (event.key.mod & SDL_KMOD_CTRL))
            running = false;
          if (code == SDL_SCANCODE_F11)
            SDL_SetWindowFullscreen(window.get(),
                                    !(SDL_GetWindowFlags(window.get()) & SDL_WINDOW_FULLSCREEN));
          if (code == SDL_SCANCODE_V)
            vsync = !vsync;
          if (code == SDL_SCANCODE_M)
            audio.enabled = !audio.enabled;
          if (session->mode == Mode::Selection) {
            if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_9)
              session->module.kind = Gun(int(code - SDL_SCANCODE_1));
            if (code == SDL_SCANCODE_RETURN || code == SDL_SCANCODE_KP_ENTER)
              launch();
            if (code == SDL_SCANCODE_ESCAPE)
              running = false;
            continue;
          }
          if (code == SDL_SCANCODE_R) {
            reset();
            continue;
          }
          if (session->expedition.dead())
            continue;
          if (session->terminalOpen || session->servicesOpen) {
            if (code == SDL_SCANCODE_ESCAPE || code == SDL_SCANCODE_E) {
              session->terminalOpen = false;
              session->servicesOpen = false;
              release();
              SDL_SetWindowRelativeMouseMode(window.get(), true);
            } else if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_3) {
              if (session->servicesOpen)
                chooseService(int(code - SDL_SCANCODE_1));
              else
                choose(int(code - SDL_SCANCODE_1));
            }
            continue;
          }
          if (code == SDL_SCANCODE_TAB && !help)
            toggleTactical();
          if (code == SDL_SCANCODE_C && !help)
            collect(session->frontier.nearestCrystal());
          if (code == SDL_SCANCODE_F1) {
            help = !help;
            release();
          }
          if (code == SDL_SCANCODE_ESCAPE) {
            if (help)
              help = false;
            else if (session->tactical)
              toggleTactical();
            else if (SDL_GetWindowRelativeMouseMode(window.get()))
              release();
            else
              pause();
          }
          if (code == SDL_SCANCODE_P && !help)
            pause();
          if (code == SDL_SCANCODE_E && !help && !session->world.paused && !session->tactical) {
            if (session->interact()) {
              release();
              SDL_SetWindowRelativeMouseMode(window.get(),
                                             !session->terminalOpen && !session->servicesOpen);
            }
          }
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION && !help &&
            SDL_GetWindowRelativeMouseMode(window.get()))
          session->look(event.motion.xrel * .0024f, -event.motion.yrel * .0024f);
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT)
          firing = false;
        if (event.type == SDL_EVENT_MOUSE_WHEEL && session->tactical)
          tacticalView.zoom =
              std::clamp(tacticalView.zoom - event.wheel.y * tacticalView.zoom * .08f, 90.f, 700.f);
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT &&
            session->tactical)
          toggleTactical();
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
          if (session->servicesOpen) {
            chooseService(ui.rewardClick(event.button.x, event.button.y));
            continue;
          }
          if (session->terminalOpen) {
            choose(ui.rewardClick(event.button.x, event.button.y));
            continue;
          }
          if (session->expedition.dead()) {
            if (ui.restartButton().contains(event.button.x / ui.scale, event.button.y / ui.scale))
              reset();
            continue;
          }
          if (!SDL_GetWindowRelativeMouseMode(window.get()) && session->mode != Mode::Selection &&
              ui.collectorButton(session->expedition.levels[size_t(Upgrade::Radar)] > 0)
                  .contains(event.button.x / ui.scale, event.button.y / ui.scale)) {
            collect(session->frontier.nearestCrystal());
            continue;
          }
          if (session->tactical) {
            bool radar = session->expedition.levels[size_t(Upgrade::Radar)] > 0;
            if (ui.collectorButton(radar).contains(event.button.x / ui.scale,
                                                   event.button.y / ui.scale)) {
              collect(session->frontier.nearestCrystal());
              continue;
            }
            if (!help && !ui.tacticalCovered(event.button.x, event.button.y)) {
              if (session->serviceTarget != Service::None) {
                if (!tacticalView.pick(camera, event.button.x, event.button.y,
                                       float(renderer.width), float(renderer.height)))
                  continue;
                int slot = nearestTurretSlot(tacticalView.placeX, tacticalView.placeZ);
                if (session->targetService({tacticalView.placeX, 0, tacticalView.placeZ}, slot)) {
                  audio.build();
                  accumulator = 0;
                }
                continue;
              }
              uint64_t crystal = crystalAt(event.button.x, event.button.y);
              if (crystal)
                collect(crystal);
              else if (session->orderSentry(pickSlot(event.button.x, event.button.y)))
                audio.build();
            }
            continue;
          }
          if (session->mode == Mode::Selection) {
            int action = ui.selectionClick(event.button.x, event.button.y);
            if (action >= 0 && action < GUN_COUNT)
              session->module.kind = Gun(action);
            if (action == GUN_COUNT)
              launch();
          } else if (!help && !session->world.paused) {
            // First click recaptures the pointer; it cannot accidentally shoot.
            if (SDL_GetWindowRelativeMouseMode(window.get()))
              firing = true;
            else
              SDL_SetWindowRelativeMouseMode(window.get(), true);
          }
        }
      }
      if (!running)
        break;
      if (minimized) {
        SDL_Delay(30);
        accumulator = 0;
        continue;
      }
      Input input{float(held[SDL_SCANCODE_D]) - float(held[SDL_SCANCODE_A]),
                  float(held[SDL_SCANCODE_W]) - float(held[SDL_SCANCODE_S]),
                  held[SDL_SCANCODE_LSHIFT], firing};
      if (session->tactical && !help) {
        tacticalView.move(input.side, input.forward, float(dt), input.fast);
        tacticalView.orbit +=
            (float(held[SDL_SCANCODE_E]) - float(held[SDL_SCANCODE_Q])) * float(dt);
        input = {};
      }
      if (!help)
        session->updateAim(float(dt));
      if (!help && !session->blocked() && session->mode != Mode::Selection) {
        accumulator += dt;
        for (int steps = 0; accumulator >= 1. / 60 && steps < 8; ++steps) {
          session->step(1.f / 60, input);
          accumulator -= 1. / 60;
        }
      } else
        accumulator = 0;
      if (session->expedition.dead() && !deathHandled) {
        release();
        deathHandled = true;
      }
      audio.update(session->world);
      camera.farClip = 5000;
      if (session->tactical && !session->expedition.dead()) {
        tacticalView.camera(camera, float(renderer.width) / renderer.height);
      } else if (session->mode == Mode::Selection) {
        camera.eye = {7, 21.8f, -9};
        camera.target = {-3.5f, 19.4f, 0};
      } else {
        camera.eye = session->eye();
        camera.target = camera.eye + session->forward();
      }
      if (capture == "06-round-roof.png") {
        camera.eye = {29, 34, -34};
        camera.target = {0, CORE_HEIGHT + 2, 0};
      }
      if (capture == "expedition-02-freighter.png") {
        camera.eye = {55, 42, -48};
        camera.target = session->expedition.shipPosition();
      }
      if (supportTest && capture == "delivery-04-return-boost.png") {
        auto t = session->world.towers.front();
        camera.eye = {t.x + 65, 60, t.z - 55};
        camera.target = {t.x + 15, 3, t.z};
      }
      if (capture == "support-radar-tv.png") {
        camera.eye = {2.5f, CORE_HEIGHT + 1.72f, 1};
        camera.target = RADAR_TV_POSITION;
      }
      if (capture == "frontier-02-rover.png" && !session->frontier.rovers.empty()) {
        auto rover = session->frontier.rovers.front().position;
        camera.eye = rover + Vec3{8, 8, -16};
        camera.target = rover + Vec3{0, 1, 3};
      }
      if (capture == "frontier-04-collector.png") {
        camera.eye = session->frontier.collectorPosition + Vec3{13, 8, -17};
        camera.target = session->frontier.collectorPosition - Vec3{0, 1, 0};
      }
      if (capture == "services-01-start-button.png" || capture == "services-02-press.png" ||
          capture == "services-03-retract.png") {
        camera.eye = {2.2f, CORE_HEIGHT + 2.7f, 5.2f};
        camera.target = Session::START_BUTTON;
      }
      if (capture == "services-04-upgrade-lamp.png") {
        camera.eye = {-5.3f, CORE_HEIGHT + 2.0f, -6.0f};
        camera.target = {-6.8f, CORE_HEIGHT + 1.5f, -3.75f};
      }
      if (capture == "services-08-brimstone.png" || capture == "services-09-stun.png" ||
          capture == "services-10-overload.png") {
        camera.eye = {66, 34, 1};
        camera.target = {42, 4, 36};
      }
      if (manualTest && capture.find("-model.png") != std::string::npos) {
        camera.eye = {7, session->baseHeight() + 5, -6};
        camera.target = session->gunPoint({0, 0, 1.4f});
      }
      camera.matrix(float(renderer.width) / renderer.height);
      float mouseX = 0, mouseY = 0;
      SDL_GetMouseState(&mouseX, &mouseY);
      hoveredSlot =
          session->tactical && !ui.tacticalCovered(mouseX, mouseY) ? pickSlot(mouseX, mouseY) : -1;
      hoveredCrystal =
          session->tactical && !ui.tacticalCovered(mouseX, mouseY) ? crystalAt(mouseX, mouseY) : 0;
      renderer.begin(camera, session->world.time, -1);
      scene.prepare(session->world, passive, true);
      sniperMonitor.update(renderer, scene, *session, float(dt));
      buildCommandDeck(scene, *session);
      roof.append(scene, *session);
      dropScene.append(scene, *session);
      buildSlotOverlay(scene, *session, hoveredSlot);
      frontierScene.append(scene, *session, hoveredCrystal);
      appendServiceEffects(scene, *session, {tacticalView.placeX, 0, tacticalView.placeZ},
                           hoveredSlot);
      scene.render(renderer);
      sniperMonitor.draw(renderer, *session);
      ui.drawCommand(renderer, *session, fps, SDL_GetWindowRelativeMouseMode(window.get()), help,
                     audio.enabled, hoveredSlot, hoveredCrystal);
      renderer.end();
      if (!capture.empty()) {
        renderer.screenshot(artifacts / capture);
        capture.clear();
      }
      renderer.present(vsync);
      renderer.completed.clear();
      ++frames;
      fpsTime += dt;
      if (fpsTime >= .5) {
        fps = float(frames / fpsTime);
        frames = 0;
        fpsTime = 0;
      }
    }
    release();
    if (manualTest) {
      bool pass =
          manualIndex == GUN_COUNT && sniperMonitor.frames > 0 &&
          std::all_of(manualChecks.begin(), manualChecks.end(), [](bool value) { return value; });
      std::ofstream out(artifacts / "manual-result.json");
      out << "{\n  \"passed\": " << (pass ? "true" : "false") << ",\n  \"weapons\": [";
      for (int i = 0; i < GUN_COUNT; ++i)
        out << (i ? ", " : "") << (manualChecks[i] ? "true" : "false");
      out << "],\n  \"sniper_monitor_frames\": " << sniperMonitor.frames << "\n}\n";
      std::cout << "MANUAL ARSENAL " << (pass ? "PASS" : "FAIL") << '\n';
      return pass ? 0 : 1;
    }
    if (servicesTest) {
      bool pass = stage == 24 && std::all_of(serviceChecks.begin(), serviceChecks.end(),
                                             [](bool value) { return value; });
      std::ofstream out(artifacts / "services-result.json");
      out << "{\n  \"passed\": " << (pass ? "true" : "false") << ",\n  \"stage\": " << stage;
      const char* names[] = {"manual_start_and_reset",
                             "rare_credit_and_console_roles",
                             "target_selection_and_delay",
                             "brimstone_damage_and_pause",
                             "global_stun",
                             "overload_and_reusable_slot"};
      for (size_t i = 0; i < serviceChecks.size(); ++i)
        out << ",\n  \"" << names[i] << "\": " << (serviceChecks[i] ? "true" : "false");
      out << "\n}\n";
      std::cout << "SERVICES " << (pass ? "PASS" : "FAIL") << " stage=" << stage << " checks=";
      for (bool value : serviceChecks)
        std::cout << value;
      std::cout << '\n';
      return pass ? 0 : 1;
    }
    if (frontierTest) {
      bool pass = stage == 24 && factoryPass && crystalPickPass && collectorPausePass &&
                  bossRarityPass && session->frontier.collected == 1 &&
                  session->frontier.roverHits > 0 && session->impacts > 0 &&
                  session->expedition.earned == 0;
      std::ofstream out(artifacts / "frontier-result.json");
      out << "{\n  \"passed\": " << (pass ? "true" : "false") << ",\n  \"stage\": " << stage
          << ",\n  \"factory_and_batch_cards\": " << (factoryPass ? "true" : "false")
          << ",\n  \"crystal_clicked_in_tactical_view\": " << (crystalPickPass ? "true" : "false")
          << ",\n  \"collector_pause\": " << (collectorPausePass ? "true" : "false")
          << ",\n  \"boss_reward_minimum_rare\": " << (bossRarityPass ? "true" : "false")
          << ",\n  \"collected_crystals\": " << session->frontier.collected
          << ",\n  \"rover_hits\": " << session->frontier.roverHits
          << ",\n  \"manual_impacts\": " << session->impacts << "\n}\n";
      std::cout << "FRONTIER " << (pass ? "PASS" : "FAIL") << " stage=" << stage
                << " factory=" << factoryPass << " crystal=" << crystalPickPass
                << " pause=" << collectorPausePass << " reward=" << bossRarityPass << '\n';
      return pass ? 0 : 1;
    }
    if (deliveryTest) {
      bool pass = stage == 27 && rewardPass && placementFreezePass && earlyRewardPass &&
                  dropPausePass && landedPass && balloonPass && returnViewPass;
      if (upgradeTest)
        pass &= (supportTest ? supportVisualPass : session->world.fired[Tesla] > 0) &&
                session->expedition.levels[size_t(Upgrade::Radar)] == 1;
      std::ofstream out(artifacts / (supportTest   ? "support-result.json"
                                     : upgradeTest ? "upgrade-result.json"
                                                   : "delivery-result.json"));
      out << "{\n  \"passed\": " << (pass ? "true" : "false")
          << ",\n  \"hex_slots\": " << SLOT_COUNT
          << ",\n  \"card_to_stock\": " << (rewardPass ? "true" : "false")
          << ",\n  \"placement_freezes_simulation\": " << (placementFreezePass ? "true" : "false")
          << ",\n  \"reward_before_drone_departure\": " << (earlyRewardPass ? "true" : "false")
          << ",\n  \"drop_pause\": " << (dropPausePass ? "true" : "false")
          << ",\n  \"landed_once\": " << (landedPass ? "true" : "false")
          << ",\n  \"balloon_boost_and_cleanup\": " << (balloonPass ? "true" : "false")
          << ",\n  \"returned_to_deck\": " << (returnViewPass ? "true" : "false") << "\n}\n";
      if (supportTest) {
        std::ofstream details(artifacts / "support-details.json");
        details << "{\n  \"support_links\": " << session->world.supportLinks.size()
                << ",\n  \"blizzard_pulse\": " << (supportVisualPass ? "true" : "false")
                << ",\n  \"radar_tv_unlocked\": "
                << (session->expedition.levels[size_t(Upgrade::Radar)] == 1 ? "true" : "false")
                << "\n}\n";
      } else if (upgradeTest) {
        std::ofstream details(artifacts / "upgrade-details.json");
        details << "{\n  \"radar_unlocked\": "
                << (session->expedition.levels[size_t(Upgrade::Radar)] == 1 ? "true" : "false")
                << ",\n  \"delivered_weapon\": \"Tesla\",\n  \"tesla_shots\": "
                << session->world.fired[Tesla] << "\n}\n";
      }
      std::cout << (supportTest   ? "SUPPORT "
                    : upgradeTest ? "UPGRADES "
                                  : "DELIVERY ")
                << (pass ? "PASS" : "FAIL") << " stage=" << stage << " land=" << landedPass
                << " balloon=" << balloonPass << " pause=" << dropPausePass << '\n';
      return pass ? 0 : 1;
    }
    if (progressionTest) {
      bool pass = stage == 14 && earlyRewardPass && pausedPass && rewardPass && defeatPass &&
                  resetPass && delivered == 1 && applied == 1;
      std::ofstream out(artifacts / "expedition-result.json");
      out << "{\n  \"passed\": " << (pass ? "true" : "false")
          << ",\n  \"terminal_freezes_simulation\": " << (pausedPass ? "true" : "false")
          << ",\n  \"shipments\": " << delivered << ",\n  \"upgrades_selected\": " << applied
          << ",\n  \"earned_loads\": " << earnedLoads
          << ",\n  \"reward_before_drone_departure\": " << (earlyRewardPass ? "true" : "false")
          << ",\n  \"card_applied\": " << (rewardPass ? "true" : "false")
          << ",\n  \"defeat_freezes_simulation\": " << (defeatPass ? "true" : "false")
          << ",\n  \"restart_clears_run\": " << (resetPass ? "true" : "false") << "\n}\n";
      std::cout << "EXPEDITION " << (pass ? "PASS" : "FAIL") << " stage=" << stage
                << " terminal=" << pausedPass << " rewards=" << rewardPass
                << " defeat=" << defeatPass << " reset=" << resetPass << '\n';
      return pass ? 0 : 1;
    }
    if (test) {
      bool dead = fixture >= 0 && session->world.foes[fixture].hp <= 0;
      // A killed slot may be reused by slow spawning; validate eliminations as well.
      bool hit = session->impacts > 0 && session->world.kills > 0;
      bool pass = stage == 9 && raisedPass && returnedPass && pausedPass && noBuildPass && hit &&
                  session->module.kind == (testMini ? Gun::Crank : Gun::Pulse) &&
                  session->shots >= (testMini ? 20u : 1u);
      std::ofstream out(artifacts / "result.json");
      out << "{\n  \"passed\": " << (pass ? "true" : "false") << ",\n  \"weapon\": \""
          << (testMini ? "minigun" : "laser")
          << "\",\n  \"raised_above_roof\": " << (raisedPass ? "true" : "false")
          << ",\n  \"returned_to_deck\": " << (returnedPass ? "true" : "false")
          << ",\n  \"pause_stops_combat\": " << (pausedPass ? "true" : "false")
          << ",\n  \"building_disabled\": " << (noBuildPass ? "true" : "false")
          << ",\n  \"shots\": " << session->shots << ",\n  \"hits\": " << session->impacts
          << ",\n  \"kills\": " << session->world.kills
          << ",\n  \"fixture_dead\": " << (dead ? "true" : "false") << "\n}\n";
      std::cout << "COMMAND " << (pass ? "PASS" : "FAIL") << " stage=" << stage
                << " lift=" << raisedPass << " return=" << returnedPass << " pause=" << pausedPass
                << " shots=" << session->shots << " hits=" << session->impacts
                << " kills=" << session->world.kills << '\n';
      return pass ? 0 : 1;
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    if (!test)
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RDZEŃ - błąd", e.what(), nullptr);
    return 1;
  }
}
} // namespace command
