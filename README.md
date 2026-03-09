# Quattro

A sober, historically-minded falling-blocks game for Commodore 64, built with contemporary software engineering discipline.

## Status

**Playable pre-alpha.**

The host-side core is tested, and the first C64 build is already working and playable. Quattro currently includes a minimal presentation layer with a visible playfield frame, a small HUD (score, lines, level), and an explicit game-over state.

Visual identity, title flow, restart flow, custom charset, and further feel refinement are still in progress.

## Why

Quattro explores a simple question: what happens when you develop for Commodore 64 as if it were a contemporary platform with extreme constraints — without betraying its identity?

The answer is not a feature-rich reinterpretation, but a rigorous, minimal, highly playable game built with clear architecture, reproducible builds, careful optimization and obsessive attention to feel.

## Current features

- Tested host-side core (deterministic, test-backed)
- Playable C64 build (llvm-mos, runs in VICE)
- Minimal board frame and score / lines / level HUD
- Explicit game-over state
- Same game logic on host and C64 ([host-core contract](docs/host-core-contract.md))

## Building

- **Host / tests:** `make host_debug`, `make test` (standard `cc`).
- **C64:** `make c64` (requires [llvm-mos](https://github.com/llvm-mos/llvm-mos) with `mos-c64-clang`). Run: `make c64_run` (VICE) or load `build/quattro.prg` in your emulator.
- **clangd:** `make compdb` (host) or `make compdb-all` (host + C64). Requires [Bear](https://github.com/rizsotto/Bear); generated files are not committed.

## Controls (C64)

A / D = left / right · Z / X = rotate CCW / CW · SPACE = soft drop

## Design constraints

- **Toolchain:** llvm-mos; assembly only for isolated critical modules when profiling justifies it.
- **Rendering:** character-based board (custom charset later).
- **Core:** C only; simple deterministic PRNG; no 7-bag; CW/CCW rotation, no wall kicks by default.
- **Scope:** no hold, ghost, multiplayer, or feature creep. See [docs/scope.md](docs/scope.md).

## Repository

| Path      | Purpose                |
|-----------|------------------------|
| `docs/`   | Design, spec, roadmap  |
| `src/`    | Core, platform C64, render |
| `tests/` | Core tests             |
| `tools/` | Host debug harness     |
| `assets/` | Placeholder for future |

Documentation is the source of truth for decisions and semantics. Start with [docs/host-core-contract.md](docs/host-core-contract.md) and [docs/roadmap.md](docs/roadmap.md).
