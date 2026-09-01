# Canonical parameter-group storage and access layout

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM uses one canonical physical representation for selected constant-
buffer/parameter-block storage whose retained target layout differs from ordinary LLVM natural
layout. Field addressing, loads, pointer transport, type lowering, and launch ABI validation must
consume the same representation.

The bounded primary probes are frozen `hlsl/cbuffer-float3-offsets-aligned` and
`hlsl/cbuffer-float3-offsets-unaligned`; they jointly expose a compiled runtime mismatch and a
preflight pointer stop around HLSL cbuffer packing. Discovery `bugs/type-legalize-bug-1` is an
adjacent parameter-block probe: carry it only while it shares the same canonical storage invariant,
and retain its exact next blocker otherwise. Existing permanent `compute/cbuffer-legalize` and
`bindings/nested-parameter-block-{2,3}` lanes are regression gates.

## Progress

- [x] (2026-09-01) Completed and committed Slice 170 as `b3e5ae893`; frozen v1 remains
  407/407/407 over 427 and discovery advances to 69/69/69 over 72.
- [x] (2026-09-01) Re-ranked both healthy-failure sets. Constant-buffer layout is the strongest
  in-MVP representation pair: one frozen runtime mismatch and one frozen preflight failure share
  parameter-group storage and field-address production.
- [x] (2026-09-01) Preserved final IR, retained layouts, native launch layout, direct provider
  types, PTX access offsets, and runtime outputs for both cbuffer probes in O0 and O3.
- [x] (2026-09-01) Identified the exact producer/consumer invariant. Implemented one reusable
  physical storage representation that explains both failures and preserves existing parameter-
  group gates.
- [x] (2026-09-01) Carried both bounded probes to correctness and promoted their stable exact O0/O3
  differential lanes. Discovery `type-legalize-bug-1` retains its unrelated `ParameterBlock<B>`
  conventional-global-field blocker.
- [x] (2026-09-01) Regenerated both exact corpora and bounded measurements; documented, validated,
  self-reviewed, and prepared Slice 171 for commit.

## Surprises and Discoveries

- Slice 170 advances the unaligned probe past its output buffer. Its first failure is now the exact
  `Ptr<cbuffer<SLANG_ParameterGroup_Constants>, ..., layout=ScalarLayout>` conventional-global
  field address.
- The aligned sibling already compiles in both direct modes but its compare-compute output differs
  from native CUDA. This makes admission alone insufficient: retained byte offsets and the provider
  load path must be audited together.
- The aligned provider pointee used `[2 x <3 x float>]`, giving LLVM's 16-byte vector stride where
  CUDA requires 12. Fixed numeric arrays were the only parameter-group type family that bypassed
  recursive storage lowering.
- CUDA's canonical layout gives `half3` and `half4` size eight and alignment four. A scalar Half
  array cannot express both facts, while `[2 x <2 x half>]` does so exactly through existing generic
  vector and array types.
- Requiring every parameter group to validate against its retained semantic layout graph regressed
  four old-correct existential/packing workloads because legalization intentionally changes the
  pointee graph without making that metadata structurally isomorphic. The six-row revert drill
  proved the Slice 171 representation needs only the canonical CUDA layout query; the over-broad
  retained-graph requirement was removed.

## Decision Log

- Decision: prioritize the paired constant-buffer failures ahead of unrelated single-operation or
  out-of-MVP FP8/BFloat16 gaps.
  Rationale: constant buffers and parameter blocks are explicit usable-compute MVP requirements;
  the pair can prove both compilation and runtime layout correctness through real launch data.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32 unless a concrete canonical storage type or pointer
  operation cannot be expressed through existing structs, arrays, field GEPs, loads, and bit
  transport.
  Rationale: current evidence points to compiler-side physical type/layout selection.
  Date/author: 2026-09-01, Codex.
- Decision: represent compact Half parameter-group vectors as two native `half2` chunks and
  three-lane 32-bit vectors as scalar arrays; recurse fixed arrays through that same algebra.
  Rationale: these exact provider types reproduce CUDA's complete sizes and alignments while the
  existing aggregate extraction and vector construction operations recover ordinary SSA values.
  Date/author: 2026-09-01, Codex.
- Decision: validate this boundary with the canonical CUDA layout query rather than requiring the
  retained semantic layout graph to be structurally identical to the legalized pointee.
  Rationale: the query is the producer-owned CUDA ABI for the lowered type; a mandatory structural
  retained-layout walk caused four demonstrated old-correct regressions and was not needed for the
  two compact-vector layouts.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Slice 171 unlocks both frozen cbuffer probes in O0 and O3 and promotes four permanent lanes. Frozen
v1 remains exactly 452/427 and advances from 407/407/407 to 409/409/409 with exactly two gains and
zero old-correct regression. Discovery remains exactly 82/72 and 69/69/69 with no changed row.

The selected prefix passes 433/433 and the permanent `nvvm` category passes 66/66. Both bounded
measurement gates compile and assemble with CUDA 12.9 for native NVRTC, direct O0 SM70, and direct
O3 SM70/SM80/SM90. Provider ABI revision 32 remains unchanged. Self-review removed the attempted
global retained-layout requirement after its exact four-row regression audit.

## Context and Current Pipeline

CUDA target layout and global-parameter collection preserve parameter-group byte layout in
`IRParameterGroupTypeLayout`, `IRStructTypeLayout`, and keyed `IRVarLayout` entries. Direct NVVM
currently lowers the parameter-group pointee through `NVVMTypeUse::ParameterGroupStorage`, with a
special compact representation for selected three-lane 32-bit vectors, and loads values through
generic provider pointers. The bounded fixtures mix Float32, Half, and vector fields at HLSL
constant-buffer offsets that may not equal ordinary LLVM aggregate offsets.

## Scope and Non-Goals

In scope are retained parameter-group layout metadata, finite selected scalar/vector fields,
explicit padding represented as a principled physical type when required, conventional-global
parameter-group fields, nested parameter groups already supported by permanent gates, existing
generic provider aggregate/pointer/load operations, the two fixed corpora, and bounded
measurements.

Out of scope are fixture-name checks, source-syntax reconstruction, arbitrary packing guesses,
compatibility fallbacks, accepting malformed layout graphs, dynamic-interface payload redesign,
FP8/BFloat16, provider callbacks without an operation gap, serialized IR patches, and external
workloads.

## Architecture and Invariants

- The canonical CUDA target-layout producer is the launch/storage ABI source of truth; provider
  offsets must match its complete size/alignment result before any pointer or load is emitted.
- One physical parameter-group representation determines type lowering, field addressing, compact
  storage/value conversion, size/alignment validation, and loads.
- Struct layout entries are matched by semantic key, never declaration position.
- Padding may exist only as an explicit reusable physical representation derived from canonical
  byte intervals. Downstream code may not patch an offset or infer packing from a fixture.
- Unsupported or non-finite layouts stop deterministically before provider mutation.

## Interfaces and Dependencies

Parameter-group and aggregate classification/type lowering live in
`source/slang/slang-emit-nvvm-type-lowering.cpp`; preflight, address/value resolution, and emission
live in `source/slang/slang-emit-nvvm.cpp`. Existing revision-32 generic provider operations should
be sufficient. Real repository fixtures own differential ABI proof; focused fake-provider tests
are added only for a new non-redundant representation invariant.

## Milestones

1. Dump and compare final IR/layout/PTX/runtime evidence for aligned and unaligned cbuffers.
2. Trace target-layout producer, physical type lowering, field pointer, load, and value conversion.
3. Implement the smallest shared representation and retain strict adjacent negatives.
4. Promote exact successes; run build, focused tests, selected prefix, permanent category, both
   exact corpora, and SM70/SM80/SM90 measurements.
5. Update design, ledger, five-part report, and plan; format, audit, stage exactly the slice files
   excluding `external/slang-binaries/`, and commit.

## Validation and Acceptance

All builds/tests run outside the sandbox with Windows-native tools and the isolated Release
provider. Acceptance requires exact corpus identities 452/427 and 82/72; O0/O3 differential
results; zero old-correct regression; selected-prefix and permanent-category success; retained
diagnostic ownership for unsuccessful probes; PTX assembly for promoted gates; formatting;
artifact integrity; and an exact staged-file audit.

## Failure and Recovery

If retained layout requires a provider type that cannot be expressed through existing generic
operations, stop and record the concrete ABI gap before revising the provider. If the aligned
runtime mismatch is a harness/reference issue rather than the same storage representation, keep it
separate. Never reinterpret a host launch block with incompatible LLVM offsets, erase layout
metadata, or patch emitted text.

## Artifacts and Hand-Off

Keep dumps, PTX, and logs under ignored `build/nvvm-census` paths. Retain the completed plan only
with a committed result under the user's workflow exception. Distill durable parameter-group
storage rules into `docs/design/nvvm-backend.md`, exact status into the capability ledger and
separate corpus artifacts, and every producer/input-shape decision into the Slice 171 five-part
report.
