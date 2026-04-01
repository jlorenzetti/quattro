# Sound — v1 vocabulary (micro SFX)

This note defines a **minimal v1 sound vocabulary** for Quattro: historically sober, structurally useful, aligned with project tone, and small enough to stay inside the Phase 4 candidate gate ([`phase-4-v1-candidate-gate.md`](phase-4-v1-candidate-gate.md)). Engineering notes (hooks, guardrails, refinement sign-off, **compact tuning snapshot**) live in [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md#tuning-snapshot); this file is the **design** target only.

## Framing

Quattro audio is **not** a generic “music + effects” stack. Treat it as a **small two-register sound language**:

| Register | Role | Typical events |
| -------- | ---- | -------------- |
| **Threshold / identity** | Threshold, memory, ritual brevity | title, start confirm, game over |
| **Operational / tactile** | Feedback on structurally meaningful actions | rotate, lock, line clear |

**Hard rules for v1**

- **No continuous gameplay music** — gameplay stays visually and aurally sparse; no background loop during play.
- Sound is reserved for **meaningful transitions** and **structurally important gameplay events**, not decoration.
- Avoid **chatty** audio: few events, clear hierarchy, no flourish for its own sake.

**One-sentence contract**

> Quattro v1 uses a minimal sound vocabulary: no continuous gameplay music, no sound on every repeated input, and no decorative audio. Sound is reserved for transitions and structurally meaningful gameplay events.

## Inspirations (spirit, not literal pastiche)

- **Tune A / Korobeiniki** — Only as a **restrained identity echo**: fragment, intervallic cell, short arpeggio or motto — **not** full theme playback. Suitable for a brief title motive or tiny threshold gesture. Over-citation risks turning Quattro into a Tetris tribute; keep it archetypal, not touristic.
- **Elektronika 60 / minimal bleep feedback** — Model for **in-game** feedback: essential, software-like ticks — but **filtered** through a sober C64 timbral language (SID as tool, not showpiece).
- **European home-computer / software restraint** (optional framing) — Short patterns, bare intervals, almost-signals at boot/title — **not** “chiptune” as genre; severity of the machine with minimal musicality where it helps **tone**, not fluff.

## v1 event set

Each row is the **design vocabulary**. **C64 (default build)** implements **all six** ([`phase-4-micro-sfx.md`](phase-4-micro-sfx.md)); use `QUATTRO_AUDIO=0` for a no-SID build. If a future port subsets cues, follow the trim order under **Recommended hierarchy** below.

| Event | Role | Purpose | Character | Priority (implementation) | C64 default build |
| ----- | ---- | --------- | --------- | ------------------------- | ----------------- |
| **Title motive** | Threshold / identity | Anchor “where this comes from” at boot | Short, dry, ritual; recognizable fragment, not song-like | 6 (lowest in implementation order — still valuable) | yes |
| **Start confirm** | Threshold / activation | Confirm run start without fanfare | Small, positive, decisive | 5 | yes |
| **Rotate** | Operational / tactile | Primary discrete action feedback | Short tick; precise, slightly bright, not aggressive | 3 | yes |
| **Lock** | Operational / structural | Piece becomes field | Firmer, lower or closed than rotate; weight, finality | 1 (highest) | yes |
| **Line clear** | Operational / release | Important positive event | Slightly more open or ascending than lock/rotate; still sober | 2 | yes |
| **Game over** | Threshold / closure | End of run | Short descending or flat gesture; grave, not melodramatic | 4 | yes |

**Gameplay vs outer states**

- **Outer states** (title, start, game over): may use the **identity** register — still **brief**; no invasive loop. **Title motive — default: play once on title entry; non-looping.** Loop only if a concrete implementation reason emerges mid-pass.
- **Gameplay**: **no melody, no loop** — only the operational set above.

## Recommended hierarchy (practical implementation priority)

Use this order when trimming or sequencing work — **not** an artistic ranking:

1. lock  
2. line clear  
3. rotate  
4. game over  
5. start confirm  
6. title motive  

If scope must shrink mid-pass, cut from the bottom of this list first; keep lock / line clear / rotate as long as possible. If the pass is very tight, **start confirm** and **game over** may be reordered to whichever state hook is cheaper in the existing flow; the list above remains the default **trim** order (drop from the bottom first).

## Timbre and family

- Prefer **one sonic family** for v1: same waveform family (or near), with variation mainly via **pitch**, **duration**, **envelope**, and **emphasis** — not unrelated identities (no “fat bass + metallic + chirpy” mix).
- Two acceptable directions: **(A)** software-like, dry envelopes, no tail; **(B)** slightly more C64 character, still sober. Default toward **A**; add **B** only if it stays unified.
- **Accepted refinement:** the **title motive** may use **threshold-only** SID-native colour (triangle + low-pass on the menu voice, then filter off) while **gameplay** stays **pulse** — see [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md) §6 and [`audio.c`](../../src/platform/c64/audio.c) file header.

## Explicit guidance (must respect)

- **SPACE remains soft drop** — do **not** tie a sound to every repeated soft-drop frame (“machine-gun” audio). If any drop feedback exists in a later phase, it must not fire per-frame on hold; v1 should avoid it entirely.
- **Move left/right** — stay **silent** in v1 unless a later phase proves otherwise; lateral repeat is too frequent.
- Gameplay SFX stay **sparse**; every sound must earn its place.
- All cues should feel like the **same object/system**.

## Not in v1

- Continuous gameplay music or any looping bed during play  
- Sound on every left/right step or repeat  
- Sound on every soft-drop step / per-frame hold  
- Decorative or non-functional flourishes  
- “Full” Tune A / Korobeiniki performance or any melody that dominates gameplay  

## Relation to Phase 4

This vocabulary is the **design authority** for the **micro SFX** candidate ([`phase-4-v1-candidate-gate.md`](phase-4-v1-candidate-gate.md)). **C64** shipped the full set; engineering and refinement sign-off: [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md). Other platforms may defer or subset only with explicit documentation and trim order.

Implementation details and guardrails: [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md) §6. Event set and vocabulary are fixed for v1.
