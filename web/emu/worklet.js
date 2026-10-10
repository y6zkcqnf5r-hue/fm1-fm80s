// SPDX-License-Identifier: GPL-3.0-only
// FM80S in an AudioWorklet: fm80s.wasm is the whole FM-1 app on the simulated device
// (web/emu/fm80s_web.c). Each render quantum asks it for frames, which runs the device's clock
// forward; between quanta the page's input goes in and, about 30 times a second (4 when the tab is
// hidden), the screen, the lights and the saved slots go out. Shape from X0X's and FoMni's
// web/emu/worklet.js (Charles Vestal): the stub WASI, the load / frame messages.
//
// Page -> worklet: {type: "load", wasm, store, master}, {type: "buttons" | "keys", mask},
//   {type: "enc", role, n}, {type: "master", value}, {type: "midi", data: [st, d1, d2]},
//   {type: "visible", value}.
// Worklet -> page: {type: "ready", rate}, {type: "error", message}, then
//   {type: "frame", fb?, buttons, keys, dim, red, store?, cpu, load?, now, frames, nz, peak}.
//   fb (Uint16Array, RGB565 byte-swapped) only when the screen was drawn since the last one.

const SRC = 44100;          // the device's rate
const HOLD_MS = 24;         // each button / key change stays at least this long, in device time
const clock = globalThis.performance ? () => globalThis.performance.now() : () => Date.now();

class FM80S extends AudioWorkletProcessor {
  constructor() {
    super();
    this.ex = null;
    this.rate = sampleRate;
    this.pend = { buttons: [], keys: [] };          // changes waiting for the previous one's HOLD_MS
    this.since = { buttons: -HOLD_MS, keys: -HOLD_MS };
    this.visible = true;
    this.lastPub = -1;
    this.sentBlits = -1;
    this.sentWrites = -1;
    this.frames = 0;        // output frames since load
    this.nz = 0;            // non-zero output samples since the last frame message
    this.busy = 0;
    this.busyFrames = 0;
    // linear resampling, integer phase: acc / rate is the position between source frames s0 and s1
    this.acc = 0;
    this.s0l = this.s0r = this.s1l = this.s1r = 0;
    this.port.onmessage = (e) => this.onMessage(e.data);
  }

  async load(m) {
    const wasi = new Proxy({}, { get: () => () => 0 });   // nothing is imported today: a stub WASI just in case
    const { instance } = await WebAssembly.instantiate(m.wasm, { wasi_snapshot_preview1: wasi, env: wasi });
    const ex = instance.exports;
    if (ex._initialize) ex._initialize();
    const max = ex.web_store_max();
    for (const [slot, bytes] of Object.entries(m.store || {})) {
      const b = new Uint8Array(bytes).subarray(0, max);
      new Uint8Array(ex.memory.buffer, ex.web_store_ptr(+slot), b.length).set(b);
      ex.web_store_set_len(+slot, b.length);
    }
    if (m.master != null) ex.web_master(m.master | 0);
    ex.web_boot();
    this.sentWrites = ex.web_store_writes();
    this.ex = ex;
    this.port.postMessage({ type: "ready", rate: this.rate });
  }

  onMessage(m) {
    if (m.type === "load") {
      this.load(m).catch((err) => this.port.postMessage({ type: "error", message: String(err) }));
      return;
    }
    if (m.type === "visible") { this.visible = !!m.value; return; }
    const ex = this.ex;
    if (!ex) return;
    if (m.type === "buttons" || m.type === "keys") this.pend[m.type].push(m.mask >>> 0);
    else if (m.type === "enc") ex.web_enc(m.role | 0, m.n | 0);
    else if (m.type === "master") ex.web_master(m.value | 0);
    else if (m.type === "midi") ex.web_midi(m.data[0] | 0, m.data[1] | 0, m.data[2] | 0);
  }

  // one pending change per kind, once the last one has been held HOLD_MS of device time
  pump() {
    const now = this.ex.web_now();
    for (const kind of ["buttons", "keys"]) {
      const q = this.pend[kind];
      if (q.length && now - this.since[kind] >= HOLD_MS) {
        (kind === "buttons" ? this.ex.web_buttons : this.ex.web_keys)(q.shift());
        this.since[kind] = now;
      }
    }
  }

  publish() {
    const ex = this.ex, mem = ex.memory.buffer;
    const msg = {
      type: "frame", buttons: ex.web_lit_buttons() >>> 0, keys: ex.web_lit_keys() >>> 0, dim: ex.web_dim_keys() >>> 0,
      red: ex.web_play_red(), cpu: ex.web_cpu(), now: ex.web_now() >>> 0, frames: this.frames, nz: this.nz, peak: ex.web_peak(),
    };
    this.nz = 0;
    if (this.busyFrames >= this.rate) {             // the share of real time spent running the device
      msg.load = Math.round((100 * this.busy) / ((1000 * this.busyFrames) / this.rate));
      this.busy = this.busyFrames = 0;
    }
    const transfer = [];
    const blits = ex.web_blits();
    if (blits !== this.sentBlits) {
      this.sentBlits = blits;
      msg.fb = new Uint16Array(mem, ex.web_fb(), 240 * 240).slice();
      transfer.push(msg.fb.buffer);
    }
    const writes = ex.web_store_writes();
    if (writes !== this.sentWrites) {
      this.sentWrites = writes;
      msg.store = {};
      for (let s = 0; s < ex.web_slots(); s++) {
        const n = ex.web_store_len(s);
        if (n >= 0) msg.store[s] = new Uint8Array(mem, ex.web_store_ptr(s), n).slice();
      }
    }
    this.port.postMessage(msg, transfer);
  }

  process(inputs, outputs) {
    const ex = this.ex;
    if (!ex) return true;
    const L = outputs[0][0], n = L.length, R = outputs[0][1] || new Float32Array(n), K = 1 / 32768;
    let nz = 0;
    const t0 = clock();
    this.pump();
    if (this.rate === SRC) {
      ex.web_render(n);
      const l = new Int16Array(ex.memory.buffer, ex.web_out_l(), n), r = new Int16Array(ex.memory.buffer, ex.web_out_r(), n);
      for (let i = 0; i < n; i++) {
        L[i] = l[i] * K;
        R[i] = r[i] * K;
        nz += (l[i] !== 0) + (r[i] !== 0);
      }
    } else {
      // exactly the source frames this quantum steps over (<= 1024 down to a 5.6 kHz browser)
      const rate = this.rate, need = Math.floor((this.acc + n * SRC) / rate);
      ex.web_render(need);
      const l = new Int16Array(ex.memory.buffer, ex.web_out_l(), need), r = new Int16Array(ex.memory.buffer, ex.web_out_r(), need);
      let j = 0;
      for (let i = 0; i < n; i++) {
        const f = this.acc / rate;
        const a = this.s0l + (this.s1l - this.s0l) * f, b = this.s0r + (this.s1r - this.s0r) * f;
        L[i] = a * K;
        R[i] = b * K;
        nz += (a !== 0) + (b !== 0);
        this.acc += SRC;
        while (this.acc >= rate) {
          this.acc -= rate;
          this.s0l = this.s1l;
          this.s0r = this.s1r;
          this.s1l = l[j];
          this.s1r = r[j];
          j++;
        }
      }
    }
    this.busy += clock() - t0;
    this.busyFrames += n;
    this.frames += n;
    this.nz += nz;
    if (currentTime - this.lastPub >= (this.visible ? 1 / 30 : 1 / 4) || this.lastPub < 0) {
      this.lastPub = currentTime;
      this.publish();
    }
    return true;
  }
}

registerProcessor("fm80s", FM80S);
