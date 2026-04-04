# Quattro 0.1.0

**Status:** public release reference (tag **v0.1.0**).

**First public release** of Quattro for Commodore 64: a sober, historically-minded falling-blocks game, built with contemporary engineering discipline while staying faithful to the C64 as a platform. The same game logic runs in a minimal host-side harness for tests and debugging.

**RC record:** [`phase-4-rc-closure.md`](phase-4-rc-closure.md). **Change history:** [`CHANGELOG.md`](../../CHANGELOG.md).

## Included in 0.1.0

- Full C64 loop: title → start/help (levels 0–9) → game → game over → replay to start/help
- ROM/PETSCII presentation baseline (frame, HUD: SCORE / LINES / LEVEL, field-first layout)
- Keyboard and **joystick port 2** (see [`README.md`](../../README.md) **Controls**)
- **Next-piece preview** in the left support column
- **SID micro SFX** (six one-shots); compile-time silent build via `QUATTRO_AUDIO=0`
- Shared **host-side core** with deterministic **tests** (`make test`) and optional host debug harness
- Reproducible **C64 build**: `make c64` → `build/quattro.prg` (llvm-mos); documented in [`README.md`](../../README.md) **Building**

## Explicitly not in 0.1.0

- Custom charset integration
- New gameplay mechanics (hold, ghost piece, 7-bag, wall kicks, multiplayer, etc.)
- Richer UI, extra stats panels, or expanded menus
- Broad performance campaigns or architecture refactors beyond the current baseline

See [`scope.md`](../scope.md) and [`phase-4-v1-candidate-gate.md`](phase-4-v1-candidate-gate.md) for boundary context.

## Known non-blocking limitations

- Occasional faint SID clicks or weak attack on some hardware/emulators (see [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md)).
- Experimental browser demo (GitHub Pages) is not the primary distribution path for this release.

## Build / artifact

- **Release artifact:** `build/quattro.prg` (not committed; produced by `make c64` from repo root).
- **Silent build:** `rm -f build/quattro.prg && make c64 C64_DEFS="-DQUATTRO_AUDIO=0"` — same game, no SID driver code in the binary path for SFX.
