// Small synthesized effects: no network files, no autoplay before interaction.
export class GameAudio {
  constructor() { this.enabled = true; this.ctx = null; this.lastShot = 0; }
  resume() { try { this.ctx ??= new (window.AudioContext || window.webkitAudioContext)(); this.ctx.resume(); } catch { /* Audio is optional. */ } }
  tone(from, to, duration, volume, type = 'sine') {
    if (!this.enabled || !this.ctx || this.ctx.state !== 'running') return;
    const t = this.ctx.currentTime, oscillator = this.ctx.createOscillator(), gain = this.ctx.createGain();
    oscillator.type = type; oscillator.frequency.setValueAtTime(from, t); oscillator.frequency.exponentialRampToValueAtTime(to, t + duration);
    gain.gain.setValueAtTime(volume, t); gain.gain.exponentialRampToValueAtTime(0.001, t + duration);
    oscillator.connect(gain); gain.connect(this.ctx.destination); oscillator.start(t); oscillator.stop(t + duration);
    oscillator.onended = () => { oscillator.disconnect(); gain.disconnect(); };
  }
  shot() { const t = this.ctx?.currentTime ?? 0; if (t - this.lastShot < 0.045) return; this.lastShot = t; this.tone(950, 150, 0.11, 0.045, 'sawtooth'); }
  build() { this.tone(300, 800, 0.18, 0.1, 'sine'); }
  wave() { this.tone(180, 95, 0.7, 0.12, 'triangle'); }
}
