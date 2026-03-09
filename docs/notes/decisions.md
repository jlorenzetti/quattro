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
