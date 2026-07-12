# ADR 0004: Fixed simulation timestep

- Status: accepted
- Date: 2026-07-12

## Context

Rendering cadence varies with display refresh, operating-system scheduling, and workload. Physics
and gameplay simulation need a stable timestep for predictable behavior and reproducible tests.

## Decision

Anvil separates fixed simulation updates from per-frame updates. A monotonic clock feeds a
deterministic scheduler that accumulates wall-clock time, emits zero or more fixed updates, and
provides an interpolation alpha for rendering.

Elapsed time is clamped after long stalls, and the number of fixed updates per rendered frame is
bounded. Time discarded by either limit is reported for diagnostics. This intentionally favors a
responsive process over attempting an unbounded catch-up that can create a simulation death spiral.

## Consequences

Gameplay and physics must mutate simulation state during fixed updates. Per-frame work may consume
the interpolation alpha but must not assume one simulation update per frame. Replay-grade
determinism will require later control over inputs, random seeds, floating-point behavior, and
external state in addition to this timing model.
