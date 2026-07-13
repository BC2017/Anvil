# ADR 0007: Physically based rendering data contracts

- Status: accepted
- Date: 2026-07-12

## Context

Materials and lights authored without explicit color spaces, units, or valid ranges lead to
renderer-specific interpretation, unstable shaders, and assets that cannot move between tools.

## Decision

Anvil's render-facing material colors are linear. Metallic and opacity values use the closed zero
to one range, perceptual roughness has a small nonzero floor, and emissive color supports HDR values
with intensity expressed in nits.

Directional-light intensity is illuminance in lux. Point and spot light intensity is luminous flux
in lumens, distances are meters, and spot angles are radians. Directions are normalized before
render extraction. Non-finite and invalid authoring data is corrected deterministically while a
bitmask records every correction for editor diagnostics.

## Consequences

Importers must convert source color spaces and units into these contracts. GPU packing may use a
different representation, but it must derive from normalized engine data. Silent NaN propagation
and backend-specific light units are not permitted.
