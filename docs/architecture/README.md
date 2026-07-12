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
- `anvil::platform`: SDL3-backed application lifecycle and event processing.
- `anvil_sandbox`: an executable smoke test and future vertical-slice playground.

New modules should expose a narrow public API under `include/anvil/<module>/`, keep implementation
under `src/`, and provide an `anvil::<module>` CMake alias.

## Runtime lifecycle

The platform application owns the operating-system event pump and monotonic frame clock. Client
hooks receive start and stop notifications, zero or more fixed simulation updates, and one frame
update containing interpolation and timing diagnostics. Exit requests are explicit and may be made
from any lifecycle hook running on the application thread.
