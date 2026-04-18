# Custom charset — technical investigation

**Status:** active micro-investigation (post-0.1.0). Custom charset is **not** part of the shipping game until a minimal standalone probe proves stable VIC + memory behaviour.

**Rule:** Isolate all charset experiments from the main build. No reintegration into `src/platform/c64/` until the success criteria below are met.

## What we know now

Phase A showed that **`$D018` + a custom charset can work** under llvm-mos when **`$3000–$37FF` is not used by linked sections** (probe linker script caps the load segment at `$2FFF`). The historical corruption in the **full game** is therefore **not** “VIC + custom charset in principle”, but **integration with the real runtime / linker layout**.

Phase B1 showed that a **full 2K charset image in `.rodata`** does **not** by itself break the probe: the failure mode is **not** “large embedded payload”, narrowing suspicion to **real layout**, **init order**, or **both**.

That narrows the next work to **memory and layout protocol**, not blind VIC register tuning.

## Guiding questions

**Closed (Phase A):**

> Can we produce a **minimal, repeatable** llvm-mos PRG that shows a **custom charset** correctly (no video corruption)?

**Closed (Phase B1):**

> Does a **full 2K embedded** charset break the minimal probe?

**Open (Phase B2):**

> If **`$3000–$37FF` stays free**, does the charset stay stable as **linker memory model and init** move toward the real game?

If **yes**, reintegration is plausible with a controlled map. If **no**, suspect **startup / init ordering** (ZP, data, VIC/CIA/6510) rather than (or in addition to) raw addresses.

Until Phase B2 is answered, do not treat full in-game integration as in scope.

## Workspace

| Item | Choice |
|------|--------|
| Branch | Local `investigate/custom-charset` (or equivalent); not required on `main` until conclusions are ready |
| Probe location | `tools/c64/charset_probe/` — autonomous, small, disposable |
| Build | `make c64_charset_probe` / `make c64_charset_probe_run` — B1 linker; `make c64_charset_probe_b21` / `make c64_charset_probe_b21_run` — B2.1 linker |
| Main product | **Unchanged** — `make c64` does not depend on the probe |

## Phase A — minimal probe (done)

**Goal:** Start, configure VIC for 40×25 text @ `$0400`, install test data at `$3000–$37FF`, set `$D018`, display screen codes `0–3` in the top-left, halt.

**Implementation:** `tools/c64/charset_probe/charset_probe.ld` limits the load segment to `$0801–$2FFF` so sections cannot overlap `$3000–$37FF`.

**Outcome:** Stable display in emulator; confirms toolchain + VIC path for a protected map.

## Phase B — memory / layout protocol (in progress)

One candidate charset base, one load path, one VIC sequence per experiment. Change **one** variable at a time.

### B1 — Full 2K embedded charset (done)

Same minimal probe, same `$3000–$37FF` target, B1 linker (`charset_probe.ld`).

**Change:** Embed a **full 2048-byte** charset image in `.rodata` (first four characters = test patterns; remaining bytes zero), **copy** into `$3000–$37FF`, then `$D018` + halt.

**Outcome:** Same stable display; large `.rodata` is **not** the culprit by itself.

### B2 — Layout and init vs Quattro (current)

#### B2.1 — Default C64 `MEMORY`, charset region forbidden (current)

**Goal:** Use the **same `MEMORY` line** as the default C64 PRG (`ram` @ `$0801`, `LENGTH 0xC7FF`), but **forbid** placing sections in `$3000–$37FF` (assert `__heap_start <= 0x3000` for contiguous layout).

**Why:** Same **linker script shape** as production (`mos-platform/c64/lib/link.ld`) while keeping the charset window empty. For the current probe binary, VMAs match B1; the **map file** documents the full `ram` span.

**Files:** `tools/c64/charset_probe/charset_probe_b21.ld`
**Build:** `make c64_charset_probe_b21` → `build/charset_probe_b21.prg`, `build/charset_probe_b21.map`

**Limitation:** With contiguous sections, the assert is equivalent to “everything below `$3000`”. Splitting sections across the hole (code low, bss high) **requires** a future linker script; link will fail if the binary grows past `$3000` until then.

#### B2.2 — Compare maps (next)

Without running the full game: compare `build/charset_probe_b21.map` (and B1’s `build/charset_probe.map`) with **`build/quattro.map`** from `make c64`. Inspect which sections approach `$3000` in the real game and how structural differences differ from the probe.

#### B2.3 — Quasi-runtime shim (later)

Inside the real C64 platform layer only: **no** board/gameplay — near-real init, install charset, few test glyphs, halt.

**Purpose:** Isolate **linker/layout** vs **init order** (startup, ZP/data, VIC/CIA/6510 sequencing).

**Do not skip B1 → B2.1 → B2.2 → B2.3** for a clean causal story.

## Phase C — minimal test matrix (future)

| Axis | Variants |
|------|----------|
| Data source | Full embedded vs ROM copy (only after B1 is stable) |
| Charset region | Primary: `$3000–$37FF`; second candidate only with a clear hypothesis |
| VIC | Baseline aligned with `src/platform/c64/video.c`, then single-register deltas |

## Success criteria

- Build completes reliably.
- Custom glyphs appear as intended (no checkerboard / wrong video mode artefacts).
- No corruption; result is repeatable in the chosen emulator(s).
- Setup is explainable in a short, ordered checklist.

## Useful failure criteria

A failed run still advances the investigation if it narrows blame, e.g. “only fails when `$D018` is switched” vs “fails when `.rodata` exceeds X” vs “region `$3000` overlaps linked sections without reservation”.

---

## Historical attempts (pre-0.1.0)

**Context:** Explored for presentation; **ROM + PETSCII** remained the 0.1.0 baseline because VIC/charset integration with the **full game** layout was not stable. The table is frozen; **Phase A/B above** supersede ad-hoc edits in the main tree.

| # | Approach | Result |
|---|----------|--------|
| 1 | Copy Character ROM (`$D000`) to `$2000`, set `$D018` | Severe screen corruption. `$2000` likely overlaps llvm-mos code/data. |
| 2 | Move charset to `$3800`, `$01 = $34` before copy | Lowercase chars, freeze. `$34` (CHAREN=1) maps I/O, not ROM; wrong data copied. |
| 3 | `$01 = $32` to map Character ROM | Lowercase, freeze. Copy looked correct in tooling; crash before `$D018` write; `$01` restore risk. |
| 4 | Embedded charset, copy to `$3000`, no `$01` manipulation | No freeze; game runs; screen corrupt when enabling custom charset via `$D018`. |
| 5 | Explicit VIC init (`$DD00`, `$D011`, `$D016`, `$D018`) | Same corruption. |
| 6 | ROM + PETSCII `0xA0` for block/frame | Correct display, stable. **0.1.0 baseline.** |

**Reference:** [release-0.1.0.md](release-0.1.0.md), [presentation-identity-2.5.md](presentation-identity-2.5.md).
