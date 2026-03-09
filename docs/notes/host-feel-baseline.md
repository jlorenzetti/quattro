# Host-side feel baseline

Short baseline of the current host spike behaviour. Observation and documentation only; no tuning in this pass.

## Spawn

Current choice: `QUATTRO_SPAWN_X = 3`, `QUATTRO_SPAWN_Y = 0` (top-left of spawn area, top row).

**Convention:** Spawn is the **top-left of a conceptual 4×4 cell**. All piece shapes are defined so they are **centered in that 4×4** (see `src/core/piece.c`). One common origin therefore gives consistent visual centering for all seven pieces on the 10-wide board; no piece-dependent spawn coordinates.

**Status:** Accepted for spike. Revisit when tuning feel; placement and centering affect perceived fairness and readability.

## Rotation

Rotation is defined by **static shape tables** per piece kind and rotation state (see `src/core/piece.c`). Validity is determined by `rules_can_place_piece`: no wall kicks, no implicit corrective behaviour. So rotation is **table transformation + validation**, not physics. Each piece has a fixed centre implied by its offset table; there is no separate “rotation centre” computation.

**Status:** Accepted for spike. Minimal micro-kick may be considered later only if playtesting justifies it.

## Lock

**Immediate lock** on gravity tick when the piece cannot move down. This is a deliberate spike simplification and accepted spike severity — not the intended final feel. First candidate for later feel review if controllability proves too harsh.

## RNG

Simple deterministic LCG. The choice is **historically sober and test-friendly**: reproducibility and regression diagnosis are primary. This is not a claim of historical authenticity.

## Pre–C64 playtest checklist

When playing the host debug before or during C64 integration, observe:

- Spawn position and centering feel
- Lock harshness and controllability
- Rotation predictability and readability

## Status

- Baseline accepted for host spike.
- Not yet tuned for final play feel.
- C64 integration should preserve semantics before refining feel.
