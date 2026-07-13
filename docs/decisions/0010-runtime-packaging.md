# ADR 0010: Reproducible runtime packages

- Status: accepted
- Date: 2026-07-12

## Context

A successful build tree is not a distributable engine. Release artifacts need a predictable layout,
machine-readable identity, licensing material, and validation that the installed executable works
without relying on paths inside the source checkout.

## Decision

Anvil defines a CMake install tree containing the sandbox runtime under `bin/`, project metadata,
the license, and a generated JSON manifest. The manifest has an explicit schema version and records
the engine version, build configuration, operating system, and architecture.

CPack produces a checksum-bearing ZIP on Windows and compressed tar archive on other platforms.
An install smoke test creates a staged tree, validates the manifest, and runs the installed runtime
through the SDL dummy video driver.

## Consequences

Local and CI builds can exercise the same layout that release automation will publish. The manifest
provides a stable place for future compatibility and content-version metadata. Platform signing,
symbols, redistributable prerequisites, installers, and automated GitHub releases remain separate
hardening work.
