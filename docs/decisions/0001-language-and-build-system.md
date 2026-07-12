# ADR 0001: C++23 and CMake

- Status: accepted
- Date: 2026-07-12

## Context

The engine needs direct access to native platform and graphics APIs, predictable data layout, broad
tool support, and builds on Windows and Linux.

## Decision

Anvil uses C++23 and target-oriented CMake. Checked-in presets define supported developer and CI
workflows. Compiler extensions are disabled by default, warnings are errors in Anvil-owned code,
and third-party targets do not inherit Anvil warning or analysis policies.

## Consequences

The project accepts the complexity and safety risks of native C++ in exchange for ecosystem reach
and low-level control. Sanitizers, static analysis, assertions, ownership conventions, and tests are
required engineering controls rather than optional cleanup.
