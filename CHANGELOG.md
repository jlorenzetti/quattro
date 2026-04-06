# Changelog

All notable changes to Quattro are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added
- **16 KB ROM-native C64 cartridge** (`make c64_cart` → `build/quattro-cart-8000.rom`, `make c64_crt` → `build/quattro-cart.crt`, optional `make c64_crt_run`). CBM80 autostart, KERNAL preflight before `crt0`, cart-specific `__after_main`; **`.prg`** remains the default target. See [`docs/notes/c64-cartridge.md`](docs/notes/c64-cartridge.md).

## [0.1.0] - 2026-04-04

First public release. Summary: [`release-0.1.0.md`](docs/notes/release-0.1.0.md).

### Added
- Full C64 loop: title, start/help, gameplay, game over, and replay to start/help.
- ROM/PETSCII presentation baseline with frame, HUD (`SCORE`, `LINES`, `LEVEL`), and field-first layout.
- Next-piece preview in the left support column.
- Minimal SID micro SFX, with supported silent build via `QUATTRO_AUDIO=0`.
- Keyboard controls and joystick support (port 2).
- Level-based gravity and start level selection (0–9).
- Shared host-side core with deterministic tests and host debug harness.
- Reproducible llvm-mos C64 build producing `build/quattro.prg`.
- Documentation covering scope, architecture, host/core contract, core spec, tooling, roadmap, and release notes.
- `compile_commands` workflow for host and C64 development.
