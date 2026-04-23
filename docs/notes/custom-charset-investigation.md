# Custom charset — technical investigation

**Status:** **Substantially closed** (post-0.1.0 through B3). The technical question is answered: **bank-2 staging + scoped custom charset** is a **credible, playable** integration; the historical failure was **memory overlap** at `$3000–$37FF`, not intrinsic VIC fragility. **Default shipping build** remains **`make c64`** (ROM + PETSCII). **`make c64_vic_bank2`** is the in-tree **validated** custom-charset variant. **Only remaining open item:** product choice (optional toggle, default switch, or stay ROM-only) — not further technical investigation unless that choice or the layout changes.

**Rule (for any future charset work):** Keep experiments isolated from the shipping configuration until gated; optional linker experiments use `make c64 C64_LD=…` (see Makefile).

## Investigation closure

| Delivered | Notes |
|-----------|--------|
| Root cause | **Overlap** of runtime charset buffer with **linked** program at `$3000–$37FF` (B2.2). |
| Architecture | **Path B, bank 2:** screen **`$8000`**, charset **`$8800`**, same PRG — **no** linker carve-out required for this approach. |
| Readable game | **B3** working charset + maintainer gameplay pass (title, help, HUD, well, game over, replay, preview). |
| Deprioritised | **Linker carve-out** (B2.3b) and **parallel bank variants** — not needed at current state. |
| Retained in tree | Charset **probe** targets (`make c64_charset_probe*`) and **bank-2** build — useful for **regression**, not day-to-day investigation. |

No further **investigative** code passes are assumed from this document; follow-up is **product / integration policy**, not “prove feasibility again.”

## What we know now

Phase A showed that **`$D018` + a custom charset can work** under llvm-mos when **`$3000–$37FF` is not used by linked sections** (probe linker script caps the load segment at `$2FFF`). The historical corruption in the **full game** is therefore **not** “VIC + custom charset in principle”, but **integration with the real runtime / linker layout**.

Phase B1 showed that a **full 2K charset image in `.rodata`** does **not** by itself break the probe: the failure mode is **not** “large embedded payload”.

**B2.2 (map comparison — closed):** `build/quattro.map` from `make c64` shows **`.text` (and the rest of the linked image) occupying addresses that include `$3000–$37FF`**. Copying a 2K charset into that range at runtime therefore **overwrites loaded code or data** — a concrete, checkable explanation for the historical “mystery VIC” symptoms. The primary hypothesis is now **memory overlap**, not unexplained `$D018` behaviour.

**B2.3c (bank-2 stage — closed):** Full PRG with **`QUATTRO_VIC_BANK2_CHARSET`** validated in VICE: bank 2 + `$8000`/`$8800` path OK; forced screen codes `0–3` show probe glyphs; blind play OK. Flat field with normal PETSCII is expected with a **four-glyph-only** charset (see B2.3c outcome).

**B3 (readable bank-2 charset — done):** Working **2K** payload + same copy/install as B2.3c; see **B3** subsection below. **Consequence:** the next product-level question is **whether** custom charset becomes a supported or default build, not whether bank-2 staging can work.

## Guiding questions

**Closed (Phase A):**

> Can we produce a **minimal, repeatable** llvm-mos PRG that shows a **custom charset** correctly (no video corruption)?

**Closed (Phase B1):**

> Does a **full 2K embedded** charset break the minimal probe?

**Closed (Phase B2.2):**

> Does the **real** Quattro PRG leave `$3000–$37FF` unused?

**No** — the map shows content in that range; the charset buffer overlaps the executable image.

**Closed (layout / reintegration — B2.3a + B2.3c):**

> Is there a **non-overlapping** screen + 2K charset window in the **real** PRG without linker carve-out?

**Yes** — **bank 2** (`$8000` / `$8800`) under **`QUATTRO_VIC_BANK2_CHARSET`**: stable VIC path and normal game loop (see B2.3c outcome). Historical symptoms match **overlap** (B2.2), not “VIC is inherently fragile.”

**Closed (B3 — readable in-game pass):**

> Can the bank-2 build show **legible** HUD, help, title line, game-over text, digits, and board/frame using a **scoped** custom charset without new linker or bank variants?

**Yes** — see **B3** subsection (generator + sober 8×8 ASCII-aligned set; reverse band via inverted indices `+ 0x80`). Builds: `make c64_vic_bank2`. **CI:** `make test`, `make c64`, `make c64_vic_bank2` succeed on the agent host. **Gameplay (emulator):** maintainer confirmed title line, start/help, HUD, playfield, game over, replay line, and next-piece preview all behave as expected.

**Open (product):**

> Should **custom charset on bank 2** become a **supported option** or **default** vs ROM/PETSCII — and on what release criteria?

**Fork (decide before linker surgery):**

| Path | Idea | When it wins |
|------|------|----------------|
| **A — Linker carve-out** | Reserve `$3000–$37FF` (or split sections around it) | **Not required** for the adopted bank-2 path — **fallback only** if bank-2 approach is ever abandoned |
| **B — Move VIC bank + bases** | Keep one contiguous PRG; point VIC at another 16K window with free RAM | **Validated (bank 2 in-game)** — **default trajectory** for readable custom charset (B3) |

## Workspace

| Item | Choice |
|------|--------|
| Branch | Any; investigation **closed** — merge policy is a normal release decision |
| Probe location | `tools/c64/charset_probe/` — **archival / regression** (optional); not required for the bank-2 solution |
| Probe build | `make c64_charset_probe` (B1 linker); `make c64_charset_probe_b21` (B2.1 linker) |
| Main PRG | Default `make c64`; `make c64_vic_bank2` / `make c64_vic_bank2_run` — bank-2 + B3 charset (`bank2_working_charset_data.h`, regen: `tools/c64/gen_bank2_working_charset.py`); optional `make c64 C64_LD=path/to/script.ld` for linker experiments |

## Phase A — minimal probe (done)

**Goal:** Start, configure VIC for 40×25 text @ `$0400`, install test data at `$3000–$37FF`, set `$D018`, display screen codes `0–3` in the top-left, halt.

**Implementation:** `tools/c64/charset_probe/charset_probe.ld` limits the load segment to `$0801–$2FFF` so sections cannot overlap `$3000–$37FF`.

**Outcome:** Stable display in emulator; confirms toolchain + VIC path for a protected map.

## Phase B — memory / layout protocol

### B1 — Full 2K embedded charset (done)

Same minimal probe, B1 linker (`charset_probe.ld`). **Outcome:** Same stable display; large `.rodata` is **not** the culprit by itself.

### B2.1 — Default C64 `MEMORY`, charset region forbidden (done)

**Files:** `tools/c64/charset_probe/charset_probe_b21.ld` — same `MEMORY` shape as the default C64 PRG, `ASSERT(__heap_start <= 0x3000)`.

**Outcome:** For the current probe, same behaviour as B1; confirms the probe stays valid under the production memory model **while** the binary stays below `$3000`.

### B2.2 — Compare maps (closed)

Compare `build/charset_probe_b21.map` with **`build/quattro.map`**. **Finding:** the real game’s linked image **uses** `$3000–$37FF`; a runtime copy into that range **collides** with the loaded program.

### B2.3a — VIC-visible free-window inventory (done)

**Deliverable:** Pick a **screen base + charset base** pair (2K-aligned charset, 1K-step screen within the same VIC 16K bank) that does **not** collide with the **linked** program, before writing a heavy `quattro_*.ld`.

**Map snapshot (representative `build/quattro.map`):** contiguous linked VMAs cover roughly **`$0801`–`$44E9`** — `.text` through **`$4173`**, `.rodata` **`$4173`–`$4358`**, `.bss` **`$4358`–`$44E9`**, then **`__heap_start` `$44E9`**. The program therefore occupies **`$3000`–`$37FF`** (among others). Default VIC setup in `video.c` uses **CIA2 `$DD00` bits 0–1 = `%11`** → VIC bank **0** (`$0000`–`$3FFF`). Color RAM stays **`$D800`** regardless of VIC bank.

| Candidate | VIC bank (CPU 16K) | Example screen base | Example 2K charset base | Overlap linked image (`$0801`–`$44E9` + heap growth) | Integration cost (rank) |
|-----------|-------------------|---------------------|---------------------------|------------------------------------------------------|---------------------------|
| **0 (current)** | `$0000`–`$3FFF` | `$0400` (shipping) | e.g. `$3000` | **Yes** — `.text` in `$3000`–`$37FF` | **High** if charset must live here (carve-out / path A) |
| **1** | `$4000`–`$7FFF` | e.g. `$4000` | e.g. `$6000`–`$67FF` | **Lower** — linked use ends ~`$44E9`; **`$6000`+** is outside current VMAs; **watch** heap growing from `$44E9` toward `$6000` | **Medium** — set `$DD00` for bank 1, re-point screen in `memory_map` / `video.c`, recompute `$D018` |
| **2** | `$8000`–`$BFFF` | e.g. `$8000` | e.g. `$8800`–`$8FFF` | **None** — no program sections in this window on standard PRG | **Medium** — same as bank 1 + move “stage” to an empty 16K island; update all screen addressing |
| **3** | `$C000`–`$FFFF` | — | — | **Avoid for v1** — RAM/ROM/`$D000` I/O overlap; harder to reason about | **High** |

**Reading:** Path **B** was validated in the real PRG; **bank 2** was chosen (cleanest no-overlap island for a standard PRG). **Bank 1** remains a theoretical lighter move of the matrix; **not** pursued after bank-2 success.

### B2.3c — In-game bank 2 “stage” experiment (done)

**Goal:** Same real Quattro PRG (no linker surgery): **VIC bank 2** (`$8000`–`$BFFF`), **screen matrix** at **`$8000`**, **2K charset** copied to **`$8800`–`$8FFF`**, **`$D018` = `0x02`** (indices relative to bank base). **B2.3c milestone** used a **four-glyph probe** in RAM to prove **stability** only (no overlap corruption). **This branch:** with **`QUATTRO_VIC_BANK2_CHARSET`**, `video.c` installs the **B3** readable charset (`bank2_working_charset_data.h`), not the probe payload.

**Build:** `make c64_vic_bank2` (optional `make c64_vic_bank2_run` with VICE `x64sc`). Implemented with **`QUATTRO_VIC_BANK2_CHARSET`** in `video.c` / `memory_map.h`.

**Outcome:** VICE: **`$DD00`** bank **2**; **`$8000`** / **`$8800`** match the experiment. Forced screen codes **`0`–`3`** at **`$8000`**: four probe patterns visible; **blind play** (input + audio) normal. A flat light field from **`$d021`** only: **expected** when common PETSCII maps to **undefined (zero) cells** in the four-glyph charset — not a wrong matrix base (see **`$D018`** readback note below).

**VICE / `$D018` readback:** On VIC-II, **`peek` / `m $d018`** often does **not** match the last **write** (e.g. **`$03`** after writing **`$02`**) because unused bits (notably **bit 0**) are not stored or read as written. Trust the **programmed** value for the effective screen/charset layout; use **border/background pokes** (`$d020` / `$d021`) or the **VIC register view** in the debugger if the monitor’s memory dump is ambiguous.

**Sanity check (VICE 3.9 monitor):** `> d020 0e`, `> d021 06`, then **`g`** — border/background update as expected, so the session is **not** “stuck” with dead I/O; a **black** picture with unchanged defaults is then plausibly **ink vs background** in **`$d800`** (and/or charset mismatch), not a generic emulator hang.

### B3 — Readable in-game custom charset pass (done)

**Intent:** Same integration as B2.3c; replace probe payload with a **readable** working 2K charset — **not** shipping default, **not** full ROM parity.

**Implementation:** `src/platform/c64/bank2_working_charset_data.h` (**generated** — do not hand-edit). Source: `tools/c64/gen_bank2_working_charset.py`. `video.c` includes the header under **`QUATTRO_VIC_BANK2_CHARSET`** only; **`make c64`** unchanged.

**Glyph scope (0–127):** Space, **`/`**, **`-`**, digits **`0`–`9`**, uppercase **`A`–`Z`** as sober 8×8 forms (MSB = left pixel). Other codes **blank** (zeros).

**Indices 128–255:** Bitwise inverse of 0–127 so **`put_string_reverse_at`** (`char + 0x80`) matches C64-style reverse video; **`0xA0`** = inverted space = **solid block** (board, frame, GAME OVER band).

**Intentionally unsupported in this pass:** Lowercase, symbols outside the small set, full PETSCII, ROM-identical shapes, polish pass.

**Outcome:** Bank-2 path **unchanged** technically; charset swap only. **Stable** at build level (`make c64_vic_bank2`). **Readable** for current `video.c` strings and ASCII HUD/title/replay/game-over usage; title **QUATTRO** wordmark remains **bitmap blocks** (unchanged). **Visually:** minimal geometric caps — coherent with a severe, non-decorative direction; not final art. **In-emulator gameplay pass:** title prompt, help, HUD, well + frame, game-over band + text, replay line, preview — all confirmed OK (maintainer).

**Shipping discussion:** B3 is **strong enough** to justify a later **product** decision (toggle, default, or stay ROM-only); it does **not** by itself mandate shipping custom charset.

### B2.3b — Linker carve-out experiment (deferred)

**When:** Only if **B2.3a** does not yield an acceptable screen+charset pair (or validation fails in-game). **Note:** After B2.3c, treat as **fallback**, not the default next step.

**Intent:** Custom `C64_LD` script — split sections / reserve range — see technical note below.

**Technical note (llvm-mos PRG):** Default output is a **single** contiguous load from `$0801` (`TRIM(ram)`). A **middle** hole usually implies **section splitting** (`-ffunction-sections`, named output sections) or a different charset **base** — not a one-line tweak.

**Makefile:** `make c64 C64_LD=tools/c64/your_script.ld`

## Phase C — optional follow-ups (not part of closure)

**Not required** to consider the investigation complete. Useful only if product asks for extra variants later.

| Axis | Variants |
|------|----------|
| Data source | Full embedded vs ROM copy |
| Charset region | `$3000–$37FF` vs another 2K-aligned slot in the active VIC 16K bank |
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
