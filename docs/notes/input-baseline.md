# Input baseline (C64 keyboard)

**Status:** active reference — current Quattro-owned gameplay input semantics for the C64 build.

### Scope and guardrails

- **SPACE remains soft drop** (not hard drop).
- **Repeat behaviour is owned by Quattro** (frame-based), not derived from C64 KERNAL key repeat.
- **Rotation is edge-triggered by default** and does not inherit repeat/hold semantics unless explicitly changed later.

### Timing baseline (frames)

#### Lateral movement (PAL-inspired)

- **DAS initial delay**: **6 frames**
- **Auto-repeat interval**: **2 frames**

#### Soft drop (SPACE)

- **Initial delay**: **3 frames**
- **Repeat interval**: **1 frame**
- **First press semantics**: **immediate 1-cell soft drop**, then delay/repeat while held.

### Left/right conflict rule (stable)

- **Newest held direction wins** (deterministic).
- Only **one lateral direction** is active at a time; no oscillation/jitter when both are held.
- If the **dominant direction is released** while the opposite direction is still held, the opposite direction takes over with a **fresh DAS window** (treated as a new press: immediate step, then DAS delay before repeats).
