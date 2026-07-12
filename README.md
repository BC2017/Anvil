# Anvil

Anvil is a work-in-progress, cross-platform 3D game engine. The project is currently in
Milestone 0: establishing a reproducible, tested engineering foundation before renderer work
begins.

## Prerequisites

- CMake 3.28 or newer
- A C++23 compiler
- Git (CMake fetches pinned dependencies on first configure)
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
directory. To run the sandbox after a Windows debug build:

```powershell
./build/windows-msvc/apps/sandbox/Debug/AnvilSandbox.exe
```

## Useful options

- `ANVIL_BUILD_SANDBOX`: build the SDL3 platform smoke-test application.
- `ANVIL_WARNINGS_AS_ERRORS`: reject warnings in Anvil-owned code.
- `ANVIL_ENABLE_ASAN`: enable AddressSanitizer where supported.
- `ANVIL_ENABLE_UBSAN`: enable UndefinedBehaviorSanitizer where supported.
- `ANVIL_ENABLE_CLANG_TIDY`: analyze Anvil-owned targets during compilation.

See [the roadmap](docs/roadmap.md), [architecture](docs/architecture/README.md), and
[contribution guide](CONTRIBUTING.md) before making structural changes.

## License

Anvil is available under the [MIT License](LICENSE).
