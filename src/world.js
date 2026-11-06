import * as THREE from 'three';
import { RULES, makeRandom } from './simulation.js';

export { THREE };
const rand = makeRandom(2901);
const palette = { teal: 0x8dffdc, red: 0xff425b, ground: 0x46545a };

function canvasTexture(size, paint) {
  const c = document.createElement('canvas'); c.width = c.height = size;
  paint(c.getContext('2d'), size);
  const t = new THREE.CanvasTexture(c); t.colorSpace = THREE.SRGBColorSpace; return t;
}
function terrainTexture() {
  const t = canvasTexture(512, (ctx, n) => {
    ctx.fillStyle = '#59636a'; ctx.fillRect(0, 0, n, n);
    for (let i = 0; i < 26000; i++) {
      const v = 52 + Math.floor(rand() * 64);
      ctx.fillStyle = `rgba(${v},${v + 6},${v + 9},${rand() * 0.5})`;
      ctx.fillRect(rand() * n, rand() * n, 1 + rand() * 3, 1 + rand() * 3);
    }
    for (let i = 0; i < 220; i++) {
      ctx.strokeStyle = '#353e4440'; ctx.beginPath(); const x = rand() * n, y = rand() * n;
      ctx.moveTo(x, y); ctx.lineTo(x + rand() * 15, y + rand() * 5); ctx.stroke();
    }
  });
  t.wrapS = t.wrapT = THREE.RepeatWrapping; t.repeat.set(55, 55); return t;
}
const box = (w, h, d) => new THREE.BoxGeometry(w, h, d);
function mesh(geometry, material, x = 0, y = 0, z = 0, parent) {
  const m = new THREE.Mesh(geometry, material); m.position.set(x, y, z);
  if (parent) parent.add(m); return m;
}
function ring(radius, color, opacity = 1, width = 0.06) {
  const m = mesh(new THREE.RingGeometry(radius - width, radius + width, 128), new THREE.MeshBasicMaterial({ color, transparent: true, opacity, side: THREE.DoubleSide, depthWrite: false }));
  m.rotation.x = -Math.PI / 2; return m;
}

export async function createWorld(canvas) {
  const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, powerPreference: 'high-performance' });
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 1.65));
  renderer.setSize(innerWidth, innerHeight);
  renderer.shadowMap.enabled = true; renderer.shadowMap.type = THREE.PCFSoftShadowMap;
  renderer.toneMapping = THREE.ACESFilmicToneMapping; renderer.toneMappingExposure = 1.12;
  const scene = new THREE.Scene(); scene.background = new THREE.Color(0x17272e);
  scene.fog = new THREE.FogExp2(0x23373c, 0.009);
  const camera = new THREE.PerspectiveCamera(65, innerWidth / innerHeight, 0.08, 1200);
  camera.position.set(60, 53, 70); camera.lookAt(0, 0, 0);
  scene.add(new THREE.HemisphereLight(0xc5edfa, 0x414055, 2.5));
  const sun = new THREE.DirectionalLight(0xffdec2, 3.2); sun.position.set(-35, 65, -50);
  sun.castShadow = true; sun.shadow.mapSize.set(2048, 2048);
  Object.assign(sun.shadow.camera, { left: -70, right: 70, top: 70, bottom: -70, near: 1, far: 200 });
  sun.shadow.bias = -0.0006; scene.add(sun);
  const loader = new THREE.TextureLoader();
  const [metal, beast] = await Promise.all([loader.loadAsync('/assets/turret-metal.png'), loader.loadAsync('/assets/alien-beast.png')]);
  metal.colorSpace = beast.colorSpace = THREE.SRGBColorSpace;
  metal.anisotropy = Math.min(8, renderer.capabilities.getMaxAnisotropy());
  const groundTex = terrainTexture();
  const groundMat = new THREE.MeshStandardMaterial({ map: groundTex, color: 0x78868a, roughness: 1, bumpMap: groundTex, bumpScale: 0.12 });
  const ground = mesh(new THREE.CylinderGeometry(RULES.arenaRadius, RULES.arenaRadius + 5, 5, 128), groundMat, 0, -2.5, 0, scene);
  ground.receiveShadow = true;

  // Distance silhouettes and rocks, all actual 3D geometry.
  const rockGeo = new THREE.IcosahedronGeometry(1, 0);
  const rockMat = new THREE.MeshStandardMaterial({ color: 0x3a4850, roughness: 1, flatShading: true });
  const rocks = new THREE.InstancedMesh(rockGeo, rockMat, 420);
  const dummy = new THREE.Object3D();
  for (let i = 0; i < 420; i++) {
    const a = rand() * Math.PI * 2, r = 63 + rand() * 98;
    const s = i < 85 ? 3 + rand() * 10 : 0.3 + rand() * 1.6;
    dummy.position.set(Math.sin(a) * r, s * 0.23 - 0.2, Math.cos(a) * r);
    dummy.scale.set(s * (1 + rand()), s * (0.4 + rand()), s);
    dummy.rotation.set(rand(), rand() * 6.28, rand()); dummy.updateMatrix(); rocks.setMatrixAt(i, dummy.matrix);
  }
  rocks.castShadow = true; rocks.receiveShadow = true; scene.add(rocks);
  const pebbles = new THREE.InstancedMesh(rockGeo, rockMat, 900);
  for (let i = 0; i < 900; i++) {
    const a = rand() * 6.28, r = 10 + rand() * 125, s = 0.05 + rand() * 0.22;
    dummy.position.set(Math.sin(a) * r, s * 0.2, Math.cos(a) * r); dummy.scale.set(s * 2, s, s);
    dummy.rotation.set(rand(), rand() * 6.28, rand()); dummy.updateMatrix(); pebbles.setMatrixAt(i, dummy.matrix);
  }
  scene.add(pebbles);
  const crystalMat = new THREE.MeshStandardMaterial({ color: 0x285d60, emissive: 0x51d4b7, emissiveIntensity: 0.55, roughness: 0.35, metalness: 0.4 });
  for (let i = 0; i < 42; i++) {
    const a = rand() * 6.28, r = 70 + rand() * 75;
    const crystal = mesh(new THREE.ConeGeometry(0.45 + rand() * 0.7, 2 + rand() * 4, 5), crystalMat, Math.sin(a) * r, 0.7, Math.cos(a) * r, scene);
    crystal.rotation.z = (rand() - 0.5) * 0.6;
  }

  const sky = mesh(new THREE.SphereGeometry(700, 32, 24), new THREE.ShaderMaterial({
    side: THREE.BackSide, depthWrite: false,
    uniforms: { top: { value: new THREE.Color('#07131d') }, bottom: { value: new THREE.Color('#4c686b') } },
    vertexShader: 'varying vec3 vPos; void main(){vPos=position; gl_Position=projectionMatrix*modelViewMatrix*vec4(position,1.);}',
    fragmentShader: 'uniform vec3 top; uniform vec3 bottom; varying vec3 vPos; void main(){float h=normalize(vPos).y; gl_FragColor=vec4(mix(bottom,top,smoothstep(-.1,.7,h)),1.);}'
  }), 0, 0, 0, scene); sky.renderOrder = -10;
  const starPositions = [];
  for (let i = 0; i < 1000; i++) {
    const a = rand() * 6.28, y = rand() * 0.9 + 0.1, r = Math.sqrt(1 - y * y) * 650;
    starPositions.push(Math.sin(a) * r, y * 650, Math.cos(a) * r);
  }
  const starsGeo = new THREE.BufferGeometry(); starsGeo.setAttribute('position', new THREE.Float32BufferAttribute(starPositions, 3));
  scene.add(new THREE.Points(starsGeo, new THREE.PointsMaterial({ color: 0xc3e4ed, size: 0.8, transparent: true, opacity: 0.6, fog: false, depthWrite: false })));
  const moon = mesh(new THREE.SphereGeometry(43, 40, 32), new THREE.MeshStandardMaterial({ color: 0x77999c, roughness: 1, fog: false }), -230, 150, -400, scene);
  const moonRing = mesh(new THREE.RingGeometry(55, 66, 100), new THREE.MeshBasicMaterial({ color: 0x9fbbb6, side: THREE.DoubleSide, transparent: true, opacity: 0.2, fog: false }), 0, 0, 0, moon);
  moonRing.rotation.x = 1.15; moonRing.rotation.y = -0.4;

  const steel = new THREE.MeshStandardMaterial({ color: 0xbac8ce, map: metal, roughness: 0.72, metalness: 0.18 });
  const dark = new THREE.MeshStandardMaterial({ color: 0x60717a, map: metal, roughness: 0.68, metalness: 0.22 });
  const glow = new THREE.MeshBasicMaterial({ color: palette.teal });
  const core = new THREE.Group(); scene.add(core);
  const base = mesh(new THREE.CylinderGeometry(8, 8.5, 4, 64), steel, 0, 2, 0, core); base.castShadow = true; base.receiveShadow = true;
  mesh(new THREE.CylinderGeometry(8.45, 8.45, 0.4, 64), dark, 0, 0.3, 0, core);
  const deckMat = new THREE.MeshStandardMaterial({ map: metal, color: 0x6b7d85, metalness: 0.65, roughness: 0.72 });
  const deck = mesh(new THREE.CylinderGeometry(8.15, 8.15, 0.18, 64), deckMat, 0, 3.96, 0, core); deck.receiveShadow = true;
  for (const y of [0.65, 3.65]) { const r = ring(8.05, palette.teal, 0.85, 0.075); r.position.y = y; core.add(r); }
  for (let i = 0; i < 16; i++) {
    const a = i / 16 * Math.PI * 2;
    const rib = mesh(box(0.35, 2.75, 0.45), dark, Math.sin(a) * 8.01, 2, Math.cos(a) * 8.01, core); rib.rotation.y = a;
    const light = mesh(box(0.12, 1.4, 0.1), glow, Math.sin(a) * 8.27, 2.1, Math.cos(a) * 8.27, core); light.rotation.y = a;
    const marker = mesh(box(0.2, 0.035, 0.65), glow, Math.sin(a) * 7.6, 4.07, Math.cos(a) * 7.6, core); marker.rotation.y = a;
  }
  const deckRing = ring(6.8, 0x9faf9f, 0.5, 0.025); deckRing.position.y = 4.065; core.add(deckRing);
  mesh(new THREE.CylinderGeometry(1.3, 1.6, 0.4, 32), dark, 0, 4.2, 0, core);
  const reactor = mesh(new THREE.CylinderGeometry(0.6, 0.6, 2.3, 12), new THREE.MeshStandardMaterial({ color: 0x6efbd3, emissive: 0x64ffd3, emissiveIntensity: 2, transparent: true, opacity: 0.8, metalness: 0.1, roughness: 0.3 }), 0, 5.5, 0, core);
  for (const y of [4.5, 6.5]) mesh(new THREE.CylinderGeometry(1, 1, 0.22, 16), dark, 0, y, 0, core);
  for (let i = 0; i < 4; i++) { const a = i * Math.PI / 2; mesh(box(0.18, 2.3, 0.18), dark, Math.sin(a) * 0.8, 5.5, Math.cos(a) * 0.8, core); }
  const coreLight = new THREE.PointLight(palette.teal, 35, 22, 2); coreLight.position.set(0, 7, 0); scene.add(coreLight);
  const radiusLine = ring(RULES.buildRadius, palette.teal, 0.25, 0.045); radiusLine.position.y = 0.06; scene.add(radiusLine);
  const grid = new THREE.PolarGridHelper(RULES.buildRadius, 16, 5, 128, 0x305f5c, 0x365c59); grid.position.y = 0.025;
  grid.material.transparent = true; grid.material.opacity = 0.28; scene.add(grid);
  const coreHalo = ring(11, palette.teal, 0.2, 0.025); coreHalo.position.y = 0.07; scene.add(coreHalo);

  const lightningTex = canvasTexture(128, ctx => {
    ctx.fillStyle = '#e6d982'; ctx.beginPath(); ctx.moveTo(77, 12); ctx.lineTo(35, 70); ctx.lineTo(61, 70); ctx.lineTo(49, 116); ctx.lineTo(96, 53); ctx.lineTo(69, 53); ctx.closePath(); ctx.fill();
  });
  const boltMat = new THREE.MeshBasicMaterial({ map: lightningTex, transparent: true, side: THREE.DoubleSide, depthWrite: false });
  const laserMat = new THREE.MeshBasicMaterial({ color: 0xffe6e8 });
  const laserGlowMat = new THREE.MeshBasicMaterial({ color: palette.red, transparent: true, opacity: 0.35, depthWrite: false, blending: THREE.AdditiveBlending });
  const lensMat = new THREE.MeshStandardMaterial({ color: 0xff3048, emissive: 0xff143b, emissiveIntensity: 2.5, roughness: 0.25 });
  const shadowTex = canvasTexture(64, (ctx, s) => { const g = ctx.createRadialGradient(s / 2, s / 2, 0, s / 2, s / 2, s / 2); g.addColorStop(0, '#000000a0'); g.addColorStop(1, '#00000000'); ctx.fillStyle = g; ctx.fillRect(0, 0, s, s); });
  const enemyMat = new THREE.SpriteMaterial({ map: beast, transparent: true, alphaTest: 0.08, depthWrite: false });
  const enemyViews = new Map(), turretViews = new Map(), effects = [];
  const enemyShadowMat = new THREE.MeshBasicMaterial({ map: shadowTex, transparent: true, depthWrite: false });
  const enemyShadowGeo = new THREE.PlaneGeometry(1, 1);
  const barBack = new THREE.SpriteMaterial({ color: 0x1c101b, depthTest: false });
  const barFront = new THREE.SpriteMaterial({ color: 0xff6d77, depthTest: false });

  function makeTurret(ghost = false) {
    const root = new THREE.Group(), head = new THREE.Group();
    const mat = ghost ? new THREE.MeshStandardMaterial({ color: palette.teal, transparent: true, opacity: 0.42, depthWrite: false }) : steel;
    const foot = mesh(new THREE.CylinderGeometry(1.05, 1.4, 0.32, 8), ghost ? mat : dark, 0, 0.16, 0, root);
    mesh(box(0.45, 3.25, 0.5), mat, 0, 1.85, 0, root);
    mesh(new THREE.CylinderGeometry(0.5, 0.5, 0.26, 16), ghost ? mat : dark, 0, 3.45, 0, root);
    head.position.y = 3.8; root.add(head);
    mesh(box(1.55, 0.95, 1.95), mat, 0, 0, 0, head);
    mesh(box(1.62, 0.14, 2.02), ghost ? mat : dark, 0, 0.52, 0, head);
    mesh(box(0.3, 0.3, 1.05), ghost ? mat : dark, 0, 0, -1.42, head);
    const muzzle = mesh(new THREE.SphereGeometry(0.25, 16, 12), ghost ? mat : lensMat, 0, 0, -1.98, head);
    if (!ghost) {
      for (const sign of [-1, 1]) { const decal = mesh(new THREE.PlaneGeometry(0.65, 0.65), boltMat, sign * 0.782, 0, 0, head); decal.rotation.y = sign * Math.PI / 2; }
      for (let i = 0; i < 4; i++) mesh(box(1.3, 0.06, 0.05), dark, 0, -0.18 + i * 0.13, 0.982, head);
      const power = ring(1.35, palette.teal, 0.6, 0.04); power.position.y = 0.05; root.add(power);
    }
    root.traverse(o => { if (o.isMesh) { o.castShadow = !ghost; o.receiveShadow = !ghost; } });
    scene.add(root); return { root, head, muzzle, mat, foot };
  }
  const ghost = makeTurret(true); ghost.root.visible = false;
  const range = ring(RULES.turretRange, palette.teal, 0.65, 0.07); range.position.y = 0.08; range.visible = false; scene.add(range);
  const rangeFill = mesh(new THREE.CircleGeometry(RULES.turretRange, 96), new THREE.MeshBasicMaterial({ color: palette.teal, transparent: true, opacity: 0.025, depthWrite: false, side: THREE.DoubleSide })); rangeFill.rotation.x = -Math.PI / 2; rangeFill.position.y = 0.04; rangeFill.visible = false; scene.add(rangeFill);

  function shot(event) {
    const view = turretViews.get(event.turretId); if (!view) return;
    view.head.rotation.y = Math.atan2(-(event.x - view.root.position.x), -(event.z - view.root.position.z));
    view.root.updateMatrixWorld(true);
    const from = new THREE.Vector3(); view.muzzle.getWorldPosition(from);
    const to = new THREE.Vector3(event.x, event.height, event.z), direction = to.clone().sub(from);
    const beam = new THREE.Group();
    for (const [radius, mat] of [[0.045, laserMat], [0.15, laserGlowMat]]) {
      const line = mesh(new THREE.CylinderGeometry(radius, radius, direction.length(), 6), mat, 0, 0, 0, beam);
      line.quaternion.setFromUnitVectors(new THREE.Vector3(0, 1, 0), direction.clone().normalize());
    }
    beam.position.copy(from).lerp(to, 0.5); scene.add(beam); effects.push({ object: beam, life: 0.13, maxLife: 0.13 });
    const spark = mesh(new THREE.SphereGeometry(0.3, 6, 4), laserGlowMat, event.x, event.height, event.z, scene);
    effects.push({ object: spark, life: 0.2, maxLife: 0.2, expand: true });
    const enemy = enemyViews.get(event.targetId); if (enemy) enemy.flash = 0.15;
  }
  function sync(game, dt, tactical) {
    grid.visible = radiusLine.visible = tactical;
    scene.fog.density = tactical ? 0.004 : 0.009;
    reactor.scale.x = reactor.scale.z = 1 + Math.sin(game.time * 3) * 0.045;
    coreLight.intensity = 30 + Math.sin(game.time * 2) * 5;
    for (const t of game.turrets) {
      let v = turretViews.get(t.id);
      if (!v) { v = makeTurret(); v.root.position.set(t.x, 0, t.z); turretViews.set(t.id, v); }
      const target = game.enemies.find(e => e.id === t.targetId);
      if (target) {
        const desired = Math.atan2(-(target.x - t.x), -(target.z - t.z));
        const delta = Math.atan2(Math.sin(desired - v.head.rotation.y), Math.cos(desired - v.head.rotation.y));
        v.head.rotation.y += delta * Math.min(dt * 12, 1);
      } else v.head.rotation.y += dt * 0.15;
    }
    const active = new Set();
    for (const e of game.enemies) {
      active.add(e.id); let v = enemyViews.get(e.id);
      if (!v) {
        const sprite = new THREE.Sprite(enemyMat.clone());
        sprite.center.set(0.5, 0.04); scene.add(sprite);
        const shadow = mesh(enemyShadowGeo, enemyShadowMat, e.x, 0.06, e.z, scene); shadow.rotation.x = -Math.PI / 2;
        const bg = new THREE.Sprite(barBack), hp = new THREE.Sprite(barFront); scene.add(bg, hp);
        v = { sprite, shadow, bg, hp, flash: 0 }; enemyViews.set(e.id, v);
      }
      const stride = Math.sin(game.time * e.speed * 2.5 + e.phase);
      v.sprite.position.set(e.x, 0.12 + Math.abs(stride) * 0.14, e.z);
      v.sprite.scale.set(e.size * (1 + stride * 0.025), e.size * (1 - stride * 0.025), 1);
      v.flash = Math.max(0, v.flash - dt);
      v.sprite.material.color.set(v.flash > 0 ? 0xffac9d : e.type === 'runner' ? 0xc3f1fc : e.type === 'brute' ? 0xe4b8ff : 0xffffff);
      v.shadow.position.set(e.x, 0.06, e.z); v.shadow.scale.set(e.size, e.size * 0.6, 1);
      v.bg.visible = v.hp.visible = e.hp < e.maxHp;
      v.bg.position.set(e.x, e.size + 0.6, e.z); v.hp.position.copy(v.bg.position); v.hp.position.y += 0.02;
      v.bg.scale.set(e.size * 0.6, 0.13, 1); v.hp.scale.set(e.size * 0.6 * Math.max(0, e.hp / e.maxHp), 0.07, 1);
    }
    for (const [id, v] of enemyViews) if (!active.has(id)) {
      scene.remove(v.sprite, v.shadow, v.bg, v.hp); v.sprite.material.dispose(); enemyViews.delete(id);
    }
    for (let i = effects.length - 1; i >= 0; i--) {
      const e = effects[i]; e.life -= dt;
      if (e.expand) e.object.scale.setScalar(1 + (1 - e.life / e.maxLife) * 4);
      if (e.life <= 0) { scene.remove(e.object); e.object.traverse(o => o.geometry?.dispose()); effects.splice(i, 1); }
    }
  }
  function preview(x, z, valid, visible) {
    ghost.root.visible = range.visible = rangeFill.visible = visible;
    if (!visible) return;
    ghost.root.position.set(x, 0, z); range.position.set(x, 0.08, z); rangeFill.position.set(x, 0.04, z);
    const color = valid ? palette.teal : palette.red;
    ghost.mat.color.set(color); range.material.color.set(color); rangeFill.material.color.set(color);
  }
  function reset() {
    // Turret geometry is unique per instance, materials are shared.
    for (const v of turretViews.values()) { scene.remove(v.root); v.root.traverse(o => o.geometry?.dispose()); }
    turretViews.clear();
    for (const v of enemyViews.values()) { scene.remove(v.sprite, v.shadow, v.bg, v.hp); v.sprite.material.dispose(); }
    enemyViews.clear();
    for (const e of effects) { scene.remove(e.object); e.object.traverse(o => o.geometry?.dispose()); } effects.length = 0;
  }
  return { scene, camera, renderer, sync, shot, preview, reset, turretViews, enemyViews };
}
