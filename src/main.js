import './style.css';
import { createWorld, THREE } from './world.js';
import { RULES, createGame, buildTurret, canBuild, stepGame, startWave, spawnEnemy } from './simulation.js';
import { GameAudio } from './audio.js';

const hexIcon = '<svg viewBox="0 0 40 44" fill="none"><path d="M20 2 37 12v20L20 42 3 32V12Z" stroke="currentColor" stroke-width="1.5"/><path d="m23 10-12 15h9l-3 10 13-17h-9z" fill="currentColor"/></svg>';
const turretIcon = '<svg viewBox="0 0 100 80" fill="none"><path d="M39 40h11v30H39zM29 70h31v5H29z" fill="currentColor" opacity=".45"/><path d="M20 16h49v26H20z" stroke="currentColor" stroke-width="2"/><path d="M69 24h17v10H69z" fill="currentColor" opacity=".65"/><circle cx="88" cy="29" r="6" fill="#ff6072"/><path d="m47 20-9 12h7l-3 7 12-13h-8z" fill="currentColor"/></svg>';
document.querySelector('#app').innerHTML = `
  <canvas id="world" aria-label="Trójwymiarowa arena gry Rdzeń"></canvas>
  <div class="vignette"></div><div id="damage-flash"></div>
  <header class="topbar">
    <div class="brand">${hexIcon}<div><strong>RDZEŃ<span> / </span></strong><small>OSTATNI PRZYCZÓŁEK</small></div><span class="prototype">PROTOTYP 01</span></div>
    <div class="location"><span class="live-dot"></span> KEPLER–186F <span class="separator">/</span> SEKTOR 07</div>
    <div class="header-actions"><button id="sound" title="Dźwięk [M]" aria-label="Wyłącz dźwięk">DŹWIĘK <span>WŁ.</span></button><button id="pause" title="Pauza [P]" aria-label="Wstrzymaj grę">Ⅱ</button></div>
  </header>
  <div id="hud" class="hidden">
    <aside class="status-panel panel">
      <div class="eyebrow"><span class="live-dot"></span> PLATFORMA WYDOBYWCZA <span class="muted">01</span></div>
      <div class="core-title"><h2>Integralność rdzenia</h2><span id="health-percent">100<span>%</span></span></div>
      <div class="health-track"><div id="health-fill"></div></div>
      <div class="health-caption"><span id="health-value">1200 / 1200</span><span id="core-status">SYSTEM STABILNY</span></div>
      <div class="resource-row"><div class="resource-icon">◇</div><div><span class="eyebrow">SUROWCE</span><div class="resource-value" id="credits">320</div></div><span class="income">+3<span> / s</span></span></div>
      <div class="mini-stats"><div><span>WYELIMINOWANO</span><strong id="kills">000</strong></div><div><span>AKTYWNE DZIAŁA</span><strong id="turrets">03</strong></div></div>
    </aside>
    <div class="wave-panel"><span class="eyebrow">REAKCJA PLANETY</span><div><span class="wave-mark">≋</span><strong id="wave">FALA 01</strong><span id="enemy-count">0 SYGNAŁÓW</span></div><div class="wave-time"><span id="wave-label">Pierwszy kontakt za</span><span id="wave-time">00:12</span></div></div>
    <div class="view-label"><span id="view-dot"></span><span id="view-name">WIDOK OPERATORA</span><span id="view-coords">POKŁAD RDZENIA · +5,7 M</span></div>
    <aside class="radar-panel panel"><div class="eyebrow">SKAN POWIERZCHNI <span class="muted">R 160 M</span></div><canvas id="radar" width="360" height="360" aria-label="Radar wrogów i dział"></canvas><div class="radar-legend"><span><i class="teal"></i>BAZA</span><span><i class="red"></i>WROGOWIE</span><span id="elapsed">00:00</span></div></aside>
    <section id="build-panel" class="build-panel panel hidden"><div class="build-heading"><span class="eyebrow">SIEĆ OBRONNA</span><kbd>B</kbd></div><button id="build-laser" class="weapon selected">${turretIcon}<div><span class="eyebrow">L–01 / AUTOMATYCZNE</span><strong>Działo laserowe</strong><span class="weapon-cost">◇ 80 <span>SUROWCÓW</span></span></div><span class="weapon-key">1</span></button><div class="weapon-specs"><span>ZASIĘG<strong>35 m</strong></span><span>OBRAŻENIA<strong>24 / strzał</strong></span><span>TRAFIENIE<strong>100%</strong></span></div><p>Utrzymuje cel do eliminacji lub utraty zasięgu.</p><div id="build-hint">Wskaż miejsce w sieci zasilania</div></section>
    <div id="crosshair"><span></span><span></span></div>
    <div id="pointer-hint" class="hidden"><button id="capture-mouse">Kliknij, aby rozejrzeć się <span>ESC uwalnia kursor</span></button></div>
    <footer class="bottom-bar"><div class="controls" id="controls"></div><button id="switch-view"><kbd>TAB</kbd><span id="switch-label">Widok taktyczny</span><span class="switch-arrow">↗</span></button></footer>
    <div id="toast" role="status"></div>
  </div>
  <section id="menu" class="menu-overlay"><div class="menu-card"><div class="eyebrow"><span class="live-dot"></span> EKSPEDYCJA 186 / TRANSMISJA AKTYWNA</div><h1>RDZEŃ<span>.</span></h1><div class="menu-subtitle">OSTATNI PRZYCZÓŁEK</div><p class="intro">Przybyliśmy po surowce.<br>Planeta odpowiedziała.</p><p class="description">Utrzymaj wydobycie na obcej planecie. Rozstaw działa, odeprzyj napór bestii i obserwuj walkę z pokładu rdzenia.</p><button id="start" class="primary" disabled>INICJALIZACJA SYSTEMÓW… <span>→</span></button><div class="menu-controls"><span><kbd>W A S D</kbd> Ruch po pokładzie</span><span><kbd>TAB</kbd> Budowanie z góry</span></div><div class="menu-note">ŚWIAT 3D <i></i> BESTIE 2D <i></i> JEDEN RDZEŃ</div></div><div class="menu-coordinates">KPLR–186F<br><span>37° 04′ N / 122° 08′ E</span></div></section>
  <section id="pause-overlay" class="modal hidden"><div class="modal-card"><div class="eyebrow">SYSTEM W GOTOWOŚCI</div><h2>Chwila ciszy.</h2><p>Wydobycie i walka zostały wstrzymane.</p><button id="resume" class="primary">WRÓĆ DO GRY <span>→</span></button><p class="modal-help">Tab — zmień widok · P — pauza · M — dźwięk<br>WASD — ruch · Shift — bieg · Kółko — zoom z góry<br>LPM — postaw działo · PPM / B — anuluj budowanie</p></div></section>
  <section id="gameover" class="modal hidden"><div class="modal-card"><div class="eyebrow danger">POŁĄCZENIE UTRACONE</div><h2>Planeta wygrała.</h2><p>Rdzeń został zniszczony. Spróbuj innego rozmieszczenia obrony.</p><div id="final-stats"></div><button id="restart" class="primary">NOWA EKSPEDYCJA <span>↻</span></button></div></section>
  <div id="loading-error" class="modal hidden"><div class="modal-card"><h2>Nie udało się uruchomić gry</h2><p id="error-message"></p><button onclick="location.reload()" class="primary">SPRÓBUJ PONOWNIE</button></div></div>
`;

const $ = id => document.getElementById(id);
let world;
try { world = await createWorld($('world')); }
catch (e) { $('loading-error').classList.remove('hidden'); $('error-message').textContent = `Sprawdź, czy przeglądarka obsługuje WebGL 2 i ma włączoną akcelerację sprzętową. ${e.message}`; throw e; }
const audio = new GameAudio();
let game, started = false, mode = 'fps', building = true, yaw = 0, pitch = -0.08;
let tacticalZoom = 76, tacticalPan = new THREE.Vector3(), player = new THREE.Vector3(0, 5.72, -5.3);
let lastTime = 0, accumulator = 0, hudTimer = 0, toastTimer = 0, previousHp = RULES.coreMaxHp;
const keys = new Set(), pointer = new THREE.Vector2(10, 10), raycaster = new THREE.Raycaster();
const groundPlane = new THREE.Plane(new THREE.Vector3(0, 1, 0), 0), hit = new THREE.Vector3();
let placement = null;

function newGame() {
  world.reset(); game = createGame();
  // Three free starter turrets leave the player free to explore the deck first.
  for (const a of [0, 2.0944, 4.1888]) buildTurret(game, Math.sin(a) * 17, Math.cos(a) * 17);
  game.credits = RULES.startingCredits; game.events.length = 0;
  player.set(0, 5.72, -5.3); tacticalPan.set(0, 0, 0); yaw = 0; pitch = -0.08; previousHp = game.coreHp;
  world.sync(game, 0, true);
}
newGame();
$('start').disabled = false; $('start').innerHTML = 'ROZPOCZNIJ WYDOBYCIE <span>→</span>';

function toast(message) { $('toast').textContent = message; $('toast').classList.add('visible'); toastTimer = 3.5; }
function capture() {
  if (mode !== 'fps' || !started || game.paused || game.over) return;
  try { const pending = $('world').requestPointerLock(); pending?.catch(() => toast('Kliknij ponownie w ekran, aby przejąć mysz.')); }
  catch { toast('Przytrzymaj lewy przycisk myszy, aby się rozejrzeć.'); }
}
function setMode(next, lock = true) {
  mode = next; placement = null; pointer.set(10, 10);
  if (mode === 'tactical') { document.exitPointerLock?.(); }
  else if (lock) capture();
  $('build-panel').classList.toggle('hidden', mode !== 'tactical');
  $('crosshair').classList.toggle('hidden', mode !== 'fps');
  $('view-name').textContent = mode === 'fps' ? 'WIDOK OPERATORA' : 'WIDOK TAKTYCZNY';
  $('view-coords').textContent = mode === 'fps' ? 'POKŁAD RDZENIA · +5,7 M' : 'SIEĆ ZASILANIA · R 58 M';
  $('switch-label').textContent = mode === 'fps' ? 'Widok taktyczny' : 'Powrót na pokład';
  $('controls').innerHTML = mode === 'fps'
    ? '<span><kbd>W A S D</kbd> Ruch</span><span><kbd>MYSZ</kbd> Rozejrzyj się</span><span><kbd>SHIFT</kbd> Bieg</span><span><kbd>P</kbd> Pauza</span>'
    : '<span><kbd>LPM</kbd> Buduj</span><span><kbd>PPM</kbd> Anuluj</span><span><kbd>W A S D</kbd> Kamera</span><span><kbd>SCROLL</kbd> Zoom</span>';
  updatePointerHint();
}
function updatePointerHint() { $('pointer-hint').classList.toggle('hidden', mode !== 'fps' || document.pointerLockElement === $('world') || !started || game.paused || game.over); }
function start() {
  started = true; audio.resume(); $('menu').classList.add('hidden'); $('hud').classList.remove('hidden');
  setMode('fps'); toast('Tab: rozstaw obronę. Trzy działa już chronią rdzeń.');
}
function togglePause(force) {
  if (!started || game.over) return;
  game.paused = force ?? !game.paused; keys.clear();
  $('pause-overlay').classList.toggle('hidden', !game.paused);
  if (game.paused) document.exitPointerLock?.(); else if (mode === 'fps') capture();
  updatePointerHint();
}
function toggleSound() { audio.enabled = !audio.enabled; if (audio.enabled) audio.resume(); $('sound').innerHTML = `DŹWIĘK <span>${audio.enabled ? 'WŁ.' : 'WYŁ.'}</span>`; $('sound').setAttribute('aria-label', audio.enabled ? 'Wyłącz dźwięk' : 'Włącz dźwięk'); }
function toggleBuild(force) { building = force ?? !building; $('build-laser').classList.toggle('selected', building); }

$('start').onclick = start;
$('switch-view').onclick = () => setMode(mode === 'fps' ? 'tactical' : 'fps');
$('pause').onclick = () => togglePause(); $('resume').onclick = () => togglePause(false);
$('sound').onclick = toggleSound;
$('capture-mouse').onclick = capture;
$('build-laser').onclick = () => toggleBuild();
$('restart').onclick = () => { newGame(); accumulator = 0; $('gameover').classList.add('hidden'); setMode('fps'); audio.resume(); toast('Nowa ekspedycja. Rdzeń gotowy do wydobycia.'); };
document.addEventListener('pointerlockchange', updatePointerHint);
document.addEventListener('pointerlockerror', () => toast('Kliknij w ekran gry, aby przejąć mysz.'));
document.addEventListener('keydown', e => {
  if (['Tab', 'Space', 'KeyW', 'KeyA', 'KeyS', 'KeyD', 'ArrowUp', 'ArrowDown'].includes(e.code)) e.preventDefault();
  if (e.repeat) return;
  keys.add(e.code);
  if (e.code === 'KeyM') toggleSound();
  if (!started || game.over) return;
  if (e.code === 'KeyP' || (e.code === 'Escape' && mode === 'tactical')) togglePause();
  if (game.paused) return;
  if (e.code === 'Tab') setMode(mode === 'fps' ? 'tactical' : 'fps');
  if ((e.code === 'KeyB' || e.code === 'Digit1') && mode === 'tactical') toggleBuild(e.code === 'Digit1' ? true : undefined);
});
document.addEventListener('keyup', e => keys.delete(e.code));
window.addEventListener('blur', () => { keys.clear(); if (started && !game.paused && !game.over) togglePause(true); });
document.addEventListener('visibilitychange', () => { if (document.hidden && started && !game.paused && !game.over) togglePause(true); });
let dragging = false;
$('world').addEventListener('mousedown', e => { if (e.button === 0) dragging = true; });
document.addEventListener('mouseup', () => dragging = false);
document.addEventListener('mousemove', e => {
  pointer.set(e.clientX / innerWidth * 2 - 1, -(e.clientY / innerHeight) * 2 + 1);
  if (!started || game.paused || game.over) return;
  if (mode === 'fps' && (document.pointerLockElement === $('world') || dragging)) {
    yaw -= e.movementX * 0.0022; pitch = THREE.MathUtils.clamp(pitch - e.movementY * 0.0022, -1.35, 1.35);
  }
});
$('world').addEventListener('mouseleave', () => { placement = null; pointer.set(10, 10); dragging = false; });
$('world').addEventListener('click', () => {
  if (!started || game.paused || game.over) return;
  if (mode === 'fps') { capture(); return; }
  if (!building || !placement) return;
  const result = buildTurret(game, placement.x, placement.z);
  if (result.error) toast(result.error); else { audio.build(); toast('L–01 online. Automatyczne namierzanie aktywne.'); }
});
$('world').addEventListener('contextmenu', e => { e.preventDefault(); if (mode === 'tactical') toggleBuild(false); });
$('world').addEventListener('wheel', e => { if (mode === 'tactical') { e.preventDefault(); tacticalZoom = THREE.MathUtils.clamp(tacticalZoom + e.deltaY * 0.045, 28, 155); } }, { passive: false });
window.addEventListener('resize', () => { world.camera.aspect = innerWidth / innerHeight; world.camera.updateProjectionMatrix(); world.renderer.setSize(innerWidth, innerHeight); });

function updateCamera(dt) {
  const { camera } = world;
  if (!started) {
    const t = performance.now() * 0.000018;
    camera.position.set(55 * Math.cos(t), 39, 55 * Math.sin(t)); camera.lookAt(0, 1, 0); return;
  }
  const active = !game.paused && !game.over;
  const horizontal = active ? Number(keys.has('KeyD')) - Number(keys.has('KeyA')) : 0;
  const vertical = active ? Number(keys.has('KeyW')) - Number(keys.has('KeyS')) : 0;
  const norm = Math.hypot(horizontal, vertical) || 1;
  if (mode === 'fps') {
    const speed = keys.has('ShiftLeft') || keys.has('ShiftRight') ? 6 : 3.5;
    player.x += (Math.cos(yaw) * horizontal - Math.sin(yaw) * vertical) / norm * speed * dt;
    player.z += (-Math.sin(yaw) * horizontal - Math.cos(yaw) * vertical) / norm * speed * dt;
    const r = Math.hypot(player.x, player.z);
    if (r > 7.35) { player.x *= 7.35 / r; player.z *= 7.35 / r; }
    if (r < 1.65) { if (r > 0.001) { player.x *= 1.65 / r; player.z *= 1.65 / r; } else player.z = 1.65; }
    camera.position.copy(player); camera.position.y += active && (horizontal || vertical) ? Math.sin(game.time * speed * 3) * 0.035 : 0;
    camera.rotation.order = 'YXZ'; camera.rotation.set(pitch, yaw, 0);
  } else {
    tacticalPan.x = THREE.MathUtils.clamp(tacticalPan.x + horizontal / norm * dt * tacticalZoom * 0.42, -90, 90);
    tacticalPan.z = THREE.MathUtils.clamp(tacticalPan.z - vertical / norm * dt * tacticalZoom * 0.42, -90, 90);
    camera.position.set(tacticalPan.x, tacticalZoom, tacticalPan.z + tacticalZoom * 0.63);
    camera.lookAt(tacticalPan.x, 0, tacticalPan.z);
  }
  camera.updateMatrixWorld();
}
function updatePlacement() {
  const visible = started && mode === 'tactical' && building && !game.paused && !game.over && Math.abs(pointer.x) < 1 && Math.abs(pointer.y) < 1;
  if (!visible) { placement = null; world.preview(0, 0, false, false); return; }
  raycaster.setFromCamera(pointer, world.camera);
  if (!raycaster.ray.intersectPlane(groundPlane, hit)) { placement = null; world.preview(0, 0, false, false); return; }
  const x = Math.round(hit.x / 2) * 2, z = Math.round(hit.z / 2) * 2;
  const error = canBuild(game, x, z); placement = { x, z };
  world.preview(x, z, !error, true);
  $('build-hint').textContent = error ?? 'LPM — postaw działo za 80 surowców';
  $('build-hint').classList.toggle('invalid', !!error);
}
const formatTime = s => Number.isFinite(s) ? `${String(Math.floor(Math.max(0, s) / 60)).padStart(2, '0')}:${String(Math.floor(Math.max(0, s) % 60)).padStart(2, '0')}` : '—';
function updateHud() {
  const hp = Math.max(0, game.coreHp / RULES.coreMaxHp * 100);
  $('health-percent').innerHTML = `${Math.ceil(hp)}<span>%</span>`;
  $('health-fill').style.width = `${hp}%`; $('health-fill').classList.toggle('critical', hp < 30);
  $('health-value').textContent = `${Math.ceil(game.coreHp)} / ${RULES.coreMaxHp}`;
  $('core-status').textContent = hp < 30 ? 'STAN KRYTYCZNY' : hp < 75 ? 'USZKODZENIA POSZYCIA' : 'SYSTEM STABILNY';
  $('credits').textContent = Math.floor(game.credits).toLocaleString('pl-PL');
  $('kills').textContent = String(game.kills).padStart(3, '0'); $('turrets').textContent = String(game.turrets.length).padStart(2, '0');
  $('wave').textContent = `FALA ${String(Math.max(1, game.wave)).padStart(2, '0')}`;
  $('enemy-count').textContent = `${game.enemies.length} SYGNAŁÓW`;
  $('wave-label').textContent = game.pendingSpawns > 0 ? 'Nadciągają posiłki' : game.wave === 0 ? 'Pierwszy kontakt za' : 'Następna fala za';
  $('wave-time').textContent = game.pendingSpawns > 0 ? `+${game.pendingSpawns}` : formatTime(Math.ceil(game.nextWave - game.time));
  $('elapsed').textContent = formatTime(game.time);
  $('build-laser').classList.toggle('unaffordable', game.credits < RULES.turretCost);
  if (!building) $('build-hint').textContent = 'Wybierz działo lub naciśnij 1';
  drawRadar();
}
function drawRadar() {
  const c = $('radar').getContext('2d'), n = 360, center = n / 2, scale = 1.0;
  c.clearRect(0, 0, n, n); c.save(); c.translate(center, center);
  c.strokeStyle = '#84d5bc22'; c.lineWidth = 1;
  for (const r of [48, 98, 154]) { c.beginPath(); c.arc(0, 0, r, 0, Math.PI * 2); c.stroke(); }
  c.beginPath(); c.moveTo(-165, 0); c.lineTo(165, 0); c.moveTo(0, -165); c.lineTo(0, 165); c.stroke();
  const a = game.time * 0.4; c.strokeStyle = '#8dffd844'; c.beginPath(); c.moveTo(0, 0); c.lineTo(Math.cos(a) * 154, Math.sin(a) * 154); c.stroke();
  c.fillStyle = '#88f2cd'; c.beginPath(); c.arc(0, 0, 7, 0, Math.PI * 2); c.fill();
  c.strokeStyle = '#88f2cd55'; c.beginPath(); c.arc(0, 0, 58, 0, Math.PI * 2); c.stroke();
  for (const t of game.turrets) { c.fillStyle = '#b5f9e0'; c.fillRect(t.x * scale - 3, t.z * scale - 3, 6, 6); }
  for (const e of game.enemies) { c.fillStyle = e.type === 'brute' ? '#ffbb7c' : '#ff6275'; c.beginPath(); c.arc(e.x * scale, e.z * scale, e.type === 'brute' ? 3.5 : 2.5, 0, 6.28); c.fill(); }
  if (mode === 'fps') { c.strokeStyle = '#ffffff88'; c.beginPath(); c.moveTo(0, 0); c.arc(0, 0, 34, -yaw - Math.PI / 2 - 0.5, -yaw - Math.PI / 2 + 0.5); c.closePath(); c.stroke(); }
  else { c.strokeStyle = '#ffffff88'; c.strokeRect(tacticalPan.x - tacticalZoom * 0.7, tacticalPan.z - tacticalZoom * 0.5, tacticalZoom * 1.4, tacticalZoom); }
  c.fillStyle = '#99b0b3'; c.font = '16px monospace'; c.textAlign = 'center'; c.fillText('N', 0, -163); c.restore();
}
function processEvents() {
  for (const event of game.events) {
    if (event.type === 'shot') { world.shot(event); audio.shot(); }
    if (event.type === 'wave') { toast(`Fala ${event.wave} — planeta wysyła kolejne bestie.`); audio.wave(); }
    if (event.type === 'gameover') {
      document.exitPointerLock?.(); keys.clear();
      $('gameover').classList.remove('hidden');
      $('final-stats').innerHTML = `<div><strong>${formatTime(game.time)}</strong><span>CZAS OBRONY</span></div><div><strong>${game.wave}</strong><span>FALA</span></div><div><strong>${game.kills}</strong><span>ELIMINACJE</span></div>`;
      updatePointerHint();
    }
  }
  game.events.length = 0;
}
function animate(time) {
  const dt = Math.min((time - (lastTime || time)) / 1000, 0.05); lastTime = time;
  if (started && !game.paused && !game.over) {
    accumulator += dt;
    while (accumulator >= 1 / 60) { stepGame(game, 1 / 60); accumulator -= 1 / 60; }
  }
  updateCamera(dt); updatePlacement();
  world.sync(game, game.paused ? 0 : dt, mode === 'tactical' || !started);
  processEvents();
  $('damage-flash').style.opacity = game.coreHp < previousHp ? '0.28' : '0'; previousHp = game.coreHp;
  hudTimer += dt; if (hudTimer > 0.1) { updateHud(); hudTimer = 0; }
  if (toastTimer > 0) { toastTimer -= dt; if (toastTimer <= 0) $('toast').classList.remove('visible'); }
  world.renderer.render(world.scene, world.camera);
}
world.renderer.setAnimationLoop(animate);
// Explicit opt-in diagnostics, available only in local development for browser tests.
if (import.meta.env.DEV && new URLSearchParams(location.search).has('test')) {
  window.__game = { get state() { return game; }, get mode() { return mode; }, get player() { return player; }, world,
    setMode: next => setMode(next, false), spawn: options => spawnEnemy(game, options), build: (x, z) => buildTurret(game, x, z),
    step: seconds => { for (let t = 0; t < seconds; t += 1 / 60) stepGame(game, 1 / 60); }, startWave: () => startWave(game) };
}
