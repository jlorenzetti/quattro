# Decision Log

## 2026-03-09 — Primary toolchain
**Status:** accepted

**Decision**
Use `llvm-mos` as the primary toolchain.
Allow handwritten assembly only for isolated critical modules when measurement or profiling clearly justifies it.

**Guardrail**
No assembly in core game logic unless profiling demonstrates necessity.

---

## 2026-03-09 — Rendering and presentation baseline
**Status:** accepted

**Decision**
- character-based board with custom charset
- one next piece preview
- no music during gameplay
- simple local high score in v1
- first technical spike: host-side core simulation before C64 integration

---

## 2026-03-09 — Core language, RNG and rotation policy
**Status:** accepted

**Decision**
- core language: C
- piece generation: simple deterministic PRNG
- no 7-bag
- support two rotation directions
- no wall kicks by default
- evaluate a minimal micro-kick only later, and only if playtesting clearly justifies it

---

## 2026-03-12 — Custom charset: defer to isolated investigation
**Status:** accepted

**Decision**
Custom charset remains a valid direction for Phase 2.5, but implementation is blocked by unresolved C64/VIC/runtime integration issues in the current llvm-mos setup. The project baseline returns to the stable ROM-charset build (PETSCII 0xA0 for block and frame). Charset work continues as an isolated technical investigation and will only be reintegrated after a minimal standalone probe program succeeds.

**Rule:** Do not test charset changes inside the full Quattro game until the probe program validates VIC charset switching.

**Reference:** `docs/notes/custom-charset-investigation.md`
