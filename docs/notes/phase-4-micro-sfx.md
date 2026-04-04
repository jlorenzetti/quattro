# Phase 4 — Micro SFX (implementation and ship notes)

**Status:** engineering reference (C64 SID). Default build: **all six** vocabulary cues; §6 refinement **closed**. `QUATTRO_AUDIO=0` = supported silent build.

**Vocabulary (design):** [`sound-v1-vocabulary.md`](sound-v1-vocabulary.md). **Gate:** [`phase-4-v1-candidate-gate.md`](phase-4-v1-candidate-gate.md). **Driver:** [`audio.c`](../../src/platform/c64/audio.c) file header.

## 1. Goal

**Decision:** ship a minimal micro SFX layer for v1 without changing gameplay, presentation layout, or the input/timing model — or defer the entire pass per gate. **Outcome (C64):** full six-cue set accepted; criteria in §5.

## 2. What shipped (C64)

- **Core:** optional `GameStepResult *` on `game_apply_command` / `game_tick_gravity` (NULL-safe). Core stays audio-free.
- **Platform:** `audio_init` / `audio_play` in [`audio.c`](../../src/platform/c64/audio.c) + [`audio.h`](../../src/platform/c64/audio.h). Triggers in [`main.c`](../../src/platform/c64/main.c): rotate after successful command; lock / line clear after gravity telemetry; game over, start confirm, title on state entry.
- **Mute:** build with `QUATTRO_AUDIO=0` (see [`README.md`](../../README.md) **Building**; remove stale `build/quattro.prg` if the linker skips rebuild).

### Tuning snapshot

**Source of truth:** [`audio.c`](../../src/platform/c64/audio.c) (macros, `k_title_motive_notes`, `play_line_clear_arp`). Inter-note / inter-step gaps below are **`spin_iters` counts**, not milliseconds.

| Shared pacing | Value | Macros (all aligned) |
| ------------- | ----- | -------------------- |
| Arp / menu gap | 260 | `SPIN_TITLE_GAP_ITERS`, `ARP_GAP_LINE_CLEAR_ITERS`, `ARP_GAP_START_CONFIRM_ITERS` |

| Cue | SID voice | Shape | Gate hold (iters) | AD / SR (pulse) | Notes |
| --- | --------- | ----- | ----------------- | --------------- | ----- |
| Title | 2 | Triangle + LP sweep ×5 | per-note in `k_title_motive_notes` | triangle row AD + `0xA0` SR | Filter bypass after gesture |
| Start confirm | 2 | Two `blip_pulse` | `BLIP_GATE_HOLD_ROTATE_ITERS` (380), `BLIP_GATE_HOLD_LOCK_ITERS` (430) | `0x06`/`0x90`, `0x05`/`0x98` | Gap 260 between blips |
| Rotate | 1 | `blip_pulse` | 380 | `0x02`/`0x90` | Freq `k_sid_freq_rotate` |
| Lock | 1 | `blip_pulse` | 430 | `0x04`/`0xA0` | Freq `k_sid_freq_lock` |
| Line clear | 1 | 3× `blip_pulse_step_retrigger` | 300, 300, 320 | `0x04`/`0xA0` ×2; `0x06`/`0xC0` last | Gap 260 between steps |
| Game over | 1 | `blip_pulse_default_hold` | `BLIP_GATE_HOLD_ITERS` (280) | `0x06`/`0xC0` | Freq `k_sid_freq_game_over` |

## 3. Implementation order (reference)

Aligned with vocabulary priority for trimming: lock → line clear → rotate → threshold cues (game over, start confirm, title). Early stop: ship a coherent subset (trim from vocabulary bottom) or defer entirely — not “one more cue.”

## 4. Guardrails

- One sonic family for pulse gameplay; six events max; no continuous gameplay music; no title loop by default; no sound on lateral repeat or per-frame soft drop.
- No timing-loop or input-model changes; if SID cost threatens feel, measure (`make c64_perf`) or defer.
- **Rollback:** one choke point (`QUATTRO_AUDIO=0` or revert platform audio).

## 5. Accept / defer (gate)

Criteria from [`phase-4-v1-candidate-gate.md`](phase-4-v1-candidate-gate.md) §5 — **Accept** only if stable, coherent, small, non-invasive, and smoke checks pass (keyboard + joystick, primary emulator). Document **full six** or an **intentional subset** following vocabulary trim order (do not ship threshold-only without operational spine). **Defer** if risk or regressions need broader scope.

**C64 outcome:** full six accepted and implemented; occasional pop/crackle edge cases documented in §6.

## 6. Refinement (post-accept, closed)

Tightening only — same event set; no second accept/defer gate.

- **Audibility:** SID master at max; pulse cues use **~50% duty** (`PW` 0x800).
- **Anti-click:** shared `blip_pulse` path (park, load with gate off, settle, gate on, hold, gate off, repark). **Voice split:** menu (title + start confirm) on **SID voice 2**; gameplay on **voice 1**. Title: triangle + low-pass on voice 2, then filter bypass; gameplay pulse. Conventional register use (no TEST strobe).
- **Hierarchy:** line clear = three-step pulse arp; other cues per vocabulary; behaviour summary in [`audio.c`](../../src/platform/c64/audio.c) file header.

**Validation (v1):** audibility, sparseness, hierarchy, no timing/input regression. **Artifacts:** occasional faint clicks or weak attacks are **non-blocking** for v1 (hardware/emulator variance); use `QUATTRO_AUDIO=0` if needed.

## 7. Out of scope (this work)

Charset changes, gameplay rule changes, extra vocabulary, continuous gameplay music / melody beds. (Preview UI is separate from SFX; see release note.)
