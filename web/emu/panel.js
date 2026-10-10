// SPDX-License-Identifier: GPL-3.0-only
// The FM-1 panel of the virtual device: a home-made silhouette in SVG (27 keys with the black keys'
// engraved legends, 14 buttons, 7 encoders, MASTER, the screen), with an invisible HTML target over
// each control, because iOS Safari honours touch-action and touchstart on HTML elements, not on SVG.
// Pointers and the computer keyboard hold controls through the same map, so a mouse, ten fingers
// and the keyboard combine. Right-click, or a long press (0.5 s) that touched nothing else, latches a
// button (ADR-018). Geometry and input handling after X0X's and FoMni's web/emu/index.html
// (Charles Vestal), rewritten for Trawler. A plain script: it sets globalThis.FM80SPanel.
(function () {
  const NS = "http://www.w3.org/2000/svg";
  const KM = globalThis.FM80S_KEYMAP;
  const { B, E } = KM;
  const VW = 1000, VH = 640;
  const WHITE_K = [0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26];   // host_white
  const BLACK_K = [1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25];                     // host_black
  const BLACK_AFTER = [0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14];   // the white key each black one follows
  const ENGRAVED = ["OP1", "OP2", "OP3", "OP4", "OP5", "OP6", "PIT", "GLO", "MONO", "POLY", ""];  // 01_FM1_HARDWARE.md
  const C = { body: "#3a3d42", tray: "#26282c", keyW: "#d9dadc", keyB: "#c9cacd", knob: "#1f2124",
              btn: "#2e3135", label: "#e8e8ea", engraved: "#9da2a8", screen: "#111214" };
  const LONG_MS = 500;

  function el(tag, attrs, parent) {
    const e = document.createElementNS(NS, tag);
    for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, v);
    if (parent) parent.appendChild(e);
    return e;
  }
  function text(parent, x, y, s, size, fill, cls, anchor) {
    const t = el("text", { x, y, "text-anchor": anchor || "middle", "font-size": size, fill: fill || C.label, class: cls || "lbl" }, parent);
    t.textContent = s;
    return t;
  }

  // the pointer's up and moves follow it off the control (not possible for every pointer: never mind)
  function capture(d, e) { try { d.setPointerCapture(e.pointerId); } catch { /* released by pointerup on the root */ } }

  function create(root, opts) {
    const send = opts.send;
    const svg = el("svg", { viewBox: `0 0 ${VW} ${VH}`, "aria-hidden": "true" });
    root.appendChild(svg);
    const controls = [];            // {kind: "key"|"button"|"enc"|"master", id, g, hint, box, hit}

    // ------------------------------------------------------------------------------- drawing
    el("rect", { x: 4, y: 4, width: 992, height: 632, rx: 44, fill: C.body }, svg);

    function knob(x, y, r, label, role, cap) {    // cap: the colour of KNOB1-4, as their gauges on the screen
      const g = el("g", { class: "knob" }, svg);
      el("circle", { cx: x, cy: y, r: r + 5, fill: C.tray }, g);
      el("circle", { cx: x, cy: y, r, fill: cap || C.knob, stroke: "#44474c", "stroke-width": 2 }, g);
      const ind = el("line", { x1: x, y1: y - r + 5, x2: x, y2: y - r + 15, stroke: cap ? C.screen : C.label, "stroke-width": 3, "stroke-linecap": "round" }, g);
      text(g, x, y - r - 12, label, 16);
      const hint = text(g, x, y + r + 21, "", 12, C.engraved, "hint");
      const R = r + 14;
      const c = { kind: role === "master" ? "master" : "enc", role, label, g, ind, x, y, angle: 0, hint,
                  box: [x - R, y - R - 8, 2 * R, 2 * R + 16] };
      controls.push(c);
      return c;
    }
    const master = knob(105, 100, 24, "MASTER", "master");
    knob(215, 100, 24, "SELECT", E.SELECT);
    knob(105, 205, 24, "PRESETS", E.PRESETS);
    knob(215, 205, 24, "ALGORITHM", E.ALGO);
    const caps = ["#2ee6ff", "#ff3fa4", "#ffd84a", "#4cf08a"];    // TH_K1..TH_K4 of firmware/src/ui/gfx.h
    [E.K1, E.K2, E.K3, E.K4].forEach((role, i) => knob(610 + i * 104, 100, 22, `KNOB${i + 1}`, role, caps[i]));

    function button(x, y, w, h, bit, label, two) {
      const g = el("g", { class: "button" }, svg);
      el("rect", { x, y, width: w, height: h, rx: 9, fill: C.btn, stroke: "#4a4d52", "stroke-width": 1.5, class: "face" }, g);
      if (two) { text(g, x + w / 2, y + h / 2 + 2, label, 12); text(g, x + w / 2, y + h / 2 + 15, two, 12); }
      else text(g, x + w / 2, y + h / 2 + 7, label, 15);
      const hint = text(g, x + w - 6, y + 13, "", 11, C.engraved, "hint", "end");
      controls.push({ kind: "button", bit, label: two ? `${label}/${two}` : label, g, hint, box: [x, y, w, h] });
    }
    el("rect", { x: 58, y: 258, width: 210, height: 58, rx: 14, fill: C.tray }, svg);
    button(72, 268, 82, 38, B.OCTDN, "OCT−");
    button(170, 268, 82, 38, B.OCTUP, "OCT+");

    el("rect", { x: 300, y: 44, width: 236, height: 236, rx: 26, fill: C.screen }, svg);
    const canvas = document.createElement("canvas");
    canvas.width = canvas.height = 240;
    canvas.className = "lcd";
    canvas.setAttribute("aria-label", "L'écran du FM-1");
    Object.assign(canvas.style, { left: `${318 / 10}%`, top: `${62 / 6.4}%`, width: `${200 / 10}%`, height: `${200 / 6.4}%` });
    root.appendChild(canvas);
    // sharp pixels when the screen gets at least one device pixel per LCD pixel; smoothed below that,
    // where dropping whole pixels would eat the text (a phone in portrait, a small window)
    const sharpness = () => canvas.classList.toggle("small", canvas.getBoundingClientRect().width * (devicePixelRatio || 1) < 240);
    if (globalThis.ResizeObserver) new ResizeObserver(sharpness).observe(canvas);
    else addEventListener("resize", sharpness);

    el("rect", { x: 568, y: 160, width: 400, height: 140, rx: 18, fill: C.tray }, svg);
    const ROW1 = [[B.FX, "FX"], [B.SEL, "SEL"], [B.ENV, "ENV"], [B.LFO, "LFO"], [B.EDIT, "EDIT"], [B.GLO, "GLO"]];
    const ROW2 = [[B.HOME, "HOME"], [B.SAVE, "SAVE"], [B.ARP, "ARP"], [B.SEQ, "SEQ"], [B.PLAY, "PLAY", "STOP"], [B.REC, "REC"]];
    ROW1.forEach(([bit, l], i) => button(582 + i * 63, 174, 54, 48, bit, l));
    ROW2.forEach(([bit, l, two], i) => button(582 + i * 63, 236, 54, 48, bit, l, two));

    (function keybed(x0, y0, w, h) {
      el("rect", { x: x0, y: y0, width: w, height: h, rx: 18, fill: C.tray }, svg);
      const pitch = (w - 24) / 16, kw = pitch * 0.78, by = y0 + h * 0.08, bh = h * 0.31, wy = y0 + h * 0.43, wh = h * 0.52;
      for (let i = 0; i < 16; i++) {
        const g = el("g", { class: "key white" }, svg), x = x0 + 12 + i * pitch + (pitch - kw) / 2;
        el("rect", { x, y: wy, width: kw, height: wh, rx: kw / 2, fill: C.keyW, class: "face" }, g);
        const hint = text(g, x + kw / 2, wy + wh - 16, "", 14, "#5b5f65", "hint");
        controls.push({ kind: "key", bit: WHITE_K[i], g, hint, box: [x - (pitch - kw) / 2, wy, pitch, wh] });
      }
      BLACK_AFTER.forEach((a, k) => {
        const g = el("g", { class: "key black" }, svg), x = x0 + 12 + (a + 1) * pitch - kw / 2;
        el("rect", { x, y: by, width: kw, height: bh, rx: kw / 2, fill: C.keyB, class: "face" }, g);
        const hint = text(g, x + kw / 2, by + bh - 13, "", 14, "#5b5f65", "hint");
        if (ENGRAVED[k]) text(svg, x + kw / 2, by - 7, ENGRAVED[k], 11, C.engraved, "engraved");
        controls.push({ kind: "key", bit: BLACK_K[k], g, hint, box: [x - (pitch - kw) / 2, by, pitch, bh] });
      });
      for (const a of [3, 6, 10, 13])      // the dots between the groups, where the black row has a gap
        el("circle", { cx: x0 + 12 + (a + 1) * pitch, cy: by + bh / 2, r: 3, fill: C.engraved }, svg);
    })(36, 340, 928, 270);

    // ------------------------------------------------------------------------------- state
    const holds = new Map();        // "p<pointerId>" | "k<code>" -> {kind: "b"|"k", bit}
    let latched = 0, sentB = -1, sentK = -1, presses = 0, masterV = opts.master ?? 2048;
    const byBit = { b: [], k: [] };
    for (const c of controls) if (c.kind === "button") byBit.b[c.bit] = c; else if (c.kind === "key") byBit.k[c.bit] = c;

    function sync() {
      let b = latched, k = 0;
      for (const h of holds.values()) if (h.kind === "b") b |= 1 << h.bit; else k |= 1 << h.bit;
      if (b !== sentB) { sentB = b; send({ type: "buttons", mask: b >>> 0 }); }
      if (k !== sentK) { sentK = k; send({ type: "keys", mask: k >>> 0 }); }
      byBit.b.forEach((c, i) => {
        if (!c) return;
        c.g.classList.toggle("held", !!((b >> i) & 1));
        c.g.classList.toggle("latched", !!((latched >> i) & 1));
        c.hit.setAttribute("aria-pressed", (b >> i) & 1 ? "true" : "false");
      });
      byBit.k.forEach((c, i) => c.g.classList.toggle("held", !!((k >> i) & 1)));
    }
    function hold(id, kind, bit) { presses++; holds.set(id, { kind, bit }); sync(); }
    function release(id) { if (holds.delete(id)) sync(); }

    function setMaster(v) {
      masterV = Math.max(0, Math.min(4096, Math.round(v)));
      master.angle = -135 + (270 * masterV) / 4096;
      master.ind.setAttribute("transform", `rotate(${master.angle} ${master.x} ${master.y})`);
      master.hit.setAttribute("aria-valuenow", String(masterV));
      send({ type: "master", value: masterV });
    }
    function turn(c, n) {
      if (!n) return;
      if (c.kind === "master") { setMaster(masterV + n * 64); return; }
      c.angle = (c.angle + n * 18) % 360;
      c.ind.setAttribute("transform", `rotate(${c.angle} ${c.x} ${c.y})`);
      send({ type: "enc", role: c.role, n });
    }

    // ------------------------------------------------------------------------------- targets
    for (const c of controls) {
      const d = document.createElement("div"), [x, y, w, h] = c.box;
      d.className = "hit " + c.kind;
      Object.assign(d.style, { left: `${x / 10}%`, top: `${(y / VH) * 100}%`, width: `${w / 10}%`, height: `${(h / VH) * 100}%` });
      d.tabIndex = c.kind === "key" ? -1 : 0;
      if (c.kind === "key") d.setAttribute("aria-label", `touche ${c.bit}`);
      else d.setAttribute("aria-label", c.label);
      d.setAttribute("role", c.kind === "button" || c.kind === "key" ? "button" : "slider");
      d.dataset.kind = c.kind;
      if (c.kind === "key" || c.kind === "button") d.dataset.bit = c.bit;
      else d.dataset.role = c.role;
      d.addEventListener("touchstart", (e) => e.preventDefault(), { passive: false });
      d.addEventListener("contextmenu", (e) => e.preventDefault());
      root.appendChild(d);
      c.hit = d;
    }

    for (const c of controls.filter((c) => c.kind === "key")) {
      c.hit.addEventListener("pointerdown", (e) => {
        e.preventDefault();
        if (e.button === 2) return;
        capture(c.hit, e);
        hold("p" + e.pointerId, "k", c.bit);
      });
    }

    for (const c of controls.filter((c) => c.kind === "button")) {
      c.hit.addEventListener("pointerdown", (e) => {
        e.preventDefault();
        const bit = 1 << c.bit;
        if (e.button === 2 || (e.ctrlKey && e.pointerType === "mouse")) {           // right-click (or Ctrl-click): latch
          presses++; latched ^= bit; sync(); return;
        }
        if (latched & bit) { presses++; latched &= ~bit; sync(); return; }           // a latched one: unlatch
        capture(c.hit, e);
        const id = "p" + e.pointerId;
        hold(id, "b", c.bit);
        if (e.pointerType !== "mouse") {                                             // a long press latches...
          const mark = presses;
          setTimeout(() => {                                                         // ...if it touched nothing else
            if (holds.has(id) && holds.get(id).bit === c.bit && presses === mark) { latched |= bit; sync(); }
          }, LONG_MS);
        }
      });
    }

    for (const c of controls.filter((c) => c.kind === "enc" || c.kind === "master")) {
      let drag = null, wheel = 0;
      const step = c.kind === "master" ? 3 : 7;                                      // px per detent
      c.hit.addEventListener("pointerdown", (e) => {
        e.preventDefault();
        capture(c.hit, e);
        drag = { id: e.pointerId, y: e.clientY, acc: 0 };
      });
      c.hit.addEventListener("pointermove", (e) => {
        if (!drag || e.pointerId !== drag.id) return;
        drag.acc += drag.y - e.clientY;
        drag.y = e.clientY;
        const n = Math.trunc(drag.acc / step);
        if (n) { drag.acc -= n * step; turn(c, n); }
      });
      const end = (e) => { if (drag && e.pointerId === drag.id) drag = null; };
      c.hit.addEventListener("pointerup", end);
      c.hit.addEventListener("pointercancel", end);
      c.hit.addEventListener("wheel", (e) => {
        e.preventDefault();
        const dy = e.deltaMode === 1 ? e.deltaY * 33 : e.deltaY;
        if (Math.abs(dy) >= 50) { turn(c, dy < 0 ? 1 : -1); wheel = 0; return; }    // a mouse wheel notch: a detent
        wheel -= dy;                                                                  // a trackpad: by distance
        const n = Math.trunc(wheel / 25);
        if (n) { wheel -= n * 25; turn(c, n); }
      }, { passive: false });
      c.hit.addEventListener("keydown", (e) => {                                     // focused: the arrows turn it
        const n = { ArrowUp: 1, ArrowRight: 1, ArrowDown: -1, ArrowLeft: -1 }[e.key];
        if (n) { turn(c, n); e.preventDefault(); e.stopPropagation(); }
      });
    }
    master.hit.setAttribute("aria-valuemin", "0");
    master.hit.setAttribute("aria-valuemax", "4096");

    function pointerUp(e) { release("p" + e.pointerId); }
    root.addEventListener("pointerup", pointerUp);
    root.addEventListener("pointercancel", pointerUp);
    root.addEventListener("lostpointercapture", pointerUp);

    // ------------------------------------------------------------------------------- legends
    const encLegend = {
      [E.SELECT]: ["ArrowUp", "ArrowDown"], [E.ALGO]: ["ArrowLeft", "ArrowRight"], [E.PRESETS]: ["BracketLeft", "BracketRight"],
    };
    function setLegend(layout, shown) {
      const L = KM.layouts[layout] || KM.layouts.qwerty, lab = (code) => KM.label(layout, code);
      for (const c of controls) {
        let s = "";
        if (!shown) s = "";
        else if (c.kind === "key") s = lab(KM.codeOfKey(c.bit));
        else if (c.kind === "button") s = c.bit === B.SAVE ? "ctrl S" : lab(KM.codeOfButton(c.bit));
        else if (c.kind === "enc" && encLegend[c.role]) s = encLegend[c.role].map(lab).join(" ");
        else if (c.kind === "enc") {
          const [m, p] = Object.keys(KM.altTurns).filter((code) => KM.altTurns[code][0] === c.role);
          s = `${L.alt.trim()}${lab(m)} ${L.alt.trim()}${lab(p)}`;
        }
        c.hint.textContent = s;
      }
    }

    sync();
    setMaster(masterV);
    return {
      canvas,
      holdKey: (id, k) => hold(id, "k", k),
      holdButton: (id, b) => hold(id, "b", b),
      release,
      releaseKeyboard() {                       // the window lost the focus: everything the keyboard held
        let any = false;
        for (const id of [...holds.keys()]) if (id[0] === "k") { holds.delete(id); any = true; }
        if (any) sync();
      },
      turn: (role, n) => turn(controls.find((c) => c.kind === "enc" && c.role === role), n),
      get master() { return masterV; },
      resync() { sentB = sentK = -1; sync(); setMaster(masterV); },   // the worklet just started
      leds(m) {
        byBit.b.forEach((c, i) => {
          if (!c) return;
          const on = !!((m.buttons >> i) & 1);
          c.g.classList.toggle("lit", on && !(i === B.PLAY && m.red));
          c.g.classList.toggle("red", i === B.PLAY && !!m.red);
        });
        byBit.k.forEach((c, i) => {
          c.g.classList.toggle("lit", !!((m.keys >> i) & 1));
          c.g.classList.toggle("dim", !((m.keys >> i) & 1) && !!((m.dim >> i) & 1));
        });
      },
      setLegend,
      state: () => ({ buttons: sentB, keys: sentK, latched, master: masterV }),
    };
  }

  globalThis.FM80SPanel = { create };
})();
