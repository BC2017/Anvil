# ADR 0008: Command-based editor history

- Status: accepted
- Date: 2026-07-12

## Context

Inspector changes, hierarchy operations, asset edits, and viewport gizmos all need consistent undo,
redo, dirty-state tracking, and failure behavior. Implementing ad hoc history in each editor panel
would make multi-tool workflows unpredictable.

## Decision

Editor mutations are represented as commands with apply, undo, and user-facing name operations. A
bounded history owns successfully applied commands, invalidates redo history when edits branch, and
propagates command failures without moving the failing history entry between stacks.

Document cleanliness is tracked through unique state identities rather than stack depth. Returning
to the exact saved state through undo or redo is clean; creating a new edit branch remains modified
even if it happens to produce the same immediate value.

The editor command module lives under `tools/` and may depend on engine libraries. Runtime engine
modules must not depend on editor history or other tooling code.

## Consequences

Commands must keep enough state to reverse their own completed mutation and should make apply and
undo atomic where possible. Composite transactions and command coalescing will build on this core
when continuous gizmo and multi-selection editing are introduced.
