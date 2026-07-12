# ADR 0005: Backend-neutral rendering API with Vulkan first

- Status: accepted
- Date: 2026-07-12

## Context

Anvil needs explicit control over GPU resources and synchronization without coupling gameplay,
scene, or editor code to one graphics API. Supporting several backends before one is production
ready would multiply incomplete implementations.

## Decision

Engine-facing rendering APIs use Anvil-owned types and opaque implementation state. Vulkan is the
first backend and Vulkan 1.3 is the minimum runtime contract. Current SDK headers may be used while
newer runtime features remain capability-gated.

The initial rendering object owns instance lifetime, optional validation, debug diagnostics, and
adapter enumeration. Surface, device, queue, and resource ownership will be layered beneath the same
public module without exposing Vulkan handles to callers.

## Consequences

Vulkan-specific implementation complexity remains localized to the render module. A future backend
can implement the same engine contract, but backend portability is validated only when a second
implementation is actually required. Systems above rendering cannot depend on Vulkan headers or
native handles.
