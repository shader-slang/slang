# Slice 185: Plan canonical scalar emission recipes once

## Motivation

Consider an `int2` bitfield extract followed by an integer-to-Boolean cast. Final linked IR owns
the exact `BitfieldExtract` operands and `IntCast` source/result types. Before this slice,
`_validateNVVMFunction` interpreted each instruction in the first admission walk, repeated the
same resolver during SSA validation, and interpreted it for a third time during provider emission.
That made emission a second semantic decision point after capability preflight.

## Proposed solution

Extend `NVVMEmissionPlan` with distinct source-keyed records for UInt64 word construction, numeric
truthiness, CUDA floating remainder, and bitfield recipes. Populate each record at the canonical
first-pass resolver and make later passes consume it directly.

## Change summary

- Added four typed plan record families and shared owned recipe-step storage.
- Recorded exact recipe operands, semantic types, provider steps, and diagnostic identities during
  first-pass preflight.
- Replaced repeated classification in SSA validation and emission with exact plan lookup.
- Preserved the independent deduplicated provider-capability lists and ABI revision 34.

## Concepts and vocabulary

**Recipe step** is one exact generic provider value operation inside a compiler-owned compound
lowering. **Source key** is the canonical linked-IR instruction pointer used to associate one plan
record with one emission site. **Capability list** is deduplicated per overload, unlike the
per-instruction emission plan.

## Process report

`_resolveNVVMUInt64WordConstruction`, `_resolveNVVMNumericTruthiness`,
`_resolveNVVMFloatingRemainderOperation`, and `_resolveNVVMBitfieldOperation` already proved the
complete canonical instruction shape. This slice preserves those resolvers as the single semantic
source of truth and stores their results. The first walk still records every recipe step through
`_requireValueOperation`, so provider support is checked before `createModule`.

The second walk now validates only the operands named by the stored plan, and emission indexes each
family by source once before module creation. It passes the stored recipe to the existing typed
emitters. No source syntax is reconstructed, no alternate IR spelling is admitted, and no
fixture-specific condition exists.

The implementation initially exposed its key invariant in the focused bitfield test: every typed
field was populated, but `source` remained null. The failure occurred before a provider diagnostic
could be produced. Setting `source` at the end of `_resolveNVVMBitfieldOperation`, alongside the
other successfully proven fields, fixes the producer rather than adding a lookup fallback.

The self-review inventory contains the four plan structures, one generic exact-source lookup, four
first-pass append sites, four validation consumers, and four emission indexes/consumers. Every item
survives because removing it restores a repeated resolver call. The migrated resolvers now occur
only at their definitions and first-pass planning sites.

The Release targets build successfully. With the explicit repository-local LLVM 14 provider,
selected NVVM tests pass 437/437 and the permanent NVVM category passes 92/92. Frozen corpus v1
remains 452 workloads/427 healthy references at 418/418/418 O0/O3/both, with zero old-correct
regressions. All-row direct classifications remain 432 correct, three runtime mismatches, and 17
preflight failures per mode. Discovery remains 82 workloads/72 healthy references at 72/72/72;
all-row classifications remain 72 correct, seven infrastructure, one runtime mismatch, and two
preflight failures per mode.
