# Slice 185: Plan canonical scalar emission recipes once

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds.

## Purpose and Observable Result

Make preflight the single semantic-classification owner for canonical UInt64 word construction,
numeric truthiness, floating remainder, and bitfield operations. Operand validation and emission
consume typed, source-keyed plan records without invoking their resolvers again. Existing direct
NVVM results and diagnostics remain unchanged.

## Progress

- [x] 2026-09-03: Inventoried exact resolver inputs, owned fields, and repeated consumers.
- [x] 2026-09-03: Added typed scalar-recipe records to `NVVMEmissionPlan` and recorded them in preflight.
- [x] 2026-09-03: Made operand validation and emission consume the records.
- [x] 2026-09-03: Replayed focused, selected-prefix, category, and both-corpus validation.
- [x] 2026-09-03: Recorded census evidence, durable design notes, and the five-part slice report.

## Surprises and Discoveries

- The first focused replay failed without diagnostics because the bitfield record had a complete
  recipe but no source key. Setting the key in `_resolveNVVMBitfieldOperation`, next to the rest of
  the canonical result, restored the source-to-plan invariant; the focused test then passed.
- Real-provider tests require the explicit repository-local provider directory on this machine.
  Without `SLANG_NVVM_BUILDER_PATH`, failures consistently report E52016 and are infrastructure,
  not compiler regressions.

## Decision Log

- 2026-09-03, Codex: Keep distinct typed records for each recipe family. Their operands and recipe
  steps are canonical family-specific data; a universal payload would recreate an escape API.

## Outcomes and Retrospective

The four scalar recipe families are now classified exactly once. Searches find each resolver only
at its definition and its first-pass preflight call. The Release build, 437/437 selected prefix,
92/92 category, frozen 418/418/418 over 427, and discovery 72/72/72 over 72 all pass. No supported
shape, provider callback, or diagnostic changed.

## Context and Current Pipeline

`_validateNVVMFunction` first calls `_resolveNVVMUInt64WordConstruction`,
`_resolveNVVMNumericTruthiness`, `_resolveNVVMFloatingRemainderOperation`, or
`_resolveNVVMBitfieldOperation` while proving the canonical linked-IR shape. The second validation
walk repeats resolution to discover operands, and `emitNVVMIRFromLinkedIR` repeats it a third time
to select provider recipes. Slice 184 established the source-keyed plan for ordinary value
operations; this slice applies that contract to the four bounded scalar recipe families.

## Scope and Non-Goals

This slice changes no admitted IR shape, provider ABI, CUDA semantics, or GenericAsm handling. It
does not migrate atomics, resources, calls, aggregates, or pointer/addressing classifiers.

## Architecture and Invariants

The first preflight walk is the sole classifier. Each accepted source instruction has exactly one
owned typed record. Recipe steps and operand pointers remain immutable while the linked NVVM-ready
IR is emitted. Capability requirements remain deduplicated separately from per-source records.

## Interfaces and Dependencies

Extend internal structures in `source/slang/slang-emit-nvvm.h`; keep provider ABI revision 34.
Implementation remains in `source/slang/slang-emit-nvvm.cpp`.

## Milestones

1. Add owned plan record types and source lookups.
2. Record each accepted operation during first-pass validation while retaining capability queries.
3. Replace second-pass and emission resolution with plan consumption.
4. Validate focused unit, selected NVVM, permanent category, frozen corpus v1, and discovery corpus.

## Validation and Acceptance

Build and tests run outside the sandbox. Acceptance requires a clean Release build, relevant NVVM
unit tests, the established selected prefix and category totals, frozen v1 O0/O3/both with no
old-correct regression, and unchanged discovery results. A source search must find each migrated
resolver only at its definition and first-pass planning site.

## Failure and Recovery

Plan records are additive internal state. A partial migration can be diagnosed by a missing-source
assertion; revert the consumer for that family while preserving other typed records. No serialized
or external ABI changes require migration.

## Artifacts and Hand-Off

Retain Slice 185 census TSV/JSON artifacts, update the capability ledger/design document, and add
a five-part `report.slice-185-*.md`. The active ExecPlan remains a local working log.
