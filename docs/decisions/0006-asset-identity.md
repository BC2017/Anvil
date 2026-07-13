# ADR 0006: Stable, path-independent asset identity

- Status: accepted
- Date: 2026-07-12

## Context

Filesystem paths change when assets are renamed, moved, reorganized, or packaged. Persisting paths
inside scenes and prefabs would break references during ordinary project maintenance and would mix
editor source layout with runtime identity.

## Decision

Each imported asset receives a random 128-bit identifier represented in canonical UUID text form.
Scenes and runtime systems reference that identifier rather than a source path. The asset registry
maintains a bidirectional association between stable IDs and normalized, project-relative logical
paths while rejecting duplicate identities and paths.

Registry lookups return metadata snapshots and support concurrent readers with serialized writes.
All persisted asset metadata will carry explicit schema and source revisions as serialization and
import pipelines are added.

## Consequences

Moving an asset preserves references as long as its metadata retains the same ID. Duplicate or lost
metadata must be treated as an identity conflict rather than silently generating a replacement.
Logical paths remain useful for editor discovery but are not durable runtime references.
