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

- `anvil::core`: diagnostics, assertions, versioning, and future low-level services.
- `anvil::platform`: SDL3-backed application lifecycle and event processing.
- `anvil_sandbox`: an executable smoke test and future vertical-slice playground.

New modules should expose a narrow public API under `include/anvil/<module>/`, keep implementation
under `src/`, and provide an `anvil::<module>` CMake alias.
