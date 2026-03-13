# Gravity model (Quattro C64)

## Summary

- **What:** Level-based frames-between-drops. Implemented with a **dedicated gravity counter** in the game loop and a **lookup table** (platform layer).
- **Reference:** Gravity curve initially informed by NES Tetris pacing, then adapted for Quattro and PAL 50 Hz.
- **Runtime:** Table tuned for **PAL 50 Hz** (C64 jiffy). Approximate time-per-row preserved by scaling from a 60 Hz pacing reference.

## Why a dedicated counter

- **`frame % interval`** ties gravity to global frame alignment. A dedicated counter increments each game frame; when it reaches the level’s interval, we tick gravity and reset the counter. Gravity timing is then independent of when the game started or other loop timing.
- Behaviour is easy to reason about and document: “every N frames, drop one row,” with N coming from the table.

## Table and lookup

- **Module:** `src/platform/c64/gravity.c` + `gravity.h`. Table and `gravity_interval_for_level(level)`.
- **Levels 0–29:** One entry per level. Levels ≥ 30 use the last entry (fastest).
- **Levels 0–9:** Explicitly graded for start-level choice (slow at 0, fast at 9).
- **Minimum interval:** 1 frame (no division by zero; fastest level remains playable).

## Reference and adaptation

- The progression shape (slow at level 0, fast at 9) is preserved; the table is scaled for 50 Hz so that approximate time-per-row is in the same ballpark, without copying frame counts literally.

## Tests

- **Host:** `tests/test_gravity.c` checks lookup for levels 0, 9, 29, and 99 (clamp). No timing tests; gravity timing lives in the C64 loop only.
