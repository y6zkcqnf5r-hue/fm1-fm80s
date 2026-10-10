// SPDX-License-Identifier: GPL-3.0-only
// The browser build against the native one (CLAUDE.md rule 7), without a browser: node >= 20.
//
//   node tests/host/emu_test.mjs          (make parity; needs build/host/fm80s and build/emu/fm80s.wasm)
//
// 1. Parity: build/host/fm80s plays each tests/scenarios/*.trs with --events, which journals what the
//    script did to the device; the wasm replays that journal through the exports the worklet uses.
//    Every WAV it records must hash to its line of tests/golden/<name>.sha256 and equal the native
//    file byte for byte; the screen must have the same FNV-1a at every shot.
// 2. The clock: web_render(128) for 10 minutes of device time ends exactly where host_run_ms says,
//    and gives the same samples as web_run_ms.
// 3. worklet.js itself, under stubs of the AudioWorklet globals, at 44.1 and 48 kHz: a held key
//    sounds and lights, a press and release sent together stay apart >= 24 ms of device time, and the
//    resampler consumes exactly 44100 source frames per second of output.
// 4. keymap.js: KeyZ is F3, the 27 keys are reachable, no key twice.
// Shape from ChoralRoot's tools/emu/web/test_web_emu.mjs (Quixotic7), rewritten for Trawler.
import fs from "node:fs";
import path from "node:path";
import crypto from "node:crypto";
import { execFileSync } from "node:child_process";
import { fileURLToPath, pathToFileURL } from "node:url";

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const WASM = path.join(ROOT, "build/emu/fm80s.wasm");
const HOST = path.join(ROOT, "build/host/fm80s");
const TMP = path.join(ROOT, "build/emu-test");
const FS = 44100, BLOCK = 256;

let fails = 0;
const ok = (good, msg) => { console.log(`  ${good ? "ok" : "FAIL"} ${msg}`); if (!good) fails++; return good; };

for (const f of [WASM, HOST])
  if (!fs.existsSync(f)) { console.log(`  FAIL ${path.relative(ROOT, f)} manquant (make web host)`); process.exit(1); }
const bytes = fs.readFileSync(WASM);

// --------------------------------------------------------------- the module, as the worklet loads it ---
async function load() {
  const stub = new Proxy({}, { get: () => () => 0 });
  const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: stub, env: stub });
  const ex = instance.exports;
  if (ex._initialize) ex._initialize();
  return ex;
}

// take every rendered frame out of the ring, through web_render (it does not run the device then)
function drain(ex, sink) {
  for (let n = ex.web_avail(); n > 0; n = ex.web_avail()) {
    const k = Math.min(1024, n);
    ex.web_render(k);
    const l = new Int16Array(ex.memory.buffer, ex.web_out_l(), k), r = new Int16Array(ex.memory.buffer, ex.web_out_r(), k);
    if (sink) for (let i = 0; i < k; i++) sink.push(l[i], r[i]);
  }
}

function fnv32(u8) {
  let h = 2166136261;
  for (let i = 0; i < u8.length; i++) h = Math.imul(h ^ u8[i], 16777619) >>> 0;
  return h >>> 0;
}

function wavBytes(pcm) {            // host/wav.c's file: 16-bit stereo PCM at 44.1 kHz, little-endian
  const data = pcm.length * 2, b = Buffer.alloc(44 + data);
  b.write("RIFF", 0); b.writeUInt32LE(36 + data, 4); b.write("WAVEfmt ", 8);
  b.writeUInt32LE(16, 16); b.writeUInt16LE(1, 20); b.writeUInt16LE(2, 22);
  b.writeUInt32LE(FS, 24); b.writeUInt32LE(FS * 4, 28); b.writeUInt16LE(4, 32); b.writeUInt16LE(16, 34);
  b.write("data", 36); b.writeUInt32LE(data, 40);
  for (let i = 0; i < pcm.length; i++) b.writeInt16LE(pcm[i], 44 + 2 * i);
  return b;
}

const sha256 = (b) => crypto.createHash("sha256").update(b).digest("hex");

// ========================================================================================== 1 ===
async function parity(name) {
  console.log(`parité wasm / natif : scénario ${name}`);
  const dir = path.join(TMP, name), journal = path.join(dir, "events.txt");
  fs.rmSync(dir, { recursive: true, force: true });
  fs.mkdirSync(dir, { recursive: true });
  try {
    execFileSync(HOST, ["--events", journal, path.join(ROOT, `tests/scenarios/${name}.trs`), dir], { stdio: "pipe" });
  } catch (e) {
    ok(false, `le natif sort avec ${e.status} :\n${e.stdout}`);
    return;
  }
  const golden = {};
  for (const line of fs.readFileSync(path.join(ROOT, `tests/golden/${name}.sha256`), "utf8").split("\n"))
    if (line.trim()) { const [h, f] = line.trim().split(/\s+/); golden[f] = h; }

  const ex = await load();
  ex.web_boot();
  let rec = null, recName = "", wavs = 0, shots = 0;
  for (const line of fs.readFileSync(journal, "utf8").split("\n")) {
    const w = line.split(" ");
    switch (w[0]) {
      case "": break;
      case "b": ex.web_buttons(Number(w[1]) >>> 0); break;
      case "k": ex.web_keys(Number(w[1]) >>> 0); break;
      case "m": ex.web_master(Number(w[1])); break;
      case "e": ex.web_enc(Number(w[1]), Number(w[2])); break;
      case "midi": {
        const p = Number(w[1]) >>> 0;
        ex.web_midi((p >>> 8) & 0xFF, (p >>> 16) & 0x7F, (p >>> 24) & 0x7F);
        break;
      }
      case "run":
        for (let ms = Number(w[1]); ms > 0; ms--) { ex.web_run_ms(1); drain(ex, rec); }
        break;
      case "wav":
        if (w[1] === "on") { rec = []; recName = w[2]; break; }
        {
          const got = wavBytes(rec), nat = fs.readFileSync(path.join(dir, recName));
          wavs++;
          ok(sha256(got) === golden[recName], `${recName} : sha256 ${sha256(got).slice(0, 12)}, golden ${(golden[recName] || "absent").slice(0, 12)}`);
          if (!got.equals(nat)) {
            let i = 44;
            while (i < Math.min(got.length, nat.length) && got[i] === nat[i]) i++;
            ok(false, `${recName} : ${got.length} octets en wasm, ${nat.length} en natif ; première frame différente : ${(i - 44) >> 2}`);
          } else ok(true, `${recName} : ${rec.length / 2} frames identiques au WAV natif`);
          rec = null;
        }
        break;
      case "shot": {
        const h = fnv32(new Uint8Array(ex.memory.buffer, ex.web_fb(), 240 * 240 * 2));
        shots++;
        ok(h === Number(w[2]) >>> 0, `${w[1]} : écran FNV 0x${h.toString(16).padStart(8, "0")}, natif ${w[2]}`);
        break;
      }
      default: ok(false, `journal : ligne inconnue « ${line} »`);
    }
  }
  const want = (ext) => Object.keys(golden).filter((f) => f.endsWith(ext)).length;
  ok(wavs === want(".wav") && shots === want(".png") && wavs + shots > 0,
     `${wavs} WAV et ${shots} captures rejoués (golden : ${want(".wav")} et ${want(".png")}), ${ex.web_now()} ms d'appareil`);
}

// ========================================================================================== 2 ===
async function clock() {
  console.log("horloge : web_render(128) comme le worklet");
  const ex = await load();
  ex.web_boot();
  const N = Math.ceil((10 * 60 * FS) / 128);   // 10 minutes of quanta
  for (let q = 0; q < N; q++) ex.web_render(128);
  const F = N * 128, blocks = Math.ceil(F / BLOCK), ms = Math.ceil((blocks * BLOCK * 1000) / FS);
  ok(ex.web_now() === ms && ex.web_avail() === blocks * BLOCK - F,
     `10 min : ${F} frames, appareil à ${ex.web_now()} ms (attendu ${ms}), ${ex.web_avail()} frames d'avance (attendu ${blocks * BLOCK - F})`);

  // the same samples whichever way the device is run (a note held from boot)
  const a = await load(), b = await load(), pa = [], pb = [];
  a.web_boot(); b.web_boot();
  a.web_keys(1 << 7); b.web_keys(1 << 7);
  for (let q = 0; q < 1000; q++) {
    a.web_render(128);
    const l = new Int16Array(a.memory.buffer, a.web_out_l(), 128), r = new Int16Array(a.memory.buffer, a.web_out_r(), 128);
    for (let i = 0; i < 128; i++) pa.push(l[i], r[i]);
  }
  while (pb.length < pa.length) { b.web_run_ms(1); drain(b, pb); }
  let same = pa.some((v) => v !== 0);
  for (let i = 0; i < pa.length; i++) if (pa[i] !== pb[i]) { same = false; break; }
  ok(same, `${pa.length / 2} frames (une note tenue) : identiques par web_render et par web_run_ms`);
}

// ========================================================================================== 3 ===
let Proc = null;
async function worklet() {
  globalThis.AudioWorkletProcessor = class {
    constructor() { const sent = []; this.port = { sent, postMessage: (m) => sent.push(m), onmessage: null }; }
  };
  globalThis.registerProcessor = (name, cls) => { Proc = cls; };
  globalThis.sampleRate = FS;
  globalThis.currentTime = 0;
  await import(pathToFileURL(path.join(ROOT, "web/emu/worklet.js")).href);
  ok(typeof Proc === "function", "worklet.js : registerProcessor(\"fm80s\")");

  for (const rate of [44100, 48000]) {
    console.log(`worklet.js à ${rate} Hz`);
    globalThis.sampleRate = rate;
    const p = new Proc();
    p.port.onmessage({ data: { type: "load", wasm: bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.length), master: 2048 } });
    for (let i = 0; i < 100 && !p.port.sent.some((m) => m.type === "ready"); i++) await new Promise((r) => setTimeout(r, 5));
    if (!ok(p.port.sent.some((m) => m.type === "ready"), "load → ready")) continue;
    const out = [[new Float32Array(128), new Float32Array(128)]];
    let nz = 0, peak = 0;
    const quantum = () => {
      p.process([], out);
      globalThis.currentTime += 128 / rate;
      for (const ch of out[0]) for (const v of ch) { if (v !== 0) nz++; peak = Math.max(peak, Math.abs(v)); }
    };
    const seconds = (s) => { for (let q = Math.round((s * rate) / 128); q > 0; q--) quantum(); };

    seconds(0.5);
    ok(nz === 0, `0,5 s sans touche : ${nz} échantillons non nuls`);
    p.port.onmessage({ data: { type: "keys", mask: 1 } });                 // KeyZ: F3
    seconds(0.5);
    const lit = p.ex.web_lit_keys();
    const frames = p.port.sent.filter((m) => m.type === "frame");
    ok(nz > 0 && peak > 0.2 && peak <= 1, `touche F3 tenue : ${nz} échantillons non nuls, pic ${peak.toFixed(3)}`);
    ok(lit === 1 && frames.at(-1).keys === 1, `touche F3 allumée (web_lit_keys ${lit}, dernier frame keys ${frames.at(-1).keys})`);
    ok(frames.length >= 25 && frames.length <= 40 && frames.some((m) => m.fb) && !frames.at(-1).fb,
       `${frames.length} messages frame en 1 s, fb seulement quand l'écran change`);
    p.port.onmessage({ data: { type: "keys", mask: 0 } });
    seconds(0.2);

    // a tap shorter than a quantum: press and release in the same breath
    p.port.onmessage({ data: { type: "keys", mask: 1 } });
    p.port.onmessage({ data: { type: "keys", mask: 0 } });
    const at = [];
    let sawLit = false;
    for (let q = 0; q < 100 && p.pend.keys.length; q++) {
      const before = p.pend.keys.length;
      quantum();
      if (p.pend.keys.length < before) at.push(p.since.keys);
      sawLit ||= p.ex.web_lit_keys() === 1;
    }
    // the device's clock moves a block (256 frames, < 6 ms) at a time: the release lands within one block
    ok(at.length === 2 && at[1] - at[0] >= 24 && at[1] - at[0] <= 24 + 6,
       `appui et relâché envoyés ensemble : appliqués à ${at.join(" et ")} ms d'appareil (≥ 24 ms d'écart)`);
    ok(sawLit, "l'app a vu l'appui court (touche allumée)");

    // the resampler: exactly 44100 device frames per second of output, whatever the browser's rate
    const p2 = new Proc();
    p2.port.onmessage({ data: { type: "load", wasm: bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.length) } });
    for (let i = 0; i < 100 && !p2.ex; i++) await new Promise((r) => setTimeout(r, 5));
    const Q = Math.round((60 * rate) / 128);                              // ~1 min of quanta
    for (let q = 0; q < Q; q++) p2.process([], out);
    const want = Math.floor((Q * 128 * FS) / rate);                      // source frames consumed
    const produced = BLOCK * Math.floor((p2.ex.web_now() * FS) / (BLOCK * 1000));
    ok(produced - p2.ex.web_avail() === want, `${Q} quanta : ${produced - p2.ex.web_avail()} frames d'appareil consommées (attendu ${want})`);
  }
}

// ========================================================================================== 4 ===
async function keymap() {
  console.log("keymap.js");
  await import(pathToFileURL(path.join(ROOT, "web/emu/keymap.js")).href);
  const KM = globalThis.FM80S_KEYMAP, { B, E } = KM;
  const idx = Object.values(KM.keys).sort((a, b) => a - b);
  ok(KM.keys.KeyZ === 0, "KeyZ = touche 0 (F3)");
  ok(idx.length === 27 && idx.every((k, i) => k === i), `les 27 touches, chacune une fois (${idx.length} codes)`);
  const plain = [KM.keys, KM.buttons, KM.turns].flatMap(Object.keys);
  ok(new Set(plain).size === plain.length, "sans modificateur, aucun code n'a deux rôles");
  const btns = new Set(Object.values(KM.buttons).concat(Object.values(KM.ctrlButtons)));
  ok(btns.size === 14, `les 14 boutons au clavier (${btns.size})`);
  const roles = new Set(Object.values(KM.turns).concat(Object.values(KM.altTurns)).map(([r]) => r));
  ok(roles.size === 7, `les 7 encodeurs au clavier (${roles.size})`);
  ok(KM.buttons.Minus === B.OCTDN && KM.buttons.Equal === B.OCTUP && KM.buttons.ShiftLeft === B.SEL && KM.ctrlButtons.KeyS === B.SAVE,
     "- = OCT− / OCT+, Maj = SEL, Ctrl+S = SAVE");
  ok(KM.altTurns.Digit1.join() === `${E.K1},-1` && KM.altTurns.Digit8.join() === `${E.K4},1`, "⌥1 = KNOB1 −, ⌥8 = KNOB4 +");
  const all = plain.concat(Object.keys(KM.ctrlButtons), Object.keys(KM.altTurns));
  for (const [id, L] of Object.entries(KM.layouts)) {
    const missing = all.filter((c) => !L.labels[c]);
    ok(!missing.length, `${L.name} : une légende pour chaque touche${missing.length ? " (manque " + missing.join(" ") + ")" : ""}`);
  }
  ok(KM.label("azerty-mac", "KeyZ") === "W" && KM.label("azerty-pc", "KeyQ") === "A" && KM.label("azerty-mac", "KeyM") === ",",
     "AZERTY : F3 sur W, F4 sur A, E4 sur la virgule");
}

for (const f of fs.readdirSync(path.join(ROOT, "tests/scenarios")).filter((f) => f.endsWith(".trs")).sort())
  await parity(f.slice(0, -4));
await clock();
await worklet();
await keymap();
console.log(fails ? `emu_test : ${fails} faute(s)` : "emu_test : ok");
process.exit(fails ? 1 : 0);
