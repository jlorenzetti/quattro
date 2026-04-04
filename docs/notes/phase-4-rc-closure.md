# Phase 4 — RC closure

**Status:** release verification record (audit trail for **0.1.0** RC); authoritative reader-facing text is [`release-0.1.0.md`](release-0.1.0.md).

**Date:** 2026-04-03  
**Purpose:** Build/package verification and RC checklist outcome after Block A (optional candidates) closed.

**Related:** v1 boundary and candidate decisions — [`phase-4-v1-candidate-gate.md`](phase-4-v1-candidate-gate.md). Micro SFX — [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md). Public release text — [`release-0.1.0.md`](release-0.1.0.md).

## What was verified (this pass)

### Documentation

- Release-facing docs aligned: [`README.md`](../../README.md), [`roadmap.md`](../roadmap.md), [`CHANGELOG.md`](../../CHANGELOG.md).
- No repository-relative links broken in those files (spot-checked).
- No absolute filesystem paths in tracked docs for build instructions.

### Build / artifact

From repository root, end-to-end:

1. `make clean`
2. `make test` — **OK** (host core test runner).
3. `make c64` — produces **`build/quattro.prg`** (default build: SID micro SFX on).
4. `rm -f build/quattro.prg && make c64 C64_DEFS="-DQUATTRO_AUDIO=0"` — produces a **mute-audio** PRG (compile-time `QUATTRO_AUDIO=0`), as documented in README **Building**.
5. `rm -f build/quattro.prg && make c64` — **restores** the default (audio-on) PRG for release use.

**Artifact path:** `build/quattro.prg` (tracked only as build output; not committed).

### Regression checklist (RC)

| Check | Result |
| ----- | ------ |
| Documented host test command (`make test`) | **Pass** |
| Documented default C64 build (`make c64`) | **Pass** |
| Documented mute build (`QUATTRO_AUDIO=0`) | **Pass** |
| Boot → title → start/help → game → game over → start/help (full UI loop) | **Pass** (interactive smoke / playtest; no regressions observed). |
| Keyboard gameplay | **Pass** (same session). |
| Joystick port 2 gameplay | **Pass** (same session). |
| Next-piece preview / composition | **Accepted in Block A**; no code change in this pass. |
| Default SID micro SFX | **Accepted in Block A**; default PRG builds with audio. |
| Baseline presentation (ROM/PETSCII) | No rendering changes in this pass. |

### Interactive smoke / playtest

After automated checks: full UI loop, keyboard, joystick (port 2), preview, default audio, baseline presentation — no regressions observed.

## Release-blocking issues

**None** (automated tests, builds, and interactive smoke as above).

## Non-blocking limitations (known)

- **SID:** occasional faint clicks/crackle on some setups — documented as non-blocking in [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md).
- **`make clean`:** also removes `compile_commands*.json` if present (see [`Makefile`](../../Makefile)); regenerate with `make compdb` / `compdb-all` if needed.
- **Browser demo:** experimental; not part of the C64 RC artifact.

## Public release note

Authoritative **0.1.0** text for readers and GitHub Releases: [`release-0.1.0.md`](release-0.1.0.md). **Source of truth for file-level changes:** [`CHANGELOG.md`](../../CHANGELOG.md).

## Recommendation

**Ready to tag `0.1.0`** — checklist complete; no release-blocking issues.
