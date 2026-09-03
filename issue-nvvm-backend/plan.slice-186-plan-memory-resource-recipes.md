# Slice 186: Plan canonical memory and resource operations once

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds.

## Purpose and Observable Result

Make preflight own the typed decisions for ordinary atomics, explicit image surface operations,
default resource values, and ephemeral values. Later validation and emission consume stable plan
records without reclassifying these accepted instructions.

## Progress

- [x] 2026-09-03: Audited exact producer shapes and owned resolver data.
- [x] 2026-09-03: Added and populated typed memory/resource plan variants.
- [x] 2026-09-03: Replaced later resolver calls with exact plan lookup.
- [x] 2026-09-03: Validated and recorded both corpus baselines.
- [x] 2026-09-03: Completed durable documentation and prepared the slice commit.

## Surprises and Discoveries

- Ordinary atomic pointer-consumer validation was a fourth hidden semantic consumer. Passing the
  immutable requirements into `_validatePointerValue` let it use the already planned atomic
  operand instead of re-running the atomic resolver.
- The per-family emitter indexes had identical checked construction. One typed template now owns
  the source-key uniqueness invariant for all nine planned families.

## Decision Log

- 2026-09-03, Codex: Migrate source instructions whose complete semantics are already represented
  by typed resolver results. GenericAsm helper recipes remain outside this slice because their
  source identity and call-site contracts need a separate producer-side design.

## Outcomes and Retrospective

Ephemeral values, default resources, explicit image surfaces, and ordinary atomics are classified
only in first-pass preflight. Focused tests, the Release build, 437/437 selected prefix, 92/92
category, frozen 418/418/418, and discovery 72/72/72 pass without an ABI or coverage change.

## Context and Current Pipeline

Canonical atomic IR instructions, PTX-legalized `ImageLoad`/`ImageStore`, optional-resource
`DefaultConstruct`, and ephemeral markers are accepted by the first `_validateNVVMFunction` walk.
The same resolver is then called by operand validation and again during emission. This duplicates
decisions after capability validation and leaves emission coupled to classification.

## Scope and Non-Goals

Do not widen atomic types/orders/scopes, surface layouts, default resource leaves, or ephemeral
shapes. Do not change byte-address GenericAsm atomics, atomic-reduction helpers, texture helpers,
wave recipes, provider callbacks, or diagnostics.

## Architecture and Invariants

One source instruction maps to one family-specific record. Records own provider descriptors,
recipe data, operand pointers, and the diagnostic selected by the canonical resolver. Preflight
queries all provider capabilities before module creation; emission only executes recorded choices.

## Interfaces and Dependencies

Extend internal `NVVMEmissionPlan` storage and lookup helpers. The existing generic builder API and
provider ABI revision 34 are sufficient.

## Milestones

1. Introduce owned ephemeral/default-resource/surface/atomic records.
2. Populate them during the first validation walk.
3. Consume them during SSA validation and emission.
4. Run focused and corpus validation; document exact producer ownership.

## Validation and Acceptance

Build/tests run outside the sandbox. Require focused NVVM tests, selected prefix, permanent NVVM
category, frozen v1, and discovery census. Searches must show no migrated resolver calls in the
second validation pass or emitter.

## Failure and Recovery

Missing records fail with release assertions before provider mutation. Each family can be reverted
independently because no provider or serialized contract changes.

## Artifacts and Hand-Off

Retain Slice 186 census evidence and the durable five-part report/design updates. Keep this active
plan uncommitted.
