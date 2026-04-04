# Roadmap

Phases **0–4** are **complete** through public release **0.1.0** — [`release-0.1.0.md`](notes/release-0.1.0.md).

## Phase 0 — Pre-production

**Status:** completed

- define scope
- choose toolchain
- choose visual representation strategy
- choose input model
- decide minimal audio direction

## Phase 1 — Core prototype

**Status:** completed

- board model
- piece model
- movement and rotation
- collision and locking
- line clear
- score
- host-side tests and debug harness

## Phase 2 — First playable C64 build

**Status:** completed

- C64 entry point
- rendering path
- input polling
- timing loop
- minimal presentation layer
- repeated playtests

## Phase 2.5 — Presentation identity

**Status:** completed (ROM/PETSCII only; custom charset deferred.)

- Palette: black background, border = background ($D020 = $D021), frame 15, locked 1, active 3, HUD 15.
- Layout: board centred (BOARD_SCREEN_X = 15), HUD left (HUD_SCREEN_COL = 1).
- Game over: 3-row light grey band inside board, "GAME  OVER" reverse (message on field).
- Custom charset: deferred; see [`custom-charset-investigation.md`](notes/custom-charset-investigation.md).

## Phase 2.6 — Interface composition

**Status:** completed

- State flow: Title → StartHelp → Game → GameOver (with replay) → StartHelp. Boot starts at Title.
- Title: block-built QUATTRO wordmark, one prompt line (PRESS ANY KEY). Start/help: start level 0–9 (keys 0–9), RETURN to start; controls listed (A/D MOVE, Z/X ROTATE, SPACE DROP). Game over: band on field + "RETURN AGAIN" at bottom; RETURN → StartHelp.
- HUD at 2.6 ship: SCORE / LINES / LEVEL only. **0.1.0** adds next-piece preview ([`phase-4-v1-candidate-gate.md`](notes/phase-4-v1-candidate-gate.md)). Core: `game_start(..., start_level)`, scoring `initial_level`.
- Gravity: dedicated counter, PAL-friendly table (classic console pacing reference). Input: A/D repeat when held (keyboard matrix).

## Phase 3 — Feel and performance

**Status:** completed

- **Input:** Quattro-owned repeat (not KERNAL); DAS / soft-drop / rotation semantics — [`input-baseline.md`](notes/input-baseline.md).
- **Joystick:** port 2, minimal mapping; validated alongside keyboard.
- **Performance:** fewer redundant video writes; steady-state sampling via `make c64_perf` — [`performance-phase3-closure.md`](notes/performance-phase3-closure.md). Lower-level rewrites only when measured and localized.

## Phase 4 — Finish

**Status:** **completed.** RC + interactive smoke: [`phase-4-rc-closure.md`](notes/phase-4-rc-closure.md). Release text: [`release-0.1.0.md`](notes/release-0.1.0.md).

- v1 boundary fixed (must-have / optional / out-of-v1).
- Optional candidates settled — [`phase-4-v1-candidate-gate.md`](notes/phase-4-v1-candidate-gate.md): micro SFX + next-piece preview **in**; micro polish **out** for v1.
- Default SID SFX; `QUATTRO_AUDIO=0` silent build.
- Docs + `build/quattro.prg` build path verified.
