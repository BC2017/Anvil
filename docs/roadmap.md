# Roadmap

## Milestone 0 — Engineering foundation

- [x] Repository layout and dependency policy
- [x] CMake presets for supported local and CI configurations
- [x] Strict compiler warnings and opt-in sanitizers/static analysis
- [x] Core logging, assertions, version generation, and test harness
- [x] SDL3 application lifecycle smoke test
- [x] Windows and Linux continuous integration definitions
- [ ] Verify both CI jobs in the hosted environment
- [x] Adopt the MIT license for public distribution

Exit gate: a clean checkout configures, builds, tests, and runs a minimal executable from documented
commands on Windows and Linux.

## Milestone 1 — Core and platform

- [x] Application lifecycle and fixed-timestep main loop
- [ ] Input action mapping and controller support
- [ ] Math primitives and transforms
- [ ] Job system and thread-safe queues
- [ ] Memory tracking and lightweight profiling
- [ ] Filesystem abstraction and event infrastructure

Milestones 2–7 will cover rendering, worlds and assets, production rendering, editor workflows,
gameplay systems, and packaging/hardening. Their detailed requirements will be refined at the end of
each preceding milestone.

## Milestone 2 — Rendering foundation

- [x] Backend-neutral instance and capability API
- [x] Vulkan 1.3 loader, validation, and adapter bootstrap
- [ ] Physical-device selection and queue-family planning
- [ ] Logical device and feature negotiation
- [ ] GPU allocator and resource lifetime model
- [ ] Window surface and swapchain lifecycle
- [ ] Command submission and synchronization primitives
- [ ] Render graph and offscreen image tests

## Milestone 3 — World and assets

- [x] Stable asset identifiers and thread-safe registry
- [ ] Versioned asset-registry persistence
- [ ] Entity/component world model and hierarchical transforms
- [ ] Versioned scene serialization and migrations
- [ ] Asynchronous asset loading and dependency tracking
- [ ] glTF, image, material, and shader importers
- [ ] Cooked runtime formats and derived-data cache

## Milestone 4 — Production rendering

- [x] Linear-color PBR material contract and normalization
- [x] Physically unitized directional, point, and spot lights
- [ ] Render-facing scene extraction and stable resource handles
- [ ] Physically based shader and material texture bindings
- [ ] Directional and local-light shadows
- [ ] HDR targets, exposure, and tone mapping
- [ ] Frustum culling and instanced submission
- [ ] Skeletal animation and quality tiers

## Milestone 5 — Editor

- [x] Bounded undo/redo history and saved-state tracking
- [ ] Composite transactions and continuous-edit coalescing
- [ ] Scene hierarchy, selection, and inspector
- [ ] Viewport camera, picking, and transform gizmos
- [ ] Asset browser and import settings
- [ ] Play-in-editor with isolated runtime state
- [ ] Scene/prefab workflows and hot reload

## Milestone 6 — Gameplay systems

- [x] Deterministic fixed-step system graph and execution phases
- [ ] Entity/component queries and deferred structural changes
- [ ] Input actions and gameplay command buffering
- [ ] Native and managed scripting lifecycle
- [ ] Physics integration and collision events
- [ ] Spatial audio and listener/source components
- [ ] Deterministic random streams, recording, and replay
- [ ] Save-game state and versioned migrations
