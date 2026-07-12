# ADR 0003: Dependency policy

- Status: accepted
- Date: 2026-07-12

## Decision

Dependencies are accepted only when they remove substantial undifferentiated work or provide
specialist functionality. Each dependency must have a compatible license, an active maintenance
story, a pinned release or commit, and a narrow integration boundary.

CMake first looks for a compatible installed package and otherwise fetches the pinned source into
the build directory. Dependencies are never committed as generated source trees. Updating a pin is
an explicit change accompanied by builds and tests on all supported platforms.

## Consequences

Clean builds require network access unless dependencies are preinstalled or supplied through a
configured package cache. Builds remain repeatable at the source revision level without growing the
repository with vendored build artifacts.
