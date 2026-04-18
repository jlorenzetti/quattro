# Custom charset — technical investigation

**Status:** active micro-investigation (post-0.1.0). Custom charset is **not** part of the shipping game until a minimal standalone probe proves stable VIC + memory behaviour.

**Rule:** Isolate all charset experiments from the main build. No reintegration into `src/platform/c64/` until the success criteria below are met.

## Guiding question

> Can we produce a **minimal, repeatable** llvm-mos PRG that shows a **custom charset** correctly (no video corruption) using this toolchain and runtime?

Until the answer is clearly **yes**, do not treat “port this into Quattro” as in scope.

## Workspace

| Item | Choice |
|------|--------|
| Branch | Local `investigate/custom-charset` (or equivalent); not required on `main` until conclusions are ready |
| Probe location | `tools/c64/charset_probe/` — autonomous, small, disposable |
| Build | `make c64_charset_probe` (optional: `make c64_charset_probe_run` with VICE `x64sc`) |
| Main product | **Unchanged** — `make c64` does not depend on the probe |

## Phase A — minimal probe (current)

**Goal:** Start, configure VIC for 40×25 text @ `$0400`, install test glyphs at `$3000–$37FF`, set `$D018`, display screen codes `0–3` in the top-left, halt.

**Explicitly out of scope:** game loop, input, audio, HUD, board, gameplay timing.

**Implementation notes:**

- Linker script `tools/c64/charset_probe/charset_probe.ld` limits the load segment to `$0801–$2FFF` so sections cannot overlap the `$3000–$37FF` charset RAM used by VIC.
- Charset data is **not** embedded as a full 2K ROM image in the binary for Phase A; the program clears `$3000–$37FF` and writes four 8×8 patterns only (sufficient for codes `0–3`).

## Phase B — fix the memory protocol

One candidate charset base, one load path, one VIC sequence per experiment. Avoid ad-hoc address hopping; change **one** variable at a time.

## Phase C — minimal test matrix

| Axis | Variants |
|------|----------|
| Data source | Embedded vs other (e.g. ROM copy) — add only after Phase A is stable |
| Charset region | Primary: `$3000–$37FF`; optional second candidate only with a clear hypothesis |
| VIC | Baseline aligned with `src/platform/c64/video.c`, then single-register deltas |

## Success criteria

- Build completes reliably.
- Custom glyphs appear as intended (no checkerboard / wrong video mode artefacts).
- No corruption; result is repeatable in the chosen emulator(s).
- Setup is explainable in a short, ordered checklist.

## Useful failure criteria

A failed run still advances the investigation if it narrows blame, e.g. “only fails when `$D018` is switched” vs “fails even with ROM charset after the same linker map” vs “region `$3000` overlaps linked sections without the probe linker script”.

---

## Historical attempts (pre-0.1.0)

**Context:** Explored for presentation; **ROM + PETSCII** remained the 0.1.0 baseline because VIC/charset integration with the full game layout was not stable. Conclusions below are frozen; the **probe + protocol above** supersede ad-hoc edits in the main tree.

| # | Approach | Result |
|---|----------|--------|
| 1 | Copy Character ROM (`$D000`) to `$2000`, set `$D018` | Severe screen corruption. `$2000` likely overlaps llvm-mos code/data. |
| 2 | Move charset to `$3800`, `$01 = $34` before copy | Lowercase chars, freeze. `$34` (CHAREN=1) maps I/O, not ROM; wrong data copied. |
| 3 | `$01 = $32` to map Character ROM | Lowercase, freeze. Copy looked correct in tooling; crash before `$D018` write; `$01` restore risk. |
| 4 | Embedded charset, copy to `$3000`, no `$01` manipulation | No freeze; game runs; screen corrupt when enabling custom charset via `$D018`. |
| 5 | Explicit VIC init (`$DD00`, `$D011`, `$D016`, `$D018`) | Same corruption. |
| 6 | ROM + PETSCII `0xA0` for block/frame | Correct display, stable. **0.1.0 baseline.** |

**Reference:** [release-0.1.0.md](release-0.1.0.md), [presentation-identity-2.5.md](presentation-identity-2.5.md).
