// SPDX-License-Identifier: GPL-3.0-only
// The computer keyboard of the virtual FM-1 (spec §4.5), by KeyboardEvent.code, i.e. by physical
// position whatever the layout: the keys always follow the FM-1's in order. Only the legends depend on
// the layout (QWERTY, AZERTY Mac, AZERTY PC). A plain script: the page loads it with <script src>,
// tests/host/emu_test.mjs imports it; both read globalThis.FM80S_KEYMAP.
//
// The musical keyboard is a piano drawn from F, as the FM-1's own keys (key 0 = F3). By QWERTY position:
//   Z X C V B N M        F3 .. E4 (white)    S D F H J   F#3 G#3 A#3 C#4 D#4
//   Q W E R T Y U I O    F4 .. G5 (white)    2 3 4 6 7 9 F#4 G#4 A#4 C#5 D#5 F#5
// The digits are notes; with Option (Alt) the digits 1-8 turn KNOB1-4 -/+ (ADR-017).
(function () {
  // B_* and EN_* of firmware/src/app/plat.h
  const B = { OCTDN: 0, OCTUP: 1, FX: 2, SEL: 3, ENV: 4, LFO: 5, EDIT: 6, GLO: 7, HOME: 8, SAVE: 9, ARP: 10, SEQ: 11, PLAY: 12, REC: 13 };
  const E = { SELECT: 0, ALGO: 1, K1: 2, K2: 3, K3: 4, K4: 5, PRESETS: 6 };

  const keys = {};
  const row = (codes, first) => codes.forEach((c, i) => (keys[c] = first + i));
  // chromatic from F3, white and black codes interleaved (no black key between B and C, E and F)
  row(["KeyZ", "KeyS", "KeyX", "KeyD", "KeyC", "KeyF", "KeyV", "KeyB", "KeyH", "KeyN", "KeyJ", "KeyM"], 0);
  row(["KeyQ", "Digit2", "KeyW", "Digit3", "KeyE", "Digit4", "KeyR", "KeyT", "Digit6", "KeyY", "Digit7", "KeyU",
       "KeyI", "Digit9", "KeyO"], 12);

  // held like the FM-1's buttons; the first code of a button is the one its legend shows
  const buttons = {
    Minus: B.OCTDN, Equal: B.OCTUP, ShiftLeft: B.SEL, ShiftRight: B.SEL, Enter: B.EDIT, NumpadEnter: B.EDIT,
    Space: B.PLAY, Backquote: B.REC, IntlBackslash: B.REC,
    F1: B.HOME, F2: B.ARP, F3: B.SEQ, F4: B.ENV, F5: B.LFO, F6: B.GLO, F7: B.FX,
  };
  const ctrlButtons = { KeyS: B.SAVE };           // Ctrl+S
  // one detent per press, auto-repeat included: [EN_*, detents]
  const turns = {
    ArrowUp: [E.SELECT, -1], ArrowDown: [E.SELECT, 1], ArrowLeft: [E.ALGO, -1], ArrowRight: [E.ALGO, 1],
    BracketLeft: [E.PRESETS, -1], BracketRight: [E.PRESETS, 1],
  };
  const altTurns = {};                            // Option (Alt) + 1..8: KNOB1-4 -/+
  [E.K1, E.K2, E.K3, E.K4].forEach((role, i) => {
    altTurns["Digit" + (2 * i + 1)] = [role, -1];
    altTurns["Digit" + (2 * i + 2)] = [role, 1];
  });

  // ------------------------------------------------------------------ legends: code -> printed label
  const qwerty = {
    Minus: "-", Equal: "=", BracketLeft: "[", BracketRight: "]", Backquote: "`", IntlBackslash: "§",
    ShiftLeft: "⇧", ShiftRight: "⇧", Enter: "↵", NumpadEnter: "↵", Space: "␣",
    ArrowUp: "↑", ArrowDown: "↓", ArrowLeft: "←", ArrowRight: "→",
  };
  for (let c = 65; c <= 90; c++) qwerty["Key" + String.fromCharCode(c)] = String.fromCharCode(c);
  for (let d = 0; d <= 9; d++) qwerty["Digit" + d] = String(d);
  for (let f = 1; f <= 7; f++) qwerty["F" + f] = "F" + f;
  // AZERTY keycaps print the digits too (with Shift): the legends keep them, simpler than é " ' ( § è
  const azerty = { ...qwerty, KeyQ: "A", KeyW: "Z", KeyA: "Q", KeyZ: "W", KeyM: ",", Semicolon: "M",
                   Minus: ")", BracketLeft: "^", BracketRight: "$" };
  // the Mac ISO keyboard reports the key by left Shift as Backquote and the one left of 1 as IntlBackslash
  const layouts = {
    qwerty: { name: "QWERTY", alt: "⌥", labels: qwerty },
    "azerty-mac": { name: "AZERTY (Mac)", alt: "⌥", labels: { ...azerty, Equal: "-", Backquote: "<", IntlBackslash: "@" } },
    "azerty-pc": { name: "AZERTY (PC)", alt: "Alt ", labels: { ...azerty, Equal: "=", Backquote: "²", IntlBackslash: "<" } },
  };

  globalThis.FM80S_KEYMAP = {
    B, E,
    keys,                 // code -> key index, 0 = F3 .. 26 = G5
    buttons,              // code -> B_*
    ctrlButtons,          // Ctrl + code -> B_*
    turns,                // code -> [EN_*, detents]
    altTurns,             // Option / Alt + code -> [EN_*, detents]
    layouts,
    label: (layout, code) => (layouts[layout] || layouts.qwerty).labels[code] || code,
    // the code a control's legend shows: the first that plays it
    codeOfKey: (k) => Object.keys(keys).find((c) => keys[c] === k),
    codeOfButton: (b) => Object.keys(buttons).find((c) => buttons[c] === b),
  };
})();
