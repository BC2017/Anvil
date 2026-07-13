# Anvil

Anvil is a work-in-progress, cross-platform 3D game engine. The project is being built as a series
of tested vertical foundations spanning platform, rendering, assets, editor, and gameplay systems.

## Start here

New contributors should read the [code tour](docs/code-tour.md) for a file-by-file guide and the
[architecture overview](docs/architecture/README.md) for subsystem boundaries and runtime data
flow. Public C++ headers under `include/anvil/` contain the detailed contracts for each type and
function; implementation comments focus on decisions that are not apparent from the code itself.

## Prerequisites

- CMake 3.28 or newer
- A C++23 compiler
- Git (CMake fetches pinned dependencies on first configure)
- Vulkan SDK 1.3 or newer when building the renderer
- Windows: Visual Studio 2026 with the Desktop C++ workload
- Linux: Clang, Ninja, and the SDL3 development prerequisites

## Build and test

On Windows:

```powershell
cmake --workflow --preset windows-debug
```

On Linux:

```bash
cmake --workflow --preset linux-debug
```

The first configure downloads the pinned SDL3 source release into the ignored `build/`
directory. To run the animated gameplay-system visualizer after a Windows debug build:

```powershell
./build/windows-msvc/apps/sandbox/Debug/AnvilSandbox.exe
```

The visualizer shows the live fixed-step tick, compiled gameplay-system order, activity, and an
animated scene extracted during the post-simulation phase. Close its window to stop it.

## Useful options

- `ANVIL_BUILD_SANDBOX`: build the SDL3 platform smoke-test application.
- `ANVIL_BUILD_EDITOR`: build editor foundation modules.
- `ANVIL_BUILD_RENDERER`: build Vulkan rendering modules.
- `ANVIL_WARNINGS_AS_ERRORS`: reject warnings in Anvil-owned code.
- `ANVIL_ENABLE_ASAN`: enable AddressSanitizer where supported.
- `ANVIL_ENABLE_UBSAN`: enable UndefinedBehaviorSanitizer where supported.
- `ANVIL_ENABLE_CLANG_TIDY`: analyze Anvil-owned targets during compilation.

See [the roadmap](docs/roadmap.md), [architecture](docs/architecture/README.md), and
[contribution guide](CONTRIBUTING.md) before making structural changes.

## License

Anvil is available under the [MIT License](LICENSE).
