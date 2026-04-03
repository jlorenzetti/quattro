# Quattro

A sober, historically-minded falling-blocks game for Commodore 64, built with contemporary software engineering discipline.

<p align="center">
  <img src="docs/quattro-demo.gif" alt="Quattro gameplay demo" width="800">
</p>

**Experimental browser demo:** [Play Quattro](https://jlorenzetti.github.io/quattro/)

## Status

**Toward v1.0** ([`roadmap.md`](docs/roadmap.md): Phase 4 — Finish).

Host-side core is tested; the C64 build covers the full loop (title, start/help with levels 0–9, play, game over, replay to start/help).

v1 means a stable baseline: ROM/PETSCII presentation, current controls and UI, release packaging, and documentation closure. Audio: minimal micro SFX ([`phase-4-micro-sfx.md`](docs/notes/phase-4-micro-sfx.md)); `QUATTRO_AUDIO=0` remains a fallback build option (see **Building**).

## Why

Quattro explores a simple question: what happens when you develop for Commodore 64 as if it were a contemporary platform with extreme constraints — without betraying its identity?

The answer is not a feature-rich reinterpretation, but a rigorous, minimal, highly playable game built with clear architecture, reproducible builds, careful optimization and obsessive attention to feel.

## Host-side harness

The host-side harness is intentionally austere: testable, deterministic, and slightly evocative of the Elektronika 60 lineage.

```text
quattro
seed 12345

LINES: 0000
LEVEL: 00
SCORE: 0000

<! . . . .[] . . . . .!>
<! . . . .[][][] . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<! . . . . . . . . . .!>
<!====================!>

A/D MOVE  Z/X ROTATE  SPC DROP  G TICK  Q QUIT
```

## Current features

- Tested host-side core (deterministic, test-backed)
- Playable C64 build (llvm-mos, runs in VICE). C64 gameplay uses a pseudorandom seed derived at game start; deterministic seeds remain available for host tests and optional debug builds.
- Title screen (block wordmark, PRESS ANY KEY) and start/help (level 0–9, RETURN start)
- Gameplay: board frame, HUD (SCORE / LINES / LEVEL), **next-piece preview**, level-based gravity
- Game over on field + replay prompt (RETURN AGAIN → start/help)
- C64 SID micro SFX (six one-shots; disable with `QUATTRO_AUDIO=0` — see **Building** and [`phase-4-micro-sfx.md`](docs/notes/phase-4-micro-sfx.md))
- Same game logic on host and C64 ([`host-core-contract.md`](docs/host-core-contract.md))

## Building

- **Host / tests:** `make host_debug`, `make test` (standard `cc`).
- **C64:** `make c64` (requires [llvm-mos](https://github.com/llvm-mos/llvm-mos) with `mos-c64-clang`). Run: `make c64_run` (VICE) or load `build/quattro.prg` in your emulator. For a reproducible debug build: `make c64_fixed_seed` or `make c64 C64_DEFS="-DQUATTRO_FIXED_SEED=12345"`. To build without SID SFX: `rm -f build/quattro.prg && make c64 C64_DEFS="-DQUATTRO_AUDIO=0"`.
- **clangd:** `make compdb` (host) or `make compdb-all` (host + C64). Requires [Bear](https://github.com/rizsotto/Bear); generated files are not committed.
- **Browser demo (experimental):** Workflow builds and deploys to GitHub Pages. See [`README.md`](web-src/README.md).

## Controls (C64)

**Gameplay:** A / D = left / right (repeat when held) · Z / X = rotate CCW / CW · SPACE = soft drop  
**Setup:** 0–9 = start level · RETURN = start game / again after game over

## Design constraints

- **Toolchain:** llvm-mos; assembly only for isolated critical modules when profiling justifies it.
- **Rendering:** character-based board (custom charset later).
- **Core:** C only; simple deterministic PRNG; no 7-bag; CW/CCW rotation, no wall kicks by default.
- **Scope:** no hold, ghost, multiplayer, or feature creep. See [`scope.md`](docs/scope.md).

## Repository

| Path      | Purpose                |
|-----------|------------------------|
| `docs/`   | Design, spec, roadmap  |
| `src/`    | Core, platform C64, render |
| `tests/` | Core tests             |
| `tools/` | Host debug harness     |
| `web-src/` | Browser demo source (templates; Pages deployed via CI) |
| `assets/` | Placeholder for future |

Documentation is the source of truth for decisions and semantics. Start with [`host-core-contract.md`](docs/host-core-contract.md) and [`roadmap.md`](docs/roadmap.md).
