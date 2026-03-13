# Roadmap

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
- Custom charset: isolated investigation; see `docs/notes/custom-charset-investigation.md`.

## Phase 2.6 — Interface composition

**Status:** completed

- State flow: Title → StartHelp → Game → GameOver (with replay) → StartHelp. Boot starts at Title.
- Title: block-built QUATTRO wordmark, one prompt line (PRESS ANY KEY). Start/help: start level 0–9 (keys 0–9), RETURN to start; controls listed (A/D MOVE, Z/X ROTATE, SPACE DROP). Game over: band on field + "RETURN AGAIN" at bottom; RETURN → StartHelp.
- In-game UI unchanged: SCORE / LINES / LEVEL only (no next-piece in 2.6). Core: minimal `game_start(..., start_level)` and scoring `initial_level` for level progression.
- Gravity: dedicated counter, PAL-friendly table (classic console pacing reference). Input: A/D repeat when held (keyboard matrix).

## Phase 3 — Feel and performance
- tune responsiveness
- reduce unnecessary writes
- profile hotspots
- move critical paths lower-level only if justified

## Phase 4 — Finish
- retain final v1 feature set
- final audio pass
- documentation pass
- release packaging
