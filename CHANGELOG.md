# Changelog

All notable changes to Quattro are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Quattro has no version number yet; the first tagged release will be `0.1.0`.

## [Unreleased]

### Added
- Host-side core: board, piece, rules, scoring, RNG, and game state
- Deterministic host-side tests and a host debug harness
- First C64 integration: entry point, video, input, timing, and llvm-mos build support
- Runnable C64 PRG validated in VICE
- Minimal presentation pass: playfield frame, HUD (`SCORE`, `LINES`, `LEVEL`), and explicit `GAME OVER` state
- Documentation for project vision, scope, architecture, core contract, feel baseline, core spec, tooling, and roadmap
- `compile_commands` workflow for host, C64, and merged clangd databases
- **Phase 2.6 — Interface composition:** Title screen (block wordmark, PRESS ANY KEY), start/help screen (start level 0–9 via keys, RETURN to start; default level 5), game-over + replay prompt ("RETURN AGAIN" → StartHelp). Level-based gravity (dedicated counter, PAL 50 Hz table). A/D repeat when held (keyboard matrix). Core: `game_start(state, seed, start_level)` and scoring `initial_level`.
