# Roadmap

## Phase 0 — Pre-production
- define scope
- choose toolchain
- choose visual representation strategy
- choose input model
- decide minimal audio direction

## Phase 1 — Core prototype
- board model
- piece model
- movement and rotation
- collision and locking
- line clear
- score

## Phase 2 — First playable C64 build
- C64 entry point
- rendering path
- input polling
- title/game-over loop
- repeated playtests

*Completed: entry, video, input, timing, board frame, HUD, game over. Title screen and restart deferred.*

## Phase 3 — Feel and performance
- tune responsiveness
- reduce unnecessary writes
- profile hotspots
- move critical paths lower-level only if justified

## Phase 4 — Finish
- high score / next piece retained
- final audio pass
- documentation pass
- release packaging
