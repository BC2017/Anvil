# ADR 0009: Deterministic gameplay system graph

- Status: accepted
- Date: 2026-07-12

## Context

Gameplay features eventually span input, scripts, animation, physics, audio, and render extraction.
Relying on registration order alone makes hidden ordering dependencies fragile and prevents the
engine from diagnosing invalid schedules before simulation begins.

## Decision

Anvil models fixed-step gameplay work as named systems in three ordered phases: pre-simulation,
simulation, and post-simulation. Systems may declare that they run after other named systems. A
build step validates names and dependencies, rejects backwards phase edges and cycles, and creates
a stable topological order using registration order as the deterministic tie-breaker.

The graph executes only after a successful build. Every callback receives the fixed delta and a
monotonic tick index. The tick advances only after every system completes; exceptions propagate to
the runtime owner because the graph cannot generically roll back gameplay state.

## Consequences

System ordering is explicit, inspectable, and reproducible in tests. Adding or changing systems
invalidates the compiled order and requires another build. Parallel scheduling is deferred until
systems can also declare data access, which will allow safe concurrency without weakening the
deterministic contract.
