#pragma once
#include "gameplay.hpp"
#include "expedition.hpp"
#include "frontier.hpp"
#include "services.hpp"
#include "manual_arsenal.hpp"

namespace command {
using namespace arsenal;
enum class Mode { Selection, Walking, Raising, Mounted, Lowering };
struct Bullet {
  Vec3 pos, previous, velocity;
  float life, damage;
};
struct RayHit {
  int id = -1;
  float distance;
  Vec3 point;
};
struct Input {
  float side = 0, forward = 0;
  bool fast = false, fire = false;
};

inline RayHit trace(const Game& g, Vec3 origin, Vec3 direction, float range) {
  direction = normalize(direction);
  RayHit result{-1, range, origin + direction * range};
  // Limit to ground first, then query only cells touched by the ray's bounding disc.
  if (direction.y < -.00001f && origin.y > 0)
    result.distance = std::min(result.distance, -origin.y / direction.y);
  Vec3 middle = origin + direction * (result.distance * .5f);
  float radius = std::hypot(direction.x, direction.z) * result.distance * .5f + 16;
  g.grid.radius(middle.x, middle.z, radius, g.foes, [&](int i) {
    const auto& foe = g.foes[i];
    Vec3 delta = origin - g.targetPoint(i);
    float r = foe.size * .43f;
    float b = dot(delta, direction), c = dot(delta, delta) - r * r;
    float discriminant = b * b - c;
    if (discriminant < 0)
      return;
    float t = std::max(0.f, -b - std::sqrt(discriminant));
    if (-b + std::sqrt(discriminant) < 0)
      return;
    if (t < result.distance) {
      result.id = i;
      result.distance = t;
    }
  });
  result.point = origin + direction * result.distance;
  return result;
}

class Session {
public:
  static constexpr float ROOF = CORE_HEIGHT + 5.5f, LIFT = 10.f;
  static constexpr float DECK_RADIUS = CORE_RADIUS + .3f, RAIL_RADIUS = DECK_RADIUS - .15f;
  static constexpr float WALK_RADIUS = RAIL_RADIUS - .4f;
  static constexpr float ROOF_RADIUS = DECK_RADIUS, HATCH_RADIUS = 3.8f;
  static constexpr float EAVE = CORE_HEIGHT + 3.4f;
  static float roofHeight(float radius) {
    float t = std::clamp((radius - HATCH_RADIUS) / (ROOF_RADIUS - HATCH_RADIUS), 0.f, 1.f);
    return ROOF + (EAVE - ROOF) * t;
  }
  Game world{false};
  Expedition expedition;
  Frontier frontier;
  ServiceSystem services;
  bool terminalOpen = false;
  bool servicesOpen = false;
  Service serviceTarget = Service::None;
  float deckClock = 0, buttonAge = 3;
  static constexpr Vec3 START_BUTTON{0, CORE_HEIGHT + 1.2f, 8.3f};
  bool tactical = false;
  int sentryStock = 0;
  std::deque<Weapon> sentryInventory;
  std::vector<SentryDrop> sentryDrops;
  GunModule module;
  Mode mode = Mode::Selection;
  Vec3 player{0, CORE_HEIGHT + 1.72f, -9}, returnPosition = player, entryEye = player;
  float yaw = 0, pitch = -.08f, gunYaw = 0, gunPitch = -.2f;
  float pendingYaw = 0, pendingPitch = 0;
  float lift = 0, hatch = 0, transition = 0, cooldown = 0, recoil = 0, spin = 0, spinSpeed = 0;
  float hitFlash = 0, spawnTimer = 3, shotFlash = 0;
  float flatDamage = 0, frostClock = 0;
  uint64_t shots = 0, impacts = 0;
  std::vector<Bullet> bullets;
  std::vector<ManualProjectile> manualProjectiles;
  std::vector<ToxicPatch> toxicPatches;
  int smartTarget = -1;
  uint32_t smartGeneration = 0;
  float smartClock = 0, lanceCharge = 0, lanceRemaining = 0, lanceFxClock = 0;
  static constexpr float SMART_HALF_X = .24f, SMART_HALF_Y = .15f;
  Random random;
  explicit Session(bool gameplay = false, bool running = false) : expedition(gameplay, running) {
    world.autoWaves = false;
    world.recordDeaths = world.expeditionRules = gameplay;
    bullets.reserve(256);
    manualProjectiles.reserve(64);
    toxicPatches.reserve(32);
    sentryDrops.reserve(SLOT_COUNT);
  }
  bool seated() const {
    return mode == Mode::Raising || mode == Mode::Mounted || mode == Mode::Lowering;
  }
  bool placingSentry() const { return tactical && sentryStock > 0; }
  bool blocked() const {
    return world.paused || terminalOpen || servicesOpen || serviceTarget != Service::None ||
           expedition.dead() || placingSentry();
  }
  bool nearConsole(float side) const {
    if (mode != Mode::Walking || !expedition.enabled || tactical)
      return false;
    Vec3 d = Vec3{side * 6.8f, player.y, -3.5f} - player;
    return length(d) < 3.3f && dot(normalize(d), forward()) > .35f;
  }
  bool nearTerminal() const { return nearConsole(-1); }
  bool nearServices() const { return nearConsole(1); }
  bool nearStartButton() const {
    Vec3 delta = START_BUTTON - player;
    return expedition.enabled && !expedition.miningStarted && mode == Mode::Walking && !tactical &&
           length(delta) < 2.8f && dot(normalize(delta), forward()) > .4f;
  }
  bool chooseUpgrade(int index) {
    if (!terminalOpen || expedition.dead() || expedition.rewards.empty() || index < 0 || index >= 3)
      return false;
    Upgrade kind = expedition.rewards.front().choices[index];
    if (!expedition.eligible(kind)) {
      expedition.validateFront();
      return false;
    }
    Rarity rarity = expedition.rewards.front().rarities[index];
    float power = RARITIES[int(rarity)].power;
    if (isTower(kind)) {
      ++sentryStock;
      sentryInventory.push_back(towerWeapon(kind));
      tactical = true;
      expedition.towerUnlocked[towerWeapon(kind)] = true;
    }
    if (kind == Upgrade::Damage)
      module.damageMultiplier += .25f * power;
    if (kind == Upgrade::FireRate)
      module.rateMultiplier += .2f * power;
    if (kind == Upgrade::Mining)
      expedition.miningMultiplier += .25f * power;
    if (kind == Upgrade::Hull) {
      expedition.maxHp += 250 * power;
      expedition.hp = std::min(expedition.maxHp, expedition.hp + 250 * power);
    }
    if (kind == Upgrade::TowerDamage)
      world.towerDamage += .2f * power;
    if (kind == Upgrade::TowerRate)
      world.towerRate += .15f * power;
    if (kind == Upgrade::TowerRange)
      world.towerRange += .15f * power;
    if (kind == Upgrade::FlatDamage)
      flatDamage += 1;
    if (kind == Upgrade::Luck)
      ++expedition.luck;
    if (kind == Upgrade::Frost)
      world.frost = true;
    if (kind == Upgrade::Focus)
      world.focusDamageBonus = int(rarity) * .1f;
    if (kind == Upgrade::RoverBatch)
      expedition.roverBatch = std::min(128, expedition.roverBatch + int(rarity) + 1);
    if (kind == Upgrade::RoverRate)
      expedition.roverRate = std::min(24.f, expedition.roverRate + (int(rarity) + 1) * .1f);
    if (kind == Upgrade::ServiceCredit)
      ++expedition.servicePoints;
    ++expedition.levels[size_t(kind)];
    ++expedition.selected;
    expedition.rewards.pop_front();
    terminalOpen = false;
    world.announce(isTower(kind) ? "Wybierz wolny heks. LPM zamawia zrzut, Tab wraca na pokład."
                                 : std::string("Zainstalowano: ") + UPGRADES[size_t(kind)].name);
    return true;
  }
  bool chooseService(int choice) {
    if (!servicesOpen || !expedition.miningStarted || expedition.dead() ||
        expedition.servicePoints <= 0 || choice < 0 || choice >= int(Service::Count))
      return false;
    auto kind = Service(choice);
    if (kind == Service::StunWave) {
      if (!services.order(expedition, kind, {0, 0, 0}))
        return false;
    } else {
      serviceTarget = kind;
      tactical = true;
    }
    servicesOpen = false;
    cancelAim();
    return true;
  }
  bool targetService(Vec3 point, int slot = -1) {
    if (serviceTarget == Service::None || expedition.dead() || servicesOpen || terminalOpen)
      return false;
    uint32_t id = 0;
    if (serviceTarget == Service::Overload) {
      if (slot < 0 || slot >= SLOT_COUNT)
        return false;
      const auto& p = TURRET_SLOTS[slot];
      for (const auto& tower : world.towers)
        if (std::hypot(tower.x - p.x, tower.z - p.z) < .5f) {
          id = tower.id;
          point = {tower.x, 0, tower.z};
          break;
        }
      if (!id)
        return false;
    }
    if (!services.order(expedition, serviceTarget, point, id))
      return false;
    world.announce(std::string(SERVICES[int(serviceTarget)].name) + ": aktywacja za 3 s.");
    serviceTarget = Service::None;
    return true;
  }
  void cancelService() {
    serviceTarget = Service::None;
    tactical = false;
    servicesOpen = true;
  }
  Weapon pendingSentry() const { return sentryInventory.empty() ? Laser : sentryInventory.front(); }
  void markTarget(int id) {
    if (expedition.levels[size_t(Upgrade::Focus)] > 0 && id >= 0 && world.foes[id].hp > 0) {
      world.focusTarget = id;
      world.focusGeneration = world.foes[id].generation;
      world.focusTime = 3;
      world.ring(world.targetPoint(id), {1, .55f, .2f}, 3, .35f);
    }
  }
  bool slotAvailable(int slot) const {
    if (slot < 0 || slot >= SLOT_COUNT)
      return false;
    const auto& p = TURRET_SLOTS[slot];
    if (world.canBuild(p.x, p.z))
      return false;
    for (const auto& drop : sentryDrops)
      if (drop.slot == slot && !drop.installed)
        return false;
    return true;
  }
  int committedSlots() const {
    int count = 0;
    for (int i = 0; i < SLOT_COUNT; ++i)
      if (!slotAvailable(i))
        ++count;
    return count;
  }
  bool orderSentry(int slot) {
    if (!expedition.enabled || expedition.dead() || terminalOpen || mode == Mode::Selection ||
        servicesOpen || serviceTarget != Service::None || sentryStock <= 0 || !slotAvailable(slot))
      return false;
    Weapon type = pendingSentry();
    sentryDrops.push_back({slot, 0, false, type});
    if (!sentryInventory.empty())
      sentryInventory.pop_front();
    --sentryStock;
    world.announce(std::string("Zrzut: ") + WEAPONS[type].name + ". Czas do lądowania: 6 s.");
    return true;
  }
  void updateSentryDrops(float dt) {
    for (size_t i = 0; i < sentryDrops.size();) {
      auto& drop = sentryDrops[i];
      drop.age += dt;
      if (!drop.installed && drop.age >= SentryDrop::LAND_TIME) {
        const auto& p = TURRET_SLOTS[drop.slot];
        if (!world.addTower(drop.type, p.x, p.z)) {
          ++sentryStock;
          sentryInventory.push_front(drop.type);
          sentryDrops.erase(sentryDrops.begin() + i);
          world.announce("Zrzut przerwany. Wieżyczka wróciła do magazynu.");
          continue;
        }
        drop.installed = true;
        world.ring({p.x, .12f, p.z}, {.5f, .85f, 1}, 4, .7f);
        world.announce(std::string(WEAPONS[drop.type].name) +
                       " na stanowisku. Balon wraca na orbitę.");
      }
      if (drop.age >= SentryDrop::END_TIME)
        sentryDrops.erase(sentryDrops.begin() + i);
      else
        ++i;
    }
  }
  Vec3 forward() const {
    return {std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)};
  }
  float baseHeight() const { return CORE_HEIGHT + .35f + lift * LIFT; }
  Vec3 seatEye() const {
    // Keep the sight line above the receiver for the entire elevation range.
    return gunPoint({0, 1.f, -1.15f});
  }
  Vec3 eye() const {
    if (!seated())
      return player;
    if (mode == Mode::Raising) {
      float t = std::clamp(transition / .4f, 0.f, 1.f);
      t = t * t * (3 - 2 * t);
      return entryEye * (1 - t) + seatEye() * t;
    }
    return seatEye();
  }
  Vec3 gunPoint(Vec3 local) const {
    float cp = std::cos(gunPitch), sp = std::sin(gunPitch);
    Vec3 p{local.x, local.y * cp + local.z * sp, -local.y * sp + local.z * cp};
    return {p.x * std::cos(gunYaw) + p.z * std::sin(gunYaw), baseHeight() + 1.15f + p.y,
            -p.x * std::sin(gunYaw) + p.z * std::cos(gunYaw)};
  }
  Vec3 muzzle() const {
    if (module.kind == Gun::Howitzer || module.kind == Gun::Toxin)
      return gunPoint({1.5f, .12f, 3.2f});
    if (module.kind == Gun::Smart)
      return gunPoint({shots % 2 ? -1.15f : 1.15f, .1f, 2.5f});
    if (module.kind == Gun::Flame)
      return gunPoint({.9f, -.15f, 3.2f});
    return gunPoint({0, 0, 3.2f});
  }
  bool nearSeat() const {
    Vec3 d = Vec3{0, player.y, 0} - player;
    return mode == Mode::Walking && !tactical && length(d) <= 4.6f &&
           dot(normalize(d), forward()) > .45f;
  }
  void start(Gun gun) {
    module.kind = gun;
    mode = Mode::Walking;
    world.autoWaves = false;
    world.announcementUntil = 0;
    if (expedition.enabled && !expedition.miningStarted) {
      world.announce("Uruchom wydobycie: podejdź do czerwonego przycisku za działem i naciśnij E.");
      return;
    }
    deployInitialFoes();
  }
  void deployInitialFoes() {
    for (int i = 0; i < 15; ++i) {
      // Initial targets are visible ahead of the gun, with a few flankers.
      float a = i < 10 ? (i - 4.5f) * .105f : i * 2.39996f;
      float r = 85 + random.unit() * 75;
      int id = world.spawn(Crawler, std::sin(a) * r, std::cos(a) * r);
      if (id >= 0)
        world.foes[id].speed =
            expedition.enabled ? 1.8f + random.unit() * .6f : 2.3f + random.unit() * 1.2f;
    }
    world.grid.rebuild(world.foes, world.active);
  }
  bool startExtraction() {
    if (!expedition.enabled || expedition.miningStarted || mode == Mode::Selection ||
        expedition.dead())
      return false;
    expedition.miningStarted = true;
    buttonAge = 0;
    deployInitialFoes();
    world.announce("Wydobycie uruchomione. Lewa konsola: ulepszenia. Prawa: jednorazowe usługi.");
    return true;
  }
  bool interact() {
    if (servicesOpen) {
      servicesOpen = false;
      return true;
    }
    if (terminalOpen) {
      terminalOpen = false;
      return true;
    }
    if (expedition.dead())
      return false;
    if (tactical)
      return false;
    if (nearStartButton())
      return startExtraction();
    if (nearTerminal()) {
      expedition.validateFront();
      terminalOpen = true;
      cancelAim();
      return true;
    }
    if (nearServices()) {
      servicesOpen = true;
      cancelAim();
      return true;
    }
    if (nearSeat()) {
      returnPosition = player;
      entryEye = player;
      mode = Mode::Raising;
      transition = 0;
      yaw = gunYaw;
      pitch = gunPitch;
      cancelAim();
      return true;
    }
    if (mode == Mode::Mounted || mode == Mode::Raising) {
      mode = Mode::Lowering;
      transition = 0;
      return true;
    }
    return false;
  }
  void look(float dx, float dy) {
    if (mode == Mode::Selection || blocked() || tactical)
      return;
    if (seated()) {
      pendingYaw += dx;
      pendingPitch = std::clamp(pitch + pendingPitch + dy, -1.12f, .65f) - pitch;
    } else {
      yaw += dx;
      pitch = std::clamp(pitch + dy, -1.35f, 1.35f);
    }
  }
  void cancelAim() { pendingYaw = pendingPitch = 0; }
  void updateAim(float dt) {
    if (!seated() || blocked() || tactical || dt <= 0)
      return;
    // Render-rate exponential response: light inertia (~75 ms), no overshoot or spring wobble.
    // Camera, barrel and shot direction share the same actual orientation.
    float response = -std::expm1(-dt / .075f);
    float dx = pendingYaw * response, dy = pendingPitch * response;
    yaw += dx;
    pitch += dy;
    pendingYaw -= dx;
    pendingPitch -= dy;
    gunYaw = yaw;
    gunPitch = pitch;
  }
  void move(const Input& input, float dt) {
    if (mode != Mode::Walking || tactical)
      return;
    float n = std::max(1.f, std::hypot(input.side, input.forward));
    float d = dt * (input.fast ? 7.f : 4.5f) / n;
    player.x += (std::sin(yaw) * input.forward + std::cos(yaw) * input.side) * d;
    player.z += (std::cos(yaw) * input.forward - std::sin(yaw) * input.side) * d;
    float r = std::hypot(player.x, player.z);
    if (r > WALK_RADIUS) {
      player.x *= WALK_RADIUS / r;
      player.z *= WALK_RADIUS / r;
    }
    if (r < 2.9f) {
      if (r < .001f)
        player.z = -2.9f;
      else {
        player.x *= 2.9f / r;
        player.z *= 2.9f / r;
      }
    }
    // The eight canopy columns are solid, as are the command consoles.
    for (int i = 0; i < 8; ++i) {
      float a = i * PI / 4, x = std::sin(a) * 11.8f, z = std::cos(a) * 11.8f;
      float dx = player.x - x, dz = player.z - z, distance = std::hypot(dx, dz);
      if (distance < .85f) {
        if (distance < .001f) {
          dx = 1;
          dz = 0;
          distance = 1;
        }
        player.x = x + dx / distance * .85f;
        player.z = z + dz / distance * .85f;
      }
    }
    for (float side : {-1.f, 1.f}) {
      float cx = side * 6.8f, cz = -3.5f;
      if (std::abs(player.x - cx) < 1.9f && std::abs(player.z - cz) < 1.1f) {
        float dx = player.x - cx, dz = player.z - cz;
        if (1.9f - std::abs(dx) < 1.1f - std::abs(dz))
          player.x = cx + (dx < 0 ? -1.9f : 1.9f);
        else
          player.z = cz + (dz < 0 ? -1.1f : 1.1f);
      }
    }
    if (expedition.enabled && (!expedition.miningStarted || buttonAge < 1.6f)) {
      float dx = player.x - START_BUTTON.x, dz = player.z - START_BUTTON.z;
      float d = std::hypot(dx, dz);
      if (d < .85f) {
        if (d < .001f) {
          dx = 1;
          dz = 0;
          d = 1;
        }
        player.x = START_BUTTON.x + dx / d * .85f;
        player.z = START_BUTTON.z + dz / d * .85f;
      }
    }
  }
#include "manual_combat.inl"
  void fire() {
    const auto& s = module.spec();
    if (mode != Mode::Mounted || cooldown > .00001f || blocked() || tactical ||
        module.kind == Gun::Lance)
      return;
    if (module.kind == Gun::Smart) {
      updateSmartTarget();
      if (!world.valid(smartTarget, smartGeneration))
        return;
    }
    if ((module.kind == Gun::Howitzer || module.kind == Gun::Toxin || module.kind == Gun::Smart) &&
        manualProjectiles.size() >= 64)
      return;
    cooldown = std::max(-.05f, cooldown) + s.interval / std::max(.1f, module.rateMultiplier);
    recoil = 1;
    shotFlash = .09f;
    ++shots;
    ++world.fired[s.audio];
    Vec3 direction = forward();
    if (module.kind == Gun::Pulse) {
      RayHit hit = trace(world, eye(), direction, s.range);
      world.beam(muzzle(), hit.point, s.color, .12f, .16f);
      world.beam(muzzle(), hit.point, {1, 1, 1}, .035f, .1f);
      world.sparks(hit.point, s.color, 6, 4);
      if (hit.id >= 0) {
        markTarget(hit.id);
        world.hurt(hit.id, s.damage * module.damageMultiplier + flatDamage, Laser);
        ++impacts;
        hitFlash = .16f;
      }
    } else if (module.kind == Gun::Arc) {
      fireArc();
    } else if (module.kind == Gun::Rail) {
      fireRail();
    } else if (module.kind == Gun::Flame) {
      fireFlame();
    } else if (module.kind == Gun::Howitzer || module.kind == Gun::Toxin ||
               module.kind == Gun::Smart) {
      Vec3 endpoint = module.kind == Gun::Smart ? world.targetPoint(smartTarget) : groundAim();
      ManualProjectile p{module.kind, muzzle(), muzzle(), muzzle(), endpoint, {}};
      p.damage = manualDamage();
      p.duration = std::clamp(length(endpoint - p.start) / s.speed, .6f, 5.f);
      if (module.kind == Gun::Smart) {
        p.target = smartTarget;
        p.generation = smartGeneration;
        Vec3 right = normalize(cross({0, 1, 0}, direction));
        p.velocity =
            normalize(direction + right * (shots % 2 ? -.7f : .7f) + Vec3{0, .3f, 0}) * s.speed;
      }
      manualProjectiles.push_back(p);
    } else {
      Vec3 right = normalize(cross({0, 1, 0}, direction)), up = cross(direction, right);
      float a = random.unit() * 2 * PI,
            radius = std::sqrt(random.unit()) * s.spread * module.spreadMultiplier;
      // Converge on the center sight, then perturb direction inside a cone. No target lock.
      Vec3 aim = trace(world, eye(), direction, s.range).point;
      Vec3 d = normalize(normalize(aim - muzzle()) + right * (std::cos(a) * radius) +
                         up * (std::sin(a) * radius));
      if (bullets.size() < 256)
        bullets.push_back({muzzle(), muzzle(), d * s.speed, s.range / s.speed,
                           s.damage * module.damageMultiplier + flatDamage});
      world.particle(gunPoint({.7f, -.25f, 0}), right * 3 + Vec3{0, 2, 0}, {1, .7f, .25f}, .09f,
                     .6f);
    }
  }
  void step(float dt, const Input& input = {}) {
    if (mode == Mode::Selection || blocked() || dt <= 0)
      return;
    dt = std::min(dt, .05f);
    deckClock += dt;
    if (expedition.miningStarted)
      buttonAge = std::min(3.f, buttonAge + dt);
    move(input, dt);
    transition += dt;
    if (mode == Mode::Raising) {
      hatch = std::min(1.f, transition / .6f);
      float t = std::clamp((transition - .6f) / 2.8f, 0.f, 1.f);
      lift = t * t * (3 - 2 * t);
      if (t >= 1)
        mode = Mode::Mounted;
    } else if (mode == Mode::Lowering) {
      lift = std::max(0.f, lift - dt / 2.8f);
      if (lift <= 0) {
        hatch = std::max(0.f, hatch - dt / .6f);
        if (hatch <= 0) {
          mode = Mode::Walking;
          player = returnPosition;
          pitch = -.08f;
          cancelAim();
        }
      }
    }
    cooldown -= dt;
    if (!input.fire || mode != Mode::Mounted)
      cooldown = std::max(0.f, cooldown);
    recoil = std::max(0.f, recoil - dt * 5);
    shotFlash = std::max(0.f, shotFlash - dt);
    hitFlash = std::max(0.f, hitFlash - dt);
    float targetSpeed = input.fire && mode == Mode::Mounted ? 28.f : 0;
    spinSpeed += (targetSpeed - spinSpeed) * std::min(1.f, dt * 8);
    spin += spinSpeed * dt;
    if (expedition.enabled && !expedition.miningStarted)
      return;
    float damageBefore = world.coreDamage;
    services.step(world, expedition, dt);
    stepToxicPatches(dt);
    world.step(dt);
    size_t rewardsBefore = expedition.rewards.size();
    expedition.step(dt, world.coreDamage - damageBefore);
    if (expedition.levels[size_t(Upgrade::Purge)] > 0 && !expedition.purgeUsed &&
        expedition.hp <= expedition.maxHp * .5f) {
      expedition.purgeUsed = true;
      for (int id : world.active) {
        auto& foe = world.foes[id];
        if (foe.hp > 0)
          world.hurt(id, foe.type >= Titan ? foe.maxHp * .5f : foe.hp, Tesla);
      }
      world.ring({0, .8f, 0}, {1, .8f, .35f}, 400, 2.5f);
      world.announce("AWARYJNE OCZYSZCZENIE. Jednorazowy ładunek zużyty.");
    }
    if (expedition.dead()) {
      terminalOpen = false;
      servicesOpen = false;
      serviceTarget = Service::None;
      cancelAim();
      return;
    }
    updateSentryDrops(dt);
    frostClock -= dt;
    if (world.frost && frostClock <= 0) {
      frostClock = 3;
      world.ring({0, .25f, 0}, {.3f, .75f, 1}, 85, 2);
    }
    if (expedition.rewards.size() > rewardsBefore)
      world.announce("Wypłata gotowa. Podejdź do komputera i naciśnij E.");
    smartClock -= dt;
    if (module.kind == Gun::Smart && mode == Mode::Mounted && !tactical && smartClock <= 0) {
      updateSmartTarget();
      smartClock = .08f;
    }
    stepLance(dt, input.fire);
    if (input.fire)
      fire();
    stepManualProjectiles(dt);
    for (size_t i = 0; i < bullets.size();) {
      auto& b = bullets[i];
      b.previous = b.pos;
      Vec3 displacement = b.velocity * dt;
      RayHit hit = trace(world, b.pos, displacement, length(displacement));
      b.pos = hit.point;
      b.velocity.y -= 1.5f * dt;
      b.life -= dt;
      if (hit.id >= 0) {
        markTarget(hit.id);
        world.hurt(hit.id, b.damage, Minigun);
        ++impacts;
        hitFlash = .12f;
        world.sparks(hit.point, module.spec().color, 3, 3);
      }
      if (hit.id >= 0 || b.life <= 0 || b.pos.y <= .01f) {
        bullets[i] = bullets.back();
        bullets.pop_back();
      } else
        ++i;
    }
    if (expedition.enabled) {
      frontier.step(world, expedition, dt, spawnTimer);
      return;
    }
    spawnTimer -= dt;
    if (spawnTimer <= 0 && world.active.size() < 160) {
      spawnTimer = 2.8f;
      float a = random.unit() * 2 * PI, r = 160 + random.unit() * 100;
      int id = world.spawn(Crawler, std::sin(a) * r, std::cos(a) * r);
      if (id >= 0)
        world.foes[id].speed = 2.3f + random.unit() * 1.2f;
    }
  }
};
} // namespace command
