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
