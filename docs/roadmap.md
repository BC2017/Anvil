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

- Application lifecycle and fixed-timestep main loop
- Input action mapping and controller support
- Math primitives and transforms
- Job system and thread-safe queues
- Memory tracking and lightweight profiling
- Filesystem abstraction and event infrastructure

Milestones 2–7 will cover rendering, worlds and assets, production rendering, editor workflows,
gameplay systems, and packaging/hardening. Their detailed requirements will be refined at the end of
each preceding milestone.
