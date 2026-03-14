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

---

## 2026-03-14 — C64 runtime seed initialization
**Status:** accepted

**Decision**
- Mainline C64 gameplay seeds the PRNG pseudorandomly from runtime timing (jiffy clock at RETURN in start/help, mixed with start level).
- Deterministic seeds remain the default for host tests and are available for C64 debug builds via `QUATTRO_FIXED_SEED` at compile time (e.g. `make c64_fixed_seed` or `make c64 C64_DEFS="-DQUATTRO_FIXED_SEED=12345"`).
