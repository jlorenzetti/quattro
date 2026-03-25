# Phase 3 — Performance closure

## Scope

Measured **steady-state `APP_GAME`** work only (see `[src/platform/c64/main.c](../../src/platform/c64/main.c)`):


| Section | Code                                                    |
| ------- | ------------------------------------------------------- |
| I       | `input_poll()`                                          |
| A       | `game_apply_command()`                                  |
| B       | `video_draw_board()`                                    |
| H       | `video_draw_hud()`                                      |
| G       | `game_tick_gravity()` **only when a gravity tick runs** |


**Excluded from steady-state:** `video_draw_frame()` (drawn once when entering gameplay; not per-frame). Do not treat it as a recurring hotspot.

**Out of scope:** presentation/layout, input semantics, gameplay rules, broad refactors, assembly-by-default.

## Method

- **Build:** `make c64_perf` links `[src/platform/c64/perf.c](../../src/platform/c64/perf.c)` with `-DQUATTRO_PERF=1` and halts after `QUATTRO_PERF_FRAMES` (default **500**) steady gameplay frames.
- **Timer:** CIA2 Timer A free-running countdown (`$DD04`/`$DD05`); section cost = start − end (16-bit, timer counts down).
- **Output:** After 500 frames:
  - **Row 21** — PERF_APPLY discards only, by command bucket (2 hex each, capped `FF`): `an##` = `CMD_NONE`, `am##` = move L/R, `ar##` = rotate CW/CCW, `sd##` = soft drop. Sums should match the `A` bad count on row 22.
  - **Row 22** — per-section **bad sample count** and **max accepted delta** for `I A B H G`, packed as `Ibb#mmmm` … (8 chars × 5 = 40 columns, no spaces).
  - **Row 23** — **average cycles** per section: `Ixxxx Axxxx Bxxxx Hxxxx Gxxxx` (trailing `X` if **any** section had discards).
  - **Color RAM** uses **foreground white (1)** on the default black background.
  - **Row 24** — **CIA-only** calibration: `C1xxxx` = cycles for **200** XOR iterations; `C2xxxxxxxx` = **8 hex digits**, total cycles for **5000** iterations (25×200 chunks, summed in software so the 16-bit CIA timer can wrap between reads without corrupting the total). Expect **C2/C1 ≈ 25** (same inner loop; ratio checks the read path).
  - Section deltas use the **single-wrap** formula for a 16-bit countdown timer; samples above **~50k** cycles are dropped; PERF_APPLY uses `perf_section_end_apply(command)` so discards are bucketed on **row 21**.
  - **I, A, B, H:** average **per frame** (500 samples each).
  - **G:** average **per gravity tick** (fewer samples; count = number of ticks in the window).
- **Interpretation:** Compare section averages to the **PAL frame budget** (~**19.7k** CPU cycles per frame at 50 Hz on a PAL C64 — order of magnitude). The question is share of frame time, not which function is “largest” in isolation.

## Scenarios (manual)


| Scenario   | How                                                                           |
| ---------- | ----------------------------------------------------------------------------- |
| **Idle**   | Start game (`make c64_fixed_seed` or normal); minimal or no input until halt. |
| **Stress** | Same build; from game start, rapid lateral + soft drop + rotation until halt. |


Run each scenario once (or a few), note **row 21** (apply bad by command), **row 22** (bad/max per section), **row 23** (averages), and **row 24** (calibration). Optionally use a fixed seed for repeatability.

## Results


| Scenario | I (avg) | A (avg) | B (avg) | H (avg) | G (avg/tick) | Notes |
| -------- | ------- | ------- | ------- | ------- | ------------ | ----- |
| Idle     | 029E    | 007C    | 7DB2    | 012B    | 06AB         | `X`; row 21 `an02 am00 ar00 sd00`; C2/C1 ≈ 25× |
| Stress   | 0718    | 01E3    | 85FB    | 012E    | 0807         | `X`; row 21 `an02 am00 ar00 sd00`; C2/C1 ≈ 25× |

**Read:** **`A`** discards are **`CMD_NONE`-only** (`an`); treat as measurement noise, not command-path cost. **`B`** has **0** bad samples and dominates other sections → credible hotspot. **`X`** is not blocking for the conclusion below.

## Decision rule (lower-level rewrite)

Allowed **only if all** hold:

1. Hotspot is **real** (measured).
2. It **matters** relative to the **frame budget** (not only the largest of five).
3. Fix stays **localized**.
4. Does not destabilize settled architecture (input model, core rules, video contract).

Otherwise: **reject for v1** and record why.

## Outcome

- **Letter:** **B** (one dominant section vs the others in both scenarios).
- **Candidate:** **`video_draw_board()`** (section **`B`**): no discarded samples; averages largest vs I/A/H/G.
- **Follow-up:** Optional, draw-path only if priorities allow — not an assembly mandate; no input/core contract change implied by this pass.

## Build notes

- Switching between `make c64` and `make c64_perf`: `c64_perf` removes `build/quattro.prg` first so `C64_DEFS` changes are applied. If in doubt, `make clean` then rebuild.

