// Session methods: bounded projectile pools and spatial queries shared with the simulation.
float manualDamage() const {
  return module.spec().damage * module.damageMultiplier + flatDamage;
}
Vec3 groundAim() const {
  Vec3 point = trace(world, eye(), forward(), module.spec().range).point;
  point.y = .2f;
  return point;
}
void updateSmartTarget() {
  smartTarget = -1;
  smartGeneration = 0;
  Vec3 f = forward(), right = normalize(cross({0, 1, 0}, f)), up = cross(f, right);
  float best = 1e30f;
  world.grid.radius(eye().x, eye().z, module.spec().range, world.foes, [&](int id) {
    Vec3 delta = world.targetPoint(id) - eye();
    float depth = dot(delta, f);
    if (depth < 1 || length(delta) > module.spec().range)
      return;
    float x = dot(delta, right) / depth, y = dot(delta, up) / depth;
    if (std::abs(x) > SMART_HALF_X || std::abs(y) > SMART_HALF_Y)
      return;
    float score = x * x + y * y;
    if (score < best) {
      best = score;
      smartTarget = id;
      smartGeneration = world.foes[id].generation;
    }
  });
}
void manualHit(int id, float damage, Weapon source, bool focus = true) {
  if (id < 0 || world.foes[id].hp <= 0)
    return;
  if (focus)
    markTarget(id);
  world.hurt(id, damage, source);
  ++impacts;
  hitFlash = .14f;
}
void manualBlast(Vec3 point, float radius, float damage, Weapon source, int primary = -1) {
  markTarget(primary >= 0 && world.foes[primary].hp > 0
                 ? primary
                 : world.grid.nearest(point.x, point.z, radius, world.foes));
  world.grid.radius(point.x, point.z, radius, world.foes, [&](int id) {
    // A ground blast reaches tall enemies but not a distant airborne centre.
    if (length(world.targetPoint(id) - point) <= radius + world.foes[id].size * .25f)
      manualHit(id, damage, source, false);
  });
  world.ring(point, WEAPONS[source].color, radius, .65f);
  world.sparks(point + Vec3{0, 1, 0}, WEAPONS[source].color, 28, 28);
}
void fireArc() {
  auto hit = trace(world, eye(), forward(), module.spec().range);
  std::array<int, 10> visited{};
  int count = 0, id = hit.id;
  Vec3 from = muzzle();
  while (id >= 0 && count < int(visited.size())) {
    Vec3 to = world.targetPoint(id);
    Vec3 mid = (from + to) * .5f;
    mid = mid + Vec3{(random.unit() - .5f) * 2, 1 + random.unit(), (random.unit() - .5f) * 2};
    world.beam(from, mid, module.spec().color, .18f, .22f);
    world.beam(mid, to, module.spec().color, .18f, .22f);
    world.beam(from, to, {1, 1, 1}, .045f, .12f);
    visited[count++] = id;
    manualHit(id, manualDamage(), Tesla, count == 1);
    from = to;
    id = -1;
    float best = 28 * 28;
    world.grid.radius(from.x, from.z, 28, world.foes, [&](int candidate) {
      if (std::find(visited.begin(), visited.begin() + count, candidate) != visited.begin() + count)
        return;
      Vec3 d = world.targetPoint(candidate) - from;
      float distance = dot(d, d);
      if (distance < best) {
        best = distance;
        id = candidate;
      }
    });
  }
  if (!count)
    world.beam(muzzle(), hit.point, module.spec().color, .1f, .15f);
}
void fireRail() {
  Vec3 origin = eye(), direction = forward();
  float range = module.spec().range;
  if (direction.y < -.00001f)
    range = std::min(range, -origin.y / direction.y);
  std::array<RayHit, 8> hits;
  for (auto& hit : hits)
    hit = {-1, range, {}};
  Vec3 mid = origin + direction * (range * .5f);
  world.grid.radius(mid.x, mid.z, std::hypot(direction.x, direction.z) * range * .5f + 16,
                    world.foes, [&](int id) {
                      Vec3 delta = origin - world.targetPoint(id);
                      float r = world.foes[id].size * .43f, b = dot(delta, direction);
                      float discriminant = b * b - dot(delta, delta) + r * r;
                      if (discriminant < 0 || -b + std::sqrt(discriminant) < 0)
                        return;
                      float distance = std::max(0.f, -b - std::sqrt(discriminant));
                      for (int k = 0; k < int(hits.size()); ++k)
                        if (distance < hits[k].distance) {
                          for (int j = int(hits.size()) - 1; j > k; --j)
                            hits[j] = hits[j - 1];
                          hits[k] = {id, distance, origin + direction * distance};
                          break;
                        }
                    });
  bool primary = true;
  for (const auto& hit : hits)
    if (hit.id >= 0) {
      float distance = length(world.targetPoint(hit.id) - origin);
      manualHit(hit.id, manualDamage() * (1 + distance * .01f), Sniper, primary);
      primary = false;
      world.sparks(hit.point, module.spec().color, 8, 8);
    }
  world.beam(muzzle(), origin + direction * range, module.spec().color, .16f, .25f);
  world.beam(muzzle(), origin + direction * range, {1, 1, 1}, .05f, .15f);
}
void fireFlame() {
  Vec3 f = forward();
  auto hit = trace(world, eye(), f, 90);
  if (hit.id >= 0 &&
      std::hypot(world.foes[hit.id].x - eye().x, world.foes[hit.id].z - eye().z) <= 63)
    markTarget(hit.id);
  world.grid.radius(eye().x, eye().z, 63, world.foes, [&](int id) {
    Vec3 d = world.targetPoint(id) - eye();
    if (dot(normalize(d), f) >= std::cos(.5f)) {
      manualHit(id, manualDamage() * module.spec().interval, arsenal::Flame, false);
      auto& foe = world.foes[id];
      foe.burn = std::max(foe.burn, 3.f);
      foe.burnDps = std::max(foe.burnDps, 30 * module.damageMultiplier);
    }
  });
  Vec3 right = normalize(cross({0, 1, 0}, f)), up = cross(f, right);
  for (int i = 0; i < 14; ++i) {
    float a = random.unit() * 2 * PI, spread = std::sqrt(random.unit()) * .48f;
    Vec3 d = normalize(f + right * (std::cos(a) * spread) + up * (std::sin(a) * spread));
    world.particle(muzzle() + d * (random.unit() * 8), d * 105,
                   i % 3 ? module.spec().color : Color{1, .8f, .2f}, 1 + random.unit() * 3, .65f);
  }
}
void stepToxicPatches(float dt) {
  for (size_t i = 0; i < toxicPatches.size();) {
    auto& patch = toxicPatches[i];
    float tick = std::min(dt, patch.life);
    patch.life -= tick;
    patch.tick += tick;
    if (patch.tick >= .1f || patch.life <= .00001f) {
      world.grid.radius(patch.position.x, patch.position.z, 14, world.foes, [&](int id) {
        if (world.foes[id].altitude <= 6) {
          world.foes[id].toxicSlow = .16f;
          manualHit(id, patch.damage * patch.tick, Venom, false);
        }
      });
      patch.tick = 0;
    }
    if (patch.life <= .00001f) {
      toxicPatches[i] = toxicPatches.back();
      toxicPatches.pop_back();
    } else
      ++i;
  }
}
void stepManualProjectiles(float dt) {
  for (size_t i = 0; i < manualProjectiles.size();) {
    auto& p = manualProjectiles[i];
    p.age += dt;
    p.previous = p.pos;
    bool impact = false;
    if (p.kind == Gun::Smart) {
      if (world.valid(p.target, p.generation))
        p.endpoint = world.targetPoint(p.target);
      Vec3 delta = p.endpoint - p.pos;
      float distance = length(delta), speed = GUNS[int(Gun::Smart)].speed;
      Vec3 desired = normalize(delta) * speed;
      float blend = -std::expm1(-dt * 8);
      p.velocity = normalize(p.velocity * (1 - blend) + desired * blend) * speed;
      float contact = world.valid(p.target, p.generation) ? world.foes[p.target].size * .43f : .3f;
      if (distance <= speed * dt + contact) {
        p.pos = p.endpoint;
        impact = true;
      } else if (p.age >= 6) {
        impact = true;
      } else
        p.pos = p.pos + p.velocity * dt;
    } else {
      float t = std::min(1.f, p.age / p.duration);
      p.pos = p.start * (1 - t) + p.endpoint * t;
      p.pos.y += std::sin(t * PI) * (p.kind == Gun::Toxin ? 25 : 40);
      impact = t >= 1;
    }
    if (impact) {
      if (p.kind == Gun::Toxin) {
        if (toxicPatches.size() < 32)
          toxicPatches.push_back({p.endpoint, 8, 0, p.damage});
        markTarget(world.grid.nearest(p.endpoint.x, p.endpoint.z, 14, world.foes));
        world.ring(p.endpoint, GUNS[int(Gun::Toxin)].color, 14, .5f);
        world.sparks(p.endpoint, GUNS[int(Gun::Toxin)].color, 16, 12);
      } else
        manualBlast(p.pos, p.kind == Gun::Smart ? 9 : 15, p.damage,
                    p.kind == Gun::Smart ? Rocket : Mortar,
                    world.valid(p.target, p.generation) ? p.target : -1);
      manualProjectiles[i] = manualProjectiles.back();
      manualProjectiles.pop_back();
    } else
      ++i;
  }
}
void stepLance(float dt, bool held) {
  if (module.kind != Gun::Lance)
    return;
  if (!held || mode != Mode::Mounted || tactical) {
    lanceCharge = 0;
    if (lanceRemaining > 0)
      cooldown = module.spec().interval / module.rateMultiplier;
    lanceRemaining = 0;
    return;
  }
  if (cooldown > 0)
    return;
  if (lanceRemaining <= 0) {
    lanceCharge = std::min(1.8f, lanceCharge + dt * module.rateMultiplier);
    if (lanceCharge < 1.8f)
      return;
    lanceRemaining = 3;
    lanceCharge = 0;
    lanceFxClock = 0;
    ++shots;
    ++world.fired[arsenal::Lance];
  }
  float tick = std::min(dt, lanceRemaining);
  auto hit = trace(world, eye(), forward(), module.spec().range);
  if (hit.id >= 0)
    manualHit(hit.id, manualDamage() * tick, arsenal::Lance);
  lanceFxClock -= dt;
  if (lanceFxClock <= 0) {
    world.beam(muzzle(), hit.point, module.spec().color, .65f, .075f);
    world.beam(muzzle(), hit.point, {1, .85f, .9f}, .17f, .075f);
    lanceFxClock = .04f;
  }
  shotFlash = .08f;
  recoil = .25f;
  lanceRemaining -= tick;
  if (lanceRemaining <= 0)
    cooldown = module.spec().interval / module.rateMultiplier;
}
