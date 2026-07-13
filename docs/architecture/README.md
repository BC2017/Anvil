# Architecture

Anvil is organized as a layered set of libraries rather than one monolithic engine target.

```text
applications and tools
        |
runtime systems (future)
        |
core + platform
        |
operating system and third-party libraries
```

Dependencies point downward. Engine libraries never depend on applications, runtime libraries do
not depend on editor code, and platform-specific types do not cross public engine boundaries.

## Current modules

- `anvil::core`: diagnostics, assertions, versioning, and deterministic frame scheduling.
- `anvil::assets`: stable asset identity, logical-path indexing, and import metadata.
- `anvil::platform`: SDL3-backed application lifecycle and event processing.
- `anvil::render`: engine-owned rendering capabilities with a private Vulkan implementation.
- `anvil_sandbox`: an executable smoke test and future vertical-slice playground.

New modules should expose a narrow public API under `include/anvil/<module>/`, keep implementation
under `src/`, and provide an `anvil::<module>` CMake alias.

## Runtime lifecycle

The platform application owns the operating-system event pump and monotonic frame clock. Client
hooks receive start and stop notifications, zero or more fixed simulation updates, and one frame
update containing interpolation and timing diagnostics. Exit requests are explicit and may be made
from any lifecycle hook running on the application thread.

## Rendering boundary

Public rendering headers contain only Anvil-owned types. Vulkan headers, handles, validation
callbacks, and loader interaction remain private implementation details. The renderer queries
runtime support rather than assuming that the SDK header version matches the installed loader or
physical devices.

## Asset boundary

Asset references cross subsystem and serialization boundaries as stable 128-bit IDs. Source paths
are normalized project-relative metadata used by tools; they are never the durable identity of an
asset. Registry reads return value snapshots so callers do not retain pointers invalidated by an
editor reimport or asynchronous update.
