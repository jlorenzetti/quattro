# Presentation Identity — Phase 2.5 Design Note

**Status:** Phase 2.5 completed (ROM/PETSCII only)  

**Note:** Custom charset deferred to a standalone micro-project (outside Quattro scope). Build uses ROM charset / PETSCII 0xA0. See `docs/notes/custom-charset-investigation.md`.

---

## Guiding phrase

> Quattro should look like a severe character-graphics system built specifically for falling blocks, not default C64 text mode with blocks.

---

## Initial technical decision: hi-res charset

**Charset mode: standard (hi-res), not multicolour.**

Rationale: hi-res is sharper, more severe, and closer to “software-like” presentation. Multicolour risks a softer, decorative, or “toy-like” feel. This choice is fixed for Phase 2.5.

---

## Charset inventory (Phase 2.5: ROM/PETSCII only)

| Element     | Code | Glyph(s)  | Notes                                                   |
| ----------- | ---- | --------- | ------------------------------------------------------- |
| Block       | 0xA0 | Full block| Same glyph for locked and active; colour differentiates |
| Frame       | 0xA0 | Full block| Same as block; colour differentiates; no line chars     |
| Empty cell  | 0x20 | Space     | No custom glyph; no grid or texture                     |
| Text (HUD)  | ROM  | PETSCII   | Not custom in 2.5                                       |

**Current build:** ROM charset only. No custom charset. See `docs/notes/custom-charset-investigation.md`.

---

## Colour map (Phase 2.5 final)

| Role           | C64 colour | Value | Purpose                                       |
| -------------- | ---------- | ----- | --------------------------------------------- |
| Background     | Black      | 0     | Whole screen; border ($D020) = background     |
| Frame          | Light grey | 15    | Boundary; distinct from content               |
| Empty cell     | Black      | 0     | Same as background; no grid or texture        |
| Locked block   | White      | 1     | Primary content; clearer than frame           |
| Active piece   | Cyan       | 3     | Distinct from locked; readable under motion   |
| HUD text       | Light grey | 15    | Matches frame; functional                     |
| Game over band | Light grey | 15    | 3-row band inside board; reverse text (black on grey) |

**Rule:** Frame and locked blocks differ (frame = 15, locked = 1). Border matches background for a single black canvas. Colour by state, never by piece type.

---

## Layout constants (Phase 2.5 final)

Board centred; HUD left (col 1); game over band inside board.

| Constant         | Value | Meaning                        |
| ---------------- | ----- | ------------------------------ |
| `BOARD_SCREEN_X` | 15    | Board left column (centred)    |
| `BOARD_SCREEN_Y` | 2     | Board top row                  |
| `HUD_SCREEN_COL` | 1     | HUD labels/values start (left) |
| `HUD_SCREEN_ROW` | 2     | SCORE line row                 |
| Game over band   | rows 11–13, cols 15–24 | 3-row light grey band; "GAME  OVER" reverse, centred |

**Composition rationale:** Board as anchor; HUD compact at col 1; border = background (black); game over as message-on-field (3-row band, reverse PETSCII so band stays visible).


---

## Principles (reference)

- Field first; frame and HUD support, do not compete
- Functional clarity: every element has a role
- Maximum readability under motion
- C64-native austerity; one glyph family, one colour system
- No piece-type colours, no decorative playfield texture, no dotted/grid empty cell in first pass

