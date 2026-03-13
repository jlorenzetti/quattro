# Custom charset — technical investigation

**Status:** open (isolated from main build)  
**Rule:** Charset investigation is isolated from gameplay and presentation baseline. Do not test charset changes inside the full Quattro game until a minimal standalone proof succeeds.

---

## Objective

Integrate a custom hi-res charset on C64 (block + frame glyphs) for Quattro Phase 2.5. The direction is valid; the implementation is blocked by C64/VIC/runtime integration issues in the current llvm-mos setup.

---

## Attempts and outcomes

| # | Approach | Result |
|---|----------|--------|
| 1 | Copy Character ROM ($D000) to $2000, set $D018 | Severe screen corruption. $2000 likely overlaps llvm-mos code/data. |
| 2 | Move charset to $3800, use $01 = $34 to map ROM before copy | Lowercase chars, program freeze. $34 (CHAREN=1) maps I/O, not ROM; wrong data copied. |
| 3 | Use $01 = $32 (CHAREN=0) to map Character ROM | Same: lowercase, freeze. Copy worked (vsf showed correct charset at $3000); crash occurred after copy, before $D018 write. Hypothesis: $01 restore altered memory mapping and corrupted executing code. |
| 4 | Embedded charset (2K in binary), copy to $3000, no $01 manipulation | No freeze; game runs. Screen still corrupt (checkerboard, red lines, wrong chars). Corruption tied to enabling custom charset via $D018. |
| 5 | Explicit VIC init: $DD00, $D011, $D016, $D018 | Same corruption. |
| 6 | Disable custom charset; use ROM + PETSCII 0xA0 for block/frame | Correct display, stable game. **Current baseline.** |

---

## Conclusions

1. Corruption is caused by custom charset setup ($D018 / VIC), not by video draw logic or gameplay.
2. Copying from ROM via $01 manipulation is fragile with llvm-mos: $01 restore can crash or destabilise.
3. Embedded charset + copy to $3000 runs correctly (data is valid per vsf), but display corrupts when $D018 points to the custom charset.
4. Likely causes still open:
   - llvm-mos memory layout using $3000 or nearby regions
   - Interaction between CRT, stack, or runtime and $3000–$37FF
   - VIC register ordering or timing
   - VICE / llvm-mos runtime specifics

---

## Next steps (isolated)

1. **Charset probe program** — Minimal C64 program (no game loop, no input, no core): clear screen, install charset at chosen address, set $D018, print a few known chars, halt. If this fails, the issue is in the basic VIC setup.
2. **Linker / memory map** — Determine exactly where llvm-mos places code and data; identify safe 2K region for charset.
3. **Reintegration** — Only after the probe program is stable, attempt integration into Quattro.

---

## Reference

- Unused charset implementation (for investigation): `src/platform/c64/charset.c`, `charset.h` (not linked in main build).
- Design note: `docs/notes/presentation-identity-2.5.md`.
