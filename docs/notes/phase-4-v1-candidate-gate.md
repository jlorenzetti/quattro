# Phase 4 — v1 Candidate Gate

## 1. Goal
Clarify and enforce the step that decides and executes the few optional candidates that may legitimately enter v1 before RC validation.

This gate exists to keep Phase 4 disciplined: no broader feature ideation, no schedule drift, and no RC closure until v1 optional candidates are explicitly resolved.

## Status

- [x] **Micro SFX pass:** accepted into v1 (full set shipped on C64 default build; `QUATTRO_AUDIO=0` retained as fallback build option).
- [ ] **Next-piece preview:** pending (Phase 4 candidate).
- [ ] **Micro polish:** pending (only if it follows directly from the two passes above).

## 2. Candidate set
Use this exact candidate set:

- **Micro SFX pass** → strong v1 candidate (design: [`sound-v1-vocabulary.md`](sound-v1-vocabulary.md); implementation + ship notes: [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md))
- **Next-piece preview** → evaluate through a concrete compositional fit pass
- **Micro polish** → only if it emerges directly from the two passes above; no free-floating polish bucket

## 3. Rules
- no return to broad feature ideation
- no reopening settled Phase 2.5 / 2.6 / 3 decisions
- optional items are attempted one by one, not in parallel
- each candidate must end in one of two outcomes:
  - **accepted into v1**
  - **explicitly deferred from v1**
- RC validation does **not** begin until this gate is resolved

## 4. Recommended order
1. **Micro SFX pass first**
   - because it is the strongest candidate and has a simpler rollback path
2. **Next-piece preview second**
   - only after audio is resolved
   - evaluate through an actual compositional integration attempt, not abstract debate
3. **Micro polish only if needed**
   - and only if it directly follows from the previous two steps

## 5. Decision criteria

### Micro SFX
Accept into v1 only if:
- integration is low-risk
- no regressions in timing/input feel
- sound remains minimal and structurally coherent

Otherwise: defer.

**Outcome:** accepted into v1 (full set shipped on C64 default build; `QUATTRO_AUDIO=0` retained as fallback build option). See [`phase-4-micro-sfx.md`](phase-4-micro-sfx.md).

### Next-piece preview
Accept into v1 only if:
- it fits the existing left-column composition
- it does not noticeably increase interface density
- it does not weaken field-first balance

Otherwise: defer.

### Micro polish
Accept only if:
- it is tiny
- it is clearly beneficial
- it does not open new scope

Otherwise: defer.

## 6. Exit condition
This gate is closed only when:
- each candidate has been either implemented-and-accepted or explicitly deferred
- the resulting v1 boundary is now real, not hypothetical
- only then can final RC validation begin

