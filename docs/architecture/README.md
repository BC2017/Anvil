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
- `anvil::gameplay`: deterministic fixed-step system scheduling and execution.
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

The platform also offers a deliberately small debug canvas using engine-owned colors, rectangles,
lines, and text. It supports bootstrap diagnostics and visual smoke tests while keeping SDL types
private; production 3D rendering remains the responsibility of `anvil::render`.

## Rendering boundary

Public rendering headers contain only Anvil-owned types. Vulkan headers, handles, validation
callbacks, and loader interaction remain private implementation details. The renderer queries
runtime support rather than assuming that the SDK header version matches the installed loader or
physical devices.

Render-facing scene data uses linear colors and explicit physical units. Authoring values are
normalized before GPU extraction, and every correction remains observable so editor tooling can
identify malformed source assets instead of allowing NaNs or invalid ranges into shaders.

## Asset boundary

Asset references cross subsystem and serialization boundaries as stable 128-bit IDs. Source paths
are normalized project-relative metadata used by tools; they are never the durable identity of an
asset. Registry reads return value snapshots so callers do not retain pointers invalidated by an
editor reimport or asynchronous update.

## Editor boundary

Editor code lives under `tools/` and depends on public engine modules. Engine runtime modules never
depend on editor targets. All document mutations flow through named commands so hierarchy,
inspector, asset, and viewport tools share undo/redo and saved-state behavior.

## Gameplay boundary

Gameplay systems execute in explicit pre-simulation, simulation, and post-simulation phases. Named
dependencies compile into a deterministic order before execution. Systems receive fixed-step time
and tick identity but remain independent of platform, renderer, and editor implementations.

## Distribution boundary

The supported runtime layout is defined by CMake install rules rather than copied directly from a
developer build tree. Every staged runtime includes a schema-versioned manifest describing its
engine version, configuration, operating system, and architecture; packaging validation executes
the installed binary from that layout.
