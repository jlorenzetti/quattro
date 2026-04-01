# Architecture

## Principles
- Separate game logic from platform concerns
- Keep data structures simple and explicit
- Localize low-level optimizations
- Make the core readable and testable

## High-level modules

### Core
Pure or mostly pure game logic:
- board state
- active piece state
- movement and rotation rules
- collision checks
- locking
- line clear
- score/level progression
- game state transitions

### Platform
C64-specific responsibilities:
- boot and entry point
- frame timing
- input polling
- screen/color RAM writes
- memory layout
- interrupts / synchronization if needed

### Render
Responsible for:
- translating board state to screen representation
- minimizing writes
- drawing minimal UI elements

### Audio
C64: SID one-shots in `src/platform/c64/audio.c` (six cues; compile-time mute `QUATTRO_AUDIO=0`). Core stays audio-free; optional `GameStepResult` on command/gravity lets the platform fire cues without embedding SID in rules — see [`notes/phase-4-micro-sfx.md`](notes/phase-4-micro-sfx.md). No gameplay music bed in v1.

### Tooling
Responsible for:
- build pipeline
- emulator launch
- tests
- release packaging

## Optimization rule
Do not optimize preemptively.
Measure, identify hotspots, then isolate low-level code where it matters.
