# Contributing

## Development workflow

1. Keep changes focused and preserve subsystem boundaries.
2. Configure, build, and test through a checked-in CMake workflow preset.
3. Add or update tests for observable behavior.
4. Keep compiler warnings enabled and do not suppress a warning globally to fix one site.
5. Record durable architectural choices in `docs/decisions/`.

Run `clang-format` on changed C++ files when it is available. CI is the source of truth for
cross-platform compilation and static analysis.

## Commit guidance

Use concise imperative commit subjects. Do not commit generated build output, downloaded
dependencies, credentials, or machine-specific presets.

## Definition of done

A change is done when it builds without warnings, its relevant tests pass, public behavior is
documented, and failures produce enough context to diagnose the problem.
