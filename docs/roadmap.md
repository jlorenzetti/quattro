# Roadmap

## Phase 0 — Pre-production
- define scope
- choose toolchain
- choose visual representation strategy
- choose input model
- decide minimal audio direction

**Status:** completed

## Phase 1 — Core prototype
- board model
- piece model
- movement and rotation
- collision and locking
- line clear
- score
- host-side tests and debug harness

**Status:** completed

## Phase 2 — First playable C64 build
- C64 entry point
- rendering path
- input polling
- timing loop
- minimal presentation layer
- repeated playtests

**Status:** completed

## Phase 2.5 — Presentation identity

**Status:** completed (ROM/PETSCII only; custom charset deferred.)

- Palette: black background, border = background ($D020 = $D021), frame 15, locked 1, active 3, HUD 15.
- Layout: board centred (BOARD_SCREEN_X = 15), HUD left (HUD_SCREEN_COL = 1).
- Game over: 3-row light grey band inside board, "GAME  OVER" reverse (message on field).
- Custom charset: isolated investigation; see `docs/notes/custom-charset-investigation.md`.

## Phase 2.6 — Interface composition
- decide the long-term information layout around the playfield
- evaluate whether persistent help text belongs on the main game screen
- decide whether next-piece preview should become part of the visible interface
- title screen and restart / replay flow (deferred from Phase 2)
- refine game-over composition as an explicit interface state
- keep interface density low and avoid decorative or statistics-heavy UI

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
