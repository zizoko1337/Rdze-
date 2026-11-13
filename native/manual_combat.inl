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
  int candidate = -1;
  uint32_t generation = 0;
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
      candidate = id;
      generation = world.foes[id].generation;
    }
  });
  if (candidate != smartCandidate || generation != smartCandidateGeneration) {
    smartCandidate = candidate;
    smartCandidateGeneration = generation;
    smartSince = world.time;
  }
  smartLock = candidate >= 0 ? std::clamp(world.time - smartSince, 0.f, .5f) : 0;
  bool locked = candidate >= 0 && (module.instantLock || smartLock >= .49999f);
  smartTarget = locked ? candidate : -1;
  smartGeneration = locked ? generation : 0;
}
void fireSupportArc() {
  Vec3 origin = eye(), direction = forward();
  float end = module.spec().range;
  if (direction.y < -.00001f)
    end = std::min(end, -origin.y / direction.y);
  int target = -1;
  for (int i = 0; i < int(world.towers.size()); ++i) {
    const auto& t = world.towers[i];
    Vec3 delta = Vec3{t.x, WEAPONS[t.type].height * .6f, t.z} - origin;
    float along = dot(delta, direction), side = dot(delta, delta) - along * along;
    if (along > 0 && along < end && side < 3.2f * 3.2f) {
      end = along;
      target = i;
    }
  }
  Vec3 from = muzzle();
  std::array<int, 10> visited{};
  int count = 0;
  if (target < 0) {
    from = origin + direction * end;
    world.beam(muzzle(), from, {.35f, .7f, 1}, .16f, .3f);
    if (from.y > .05f || direction.y >= 0)
      return;
    visited[count++] = -1;
  }
  while (count < 10) {
    if (target < 0) {
      float best = 56 * 56;
      for (int i = 0; i < int(world.towers.size()); ++i) {
        if (std::find(visited.begin(), visited.begin() + count, i) != visited.begin() + count)
          continue;
        const auto& t = world.towers[i];
        Vec3 p{t.x, WEAPONS[t.type].height * .6f, t.z};
        float d = dot(p - from, p - from);
        if (d < best) {
          best = d;
          target = i;
        }
      }
    }
    if (target < 0)
      break;
    auto& t = world.towers[target];
    Vec3 to{t.x, WEAPONS[t.type].height * .6f, t.z};
    world.lightning(from, to, count + int(t.id));
    t.amplifiedUntil = std::max(t.amplifiedUntil, world.time + TESLA_BUFF_DURATION);
    visited[count++] = target;
    from = to;
    target = -1;
  }
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
  world.blastTargets(point, radius, [&](int id, float) {
    // A ground blast reaches tall enemies but not a distant airborne centre.
    if (world.foes[id].type == Wyrm ||
        length(world.targetPoint(id) - point) <= radius + world.foes[id].size * .25f)
      manualHit(id, damage, source, false);
  });
  world.ring(point, WEAPONS[source].color, radius, .65f);
  world.sparks(point + Vec3{0, 1, 0}, WEAPONS[source].color, 28, 28);
}
void fireArc() {
  if (module.teslaAmplifier && teslaSupport) {
    fireSupportArc();
    return;
  }
  auto hit = trace(world, eye(), forward(), module.spec().range);
  std::array<int, 10> visited{};
  int count = 0, id = hit.id;
  Vec3 from = muzzle();
  auto nearest = [&](Vec3 origin) {
    int result = -1;
    float best = 56 * 56;
    world.grid.radius(origin.x, origin.z, 56, world.foes, [&](int candidate) {
      if (std::find(visited.begin(), visited.begin() + count, candidate) != visited.begin() + count)
        return;
      Vec3 d = world.targetPoint(candidate) - origin;
      float distance = dot(d, d);
      if (distance < best) {
        best = distance;
        result = candidate;
      }
    });
    return result;
  };
  bool firstTarget = true;
  if (id < 0) {
    world.beam(from, hit.point, module.spec().color, .18f, .22f);
    if (hit.point.y > .05f || forward().y >= 0)
      return;
    world.sparks(hit.point, module.spec().color, 10, 8);
    world.groundMarks.spot(hit.point, .65f, 3, .45f);
    visited[count++] = -1; // Ground consumes the first link of the ten-link discharge.
    from = hit.point;
    id = nearest(from);
  }
  while (id >= 0 && count < int(visited.size())) {
    Vec3 to = world.targetPoint(id);
    Vec3 mid = (from + to) * .5f;
    mid = mid + Vec3{(random.unit() - .5f) * 2, 1 + random.unit(), (random.unit() - .5f) * 2};
    world.beam(from, mid, module.spec().color, .18f, .22f);
    world.beam(mid, to, module.spec().color, .18f, .22f);
    world.beam(from, to, {1, 1, 1}, .045f, .12f);
    visited[count++] = id;
    manualHit(id, manualDamage(), Tesla, firstTarget);
    firstTarget = false;
    from = to;
    id = nearest(from);
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
  // Include the worm body once in the piercing list, sharing the head's ID.
  for (const auto& body : world.worms) {
    if (!world.valid(body.head, body.generation))
      continue;
    float nearest = range;
    for (int n = 0; n < 8; ++n) {
      float size = 11.f - n * .45f;
      auto p = body.segments[n];
      Vec3 delta = origin - Vec3{p.x, size * .45f, p.z};
      float r = size * .43f, b = dot(delta, direction);
      float d = b * b - dot(delta, delta) + r * r;
      if (d >= 0 && -b + std::sqrt(d) >= 0)
        nearest = std::min(nearest, std::max(0.f, -b - std::sqrt(d)));
    }
    if (nearest >= range)
      continue;
    bool already = false;
    for (auto& hit : hits)
      if (hit.id == body.head) {
        if (nearest < hit.distance)
          hit = {body.head, nearest, origin + direction * nearest};
        already = true;
        break;
      }
    if (!already && nearest < hits.back().distance)
      hits.back() = {body.head, nearest, origin + direction * nearest};
    std::sort(hits.begin(), hits.end(),
              [](const RayHit& a, const RayHit& b) { return a.distance < b.distance; });
  }
  bool primary = true;
  for (const auto& hit : hits)
    if (hit.id >= 0) {
      float distance = length(world.targetPoint(hit.id) - origin);
      manualHit(hit.id, sniperDamageAt(distance), Sniper, primary);
      primary = false;
      world.sparks(hit.point, module.spec().color, 8, 8);
    }
  world.beam(muzzle(), origin + direction * range, module.spec().color, .16f, .25f);
  world.beam(muzzle(), origin + direction * range, {1, 1, 1}, .05f, .15f);
  Vec3 end = origin + direction * range;
  if (direction.y < 0 && std::abs(end.y) < .01f)
    world.groundMarks.spot(end, .5f, 4, .2f);
}
void fireFlameCone(bool focused) {
  Vec3 f = forward();
  float range = focused ? 94.5f : 63.f, halfAngle = focused ? std::atan(2.5f / 94.5f) : .5f;
  Vec3 emitter = module.dualFlame && focused ? gunPoint({-.9f, -.15f, 4.3f}) : muzzle();
  auto hit = trace(world, eye(), f, range + 10);
  if (hit.id < 0 && f.y < 0 && std::abs(hit.point.y) < .01f && hit.distance <= range)
    world.groundMarks.spot(hit.point, focused ? .65f : 1.2f, 4.5f, .7f);
  if (hit.id >= 0 &&
      std::hypot(world.foes[hit.id].x - eye().x, world.foes[hit.id].z - eye().z) <= range)
    markTarget(hit.id);
  world.grid.radius(eye().x, eye().z, range, world.foes, [&](int id) {
    Vec3 d = world.targetPoint(id) - eye();
    if (dot(normalize(d), f) >= std::cos(halfAngle)) {
      manualHit(id, manualDamage() * module.spec().interval, arsenal::Flame, false);
      auto& foe = world.foes[id];
      foe.burn = std::max(foe.burn, 3.f);
      foe.burnDps = std::max(foe.burnDps, 30 * module.damageMultiplier);
    }
  });
  Vec3 right = normalize(cross({0, 1, 0}, f)), up = cross(f, right);
  for (int i = 0; i < 14; ++i) {
    float a = random.unit() * 2 * PI, spread = std::sqrt(random.unit()) * std::tan(halfAngle);
    Vec3 d = normalize(f + right * (std::cos(a) * spread) + up * (std::sin(a) * spread));
    world.particle(emitter + d * (random.unit() * 8), d * (range / .6f),
                   i % 3 ? module.spec().color : Color{1, .8f, .2f},
                   focused ? .3f + random.unit() * .45f : 1 + random.unit() * 3, .65f);
  }
}
void fireFlame() {
  fireFlameCone(flameFocused);
  if (module.dualFlame)
    fireFlameCone(true);
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
          if (patch.glue)
            world.foes[id].glued = std::max(world.foes[id].glued, .16f);
          else
            world.foes[id].toxicSlow = .16f;
          world.foes[id].poison = 6;
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
void stepMinigunCooling(float dt, bool firing) {
  if (module.kind != Gun::Crank) {
    barrelHeat = 0;
    coolantActive = false;
    return;
  }
  if (module.extremeCooling && barrelHeat >= .42f)
    coolantActive = true;
  if (!module.extremeCooling || barrelHeat <= .23f)
    coolantActive = false;
  barrelHeat = std::max(0.f, barrelHeat - dt * (coolantActive ? .95f : firing ? .035f : .14f));
  coolantClock -= dt;
  if (coolantActive && coolantClock <= 0) {
    coolantClock = .05f;
    for (float side : {-1.f, 1.f}) {
      Vec3 nozzle = gunPoint({side * .62f, .18f, 3.03f});
      Vec3 tip = gunPoint({side * .08f, 0, 3.18f});
      for (int i = 0; i < 2; ++i)
        world.particle(nozzle, normalize(tip - nozzle) * (2.5f + i) + Vec3{0, .5f, 0},
                       {.65f, .88f, 1}, .14f + i * .05f, .3f, 1);
    }
  }
}
void firePlasmaMortar() {
  Vec3 center = groundAim();
  float rotation = random.unit() * 2 * PI;
  for (int i = 0; i < 20; ++i) {
    float angle = i * 2.399963f + rotation;
    float radius = i == 0 ? 0 : PlasmaSpec::spreadRadius * std::sqrt(float(i) / 19);
    Vec3 end = center + Vec3{std::sin(angle) * radius, 0, std::cos(angle) * radius};
    Vec3 start = plasmaMuzzle(i);
    ManualProjectile p{Gun::Pulse, start, start, start, end, {}};
    p.plasma = true;
    p.emitter = i;
    p.damage = manualDamage();
    float speed =
        PlasmaSpec::minSpeed + random.unit() * (PlasmaSpec::maxSpeed - PlasmaSpec::minSpeed);
    p.duration = std::max(60.f, length(end - start)) / speed;
    manualProjectiles.push_back(p);
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
      p.pos.y += std::sin(t * PI) * p.arcHeight;
      impact = t >= 1;
    }
    if (impact) {
      if (p.plasma) {
        // One vertical energy strike per endpoint. No area damage and no per-frame target search.
        auto hit = trace(world, p.endpoint + Vec3{0, 60, 0}, {0, -1, 0}, 60, PlasmaSpec::hitRadius);
        if (hit.id >= 0) {
          manualHit(hit.id, p.damage, Laser, p.emitter == 0);
          world.beam(p.pos + Vec3{0, 4, 0}, hit.point, {.2f, .95f, 1}, .13f, .12f);
        }
        world.sparks(hit.point, {.2f, .95f, 1}, 4, 5);
        if (hit.id < 0)
          world.groundMarks.spot(p.endpoint, .4f, 3, .6f);
        world.ring(p.endpoint, {.2f, .9f, 1}, 1.3f, .25f);
      } else if (p.kind == Gun::Toxin) {
        if (toxicPatches.size() < 32)
          toxicPatches.push_back({p.endpoint, 8, 0, p.damage, p.glue});
        markTarget(world.grid.nearest(p.endpoint.x, p.endpoint.z, 14, world.foes));
        world.ring(p.endpoint, GUNS[int(Gun::Toxin)].color, 14, .5f);
        world.sparks(p.endpoint, GUNS[int(Gun::Toxin)].color, 16, 12);
      } else {
        manualBlast(p.pos, p.kind == Gun::Smart ? 9 : 15, p.damage,
                    p.kind == Gun::Smart ? Rocket : Mortar,
                    world.valid(p.target, p.generation) ? p.target : -1);
        if (p.pos.y <= 2.5f)
          world.groundMarks.spot({p.pos.x, 0, p.pos.z}, p.kind == Gun::Smart ? 2.f : 3.5f, 6, .8f);
        if (p.kind == Gun::Howitzer && p.bounces > 0) {
          p.start = p.pos = p.previous = p.endpoint;
          p.endpoint = p.start + p.bounceDirection * (p.bounces == 2 ? 12.f : 8.f);
          p.endpoint.y = .2f;
          p.arcHeight = p.bounces == 2 ? 7 : 4;
          p.duration = p.bounces == 2 ? .65f : .5f;
          p.age = 0;
          --p.bounces;
          ++i;
          continue;
        }
      }
      manualProjectiles[i] = manualProjectiles.back();
      manualProjectiles.pop_back();
    } else
      ++i;
  }
}
void stepLance(float dt, bool held) {
  if (module.kind != Gun::Lance) {
    lanceGroundValid = false;
    return;
  }
  if (!held || mode != Mode::Mounted || tactical) {
    lanceGroundValid = false;
    lanceGroundClock = 0;
    lanceCharge = 0;
    if (lanceActive)
      cooldown = module.spec().interval / module.rateMultiplier;
    lanceRemaining = 0;
    lanceActive = lanceReserveActive = false;
    return;
  }
  if (cooldown > 0) {
    lanceGroundValid = false;
    return;
  }
  if (!lanceActive) {
    lanceCharge = std::min(1.8f, lanceCharge + dt * module.rateMultiplier);
    if (lanceCharge < 1.8f)
      return;
    lanceRemaining = 3;
    lanceActive = true;
    lanceReserveActive = false;
    std::fill(lanceTouched.begin(), lanceTouched.end(), 0);
    lanceCharge = 0;
    ++shots;
    feedback.shot(Gun::Lance);
    ++world.fired[arsenal::Lance];
  }
  float normalTick = std::min(dt, lanceRemaining);
  lanceRemaining = std::max(0.f, lanceRemaining - normalTick);
  float tick = normalTick;
  if (module.uraniumReserve && lanceRemaining <= .00001f && dt > normalTick) {
    float reserveTick = std::min(dt - normalTick, expedition.uranium / 5.f);
    expedition.uranium = std::max(0.f, expedition.uranium - reserveTick * 5);
    tick += reserveTick;
    lanceReserveActive = reserveTick > 0;
  }
  auto hit = trace(world, eye(), forward(), module.spec().range);
  if (tick > 0 && hit.id < 0 && forward().y < 0 && std::abs(hit.point.y) < .01f) {
    lanceGroundClock -= tick;
    if (!lanceGroundValid || lanceGroundClock <= 0) {
      Vec3 from = lanceGroundValid && length(hit.point-lanceGroundPrevious) <= 20
                      ? lanceGroundPrevious : hit.point;
      world.groundMarks.stroke(from, hit.point, .34f, 4.5f, 1.6f);
      lanceGroundPrevious = hit.point;
      lanceGroundValid = true;
      lanceGroundClock = 1.f/30;
    }
  } else {
    lanceGroundValid = false;
    lanceGroundClock = 0;
  }
  if (hit.id >= 0 && tick > 0) {
    if (lanceTouched[hit.id] != world.foes[hit.id].generation) {
      lanceTouched[hit.id] = world.foes[hit.id].generation;
      manualHit(hit.id, 200 * module.damageMultiplier + flatDamage, arsenal::Lance);
    }
    manualHit(hit.id, manualDamage() * tick, arsenal::Lance);
  }
  shotFlash = .08f;
  recoil = .25f;
  if (lanceRemaining <= .00001f && (!module.uraniumReserve || expedition.uranium <= .00001f)) {
    if (module.uraniumReserve && expedition.uranium <= .00001f)
      expedition.uranium = 0;
    lanceRemaining = 0;
    lanceActive = lanceReserveActive = false;
    cooldown = module.spec().interval / module.rateMultiplier;
  }
}
