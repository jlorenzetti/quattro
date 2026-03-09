# Host Core Contract

## Purpose
The host-side core is the reference implementation of Quattro gameplay semantics.

## Rules
1. The host-side core defines the authoritative game rules.
2. The first C64 integration must adapt to the host core, not redefine it.
3. No new gameplay features are introduced before the first playable C64 build.
4. Any change to gameplay semantics must be reflected in host-side tests first.
5. Platform-specific optimization must not silently alter core behavior.

## Implications
- Tests are the first line of protection against semantic drift.
- C64 integration is a presentation/platform task before it is a gameplay redesign task.
- Any later refinement to feel must be explicit, documented, and test-backed.
