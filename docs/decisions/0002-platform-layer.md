# ADR 0002: SDL3 platform layer

- Status: accepted
- Date: 2026-07-12

## Context

Window creation, input, controllers, and event handling vary significantly between target operating
systems. Reimplementing those facilities would delay renderer and runtime development.

## Decision

SDL3 is the initial platform dependency. It remains behind `anvil::platform`; application code must
not expose SDL types in public APIs. The dependency is pinned to release `3.4.12` and built
statically when a system package is unavailable.

## Consequences

Anvil gains a mature cross-platform base while preserving the ability to replace or supplement SDL
for a future platform. SDL updates require a focused dependency change and full platform smoke test.
