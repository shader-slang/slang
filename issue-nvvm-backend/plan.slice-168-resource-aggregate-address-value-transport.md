# Generalize finite aggregate-array values and module constants

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM transports naturally laid-out fixed arrays of resource values and
materializes finite module-scope aggregate constants at their first executable use. The bounded
target set is discovery `func-resource-result-complex` and `static-const-matrix-array`. The frozen
`cbuffer-float3-offsets-unaligned` row, discovery `type-legalize-bug-1`, and discovery
`buffer-type-splitting` remain rejected because the IR audit proved that their first diagnostics
lead to distinct global-block or physical-layout invariants rather than this value family.

## Progress

- [x] (2026-09-01) Completed and committed Slice 167 as `8d9e6bc3a`; frozen v1 advances to
  402/402/402 and discovery remains 66/66/66 O0/O3/both.
- [x] (2026-09-01) Re-ranked both corpora and selected four healthy workloads for an audit of the
  apparent resource-aggregate lvalue/value boundary.
- [x] (2026-09-01) Captured all four linked-IR shapes and narrowed the slice to the two canonical
  fixed-array value producers. Kept the three layout/global-block rows in their existing clusters.
- [x] (2026-09-01) Added exact natural resource-array value lowering, finite module-constant
  materialization, and executable-value merge parameters using only revision-32 generic builder
  operations. Both retained workloads pass O0 and O3; all three excluded shapes remain rejected.
- [x] (2026-09-01) Promoted four stable runtime lanes; regenerated the separate frozen/discovery
  census and Pareto artifacts; measured both new gates across five configurations; completed the
  design, ledger, report, formatting, build, selected-prefix, self-review, and staged-file audits.

## Surprises and Discoveries

- Frozen `aggregate-pointer-layout-transport` and discovery `aggregate-struct-field-pointer` share
  the same exact first shape: an explicit four-operand pointer to an `RWStructuredBuffer` field.
  Discovery also exposes the analogous `Texture2DMS` field pointer.
- Discovery's `aggregate-storage-layout` workload uses `S s[2]` where `S` contains two
  `RWByteAddressBuffer` fields. Its source-level aggregate may be represented as independently
  flattened bindings; layout compatibility must not be assumed from source syntax.
- Discovery's `resource-array-value-load` workload loads `Texture2D[2]` from the synthesized global
  block before helper/inlining transformations select one element. Existing resource structs are
  first-class values, but resource arrays are currently admitted only at selected storage sites.
- The fixed-array classifier is already recursive. `static-const-matrix-array` reaches accepted
  local `makeArray`, load, store, helper-parameter, and element operations; its diagnostic names
  the module-scope `makeArray` that produces `static const float3x2[2]`. Module validation retains
  that producer as IR, but `_getLoweredNVVMValue` materializes only scalar literals on demand.
- `buffer-type-splitting` has a real layout disagreement. Its canonical `S[2]` stores two raw
  buffer views per `S`; the selected provider representation is 32 bytes per element while the
  CUDA parameter layout is not that pointer/count-pair struct. Treating it as ordinary memory
  would encode the wrong launch ABI.
- The frozen cbuffer row combines an ordinary buffer field with packed constant data in one
  synthesized global block. The all-or-nothing global-block classifier reports the buffer field
  first, but accepting that field alone would merely expose the packed constant-buffer layout.
  `type-legalize-bug-1` analogously combines the output buffer with a parameter block whose
  specialized contents require a broader representation audit.
- The compiler's value map spans function emission. Caching a module-owned aggregate instruction
  after materializing it in one function would let a later function reference foreign SSA. Scalar
  provider constants remain cacheable, but vector/aggregate construction trees must be remade in
  each using function.
- After resource-array loads were admitted, `func-resource-result-complex` exposed a merge-block
  `Texture2D` parameter. This is not a new resource representation: it is the canonical phi form
  for the already-supported executable texture handle, so block parameters now use the complete
  established executable-value classification rather than the copyable-scalar subset.

## Decision Log

- Decision: retain only naturally laid-out resource arrays and finite module aggregate constants
  in Slice 168.
  Rationale: both are canonical fixed aggregate values expressible through the provider's generic
  array, aggregate-construction, load, and extraction operations. The rejected candidates require
  different physical ABI representations and cannot be made valid by widening value transport.
  Date/author: 2026-09-01, Codex.
- Decision: classify resource arrays independently from helper arrays, while reusing the existing
  recursive resource-value algebra for their leaves.
  Rationale: a texture array is an ordinary first-class value in this IR, but it is not a helper
  value under the current helper ABI. Keeping the roles explicit avoids silently widening helper
  signatures while giving value lowering one recursive source of truth.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Both selected discovery workloads now compare correctly at O0 and O3. Frozen corpus v1 remains
exactly 452 workloads/427 healthy references and 402/402/402 O0/O3/both, with no changed row and no
old-correct loss. Discovery remains exactly 82 workloads/72 healthy references and advances from
66/66/66 to 68/68/68, with exactly the two selected gains and no loss. Its direct classifications
are 68 correct, six preflight failures, seven infrastructure failures, and one runtime mismatch in
each mode. The selected prefix passes 433/433, the permanent NVVM category passes 50/50, and the
four promoted lanes pass.

The two-gate exploratory measurement produced ten successful PTX/cubin rows. Direct O3 emits
1,163-byte PTX for the module constant workload and 884-byte PTX for the resource-array workload,
versus 9,519 and 8,823 bytes from NVRTC O3. Every direct O3 module assembled with CUDA 12.9 for
SM70, SM80, and SM90. These three-repetition timings remain observations, not benchmark claims.

The slice establishes two reusable finite-value invariants without a provider change: natural
fixed resource arrays share the existing structural array representation, and immutable
module-owned constructor trees are rematerialized inside each using function. Adjacent packed
global blocks and mismatched resource layouts remain deterministic stops.

## Context and Current Pipeline

The direct backend already transports recursively finite resource structs by value, stores them in
locals, passes/returns them through helpers, extracts fields, indexes selected aggregate arrays,
and addresses resource-capable struct fields in several established roots. Conventional global
storage, parameter groups, local/helper pointers, structured-buffer element pointers, and composed
array/field paths each preserve explicit provenance.

The two retained rows reach adjacent shapes outside those exact contracts: a first-class fixed
array of opaque textures loaded from conventional global storage, and a module-owned tree of
`makeArray`/`makeVector` constants used by a function. Existing generic aggregate construction,
sequential extraction, typed texture handles, and scalar literals already express both trees; the
compiler needs to classify the resource array and recursively materialize the module constant.

## Scope and Non-Goals

In scope are the two retained workload identities; exact `IRLoad`, `IRGetElement`, `IRMakeArray`,
and `IRMakeVector` producers they expose; recursive resource-array value classification; finite
module-constant availability and materialization; generic builder reuse; focused positive/negative
coverage; both fixed corpora; and representative measurement evidence.

Out of scope are the three excluded rows, packed constant-buffer layout, raw-buffer aggregate launch
ABI, arbitrary opaque-resource pointers, dynamic descriptor indexing beyond selected fixed
aggregates, mutable module data, syntax/name reconstruction, compatibility fallback, provider
callbacks without a concrete operation need, and all other census clusters.

## Architecture and Invariants

- Resource arrays are lowered structurally only when their fixed count, natural stride, and every
  recursive leaf have an existing executable resource-value representation.
- A module aggregate constant is accepted only when its exact constructor and complete operand
  tree reduce to selected scalar literals and finite aggregate/vector constructors.
- Module constants are materialized inside each using function and aggregate/vector instructions
  are deliberately not cached in the cross-function value map; no mutable storage or new
  serialized constant ABI is invented.
- Storage layout remains checked against the physical provider representation. A mismatch is not
  patched by changing offsets or reinterpreting flattened bindings in the emitter.
- Existing generic builder operations are preferred. Provider ABI revision 32 remains fixed unless
  a concrete canonical operation cannot be expressed correctly through that interface.

## Interfaces and Dependencies

Classification and type lowering live in `source/slang/slang-emit-nvvm-type-lowering.cpp` and its
header. Producer/value/address resolution, preflight, and emission live in
`source/slang/slang-emit-nvvm.cpp`. Focused fake/real-provider tests belong in the split NVVM unit
files. Permanent runtime lanes belong only on stable existing fixtures. Census, discovery, and
measurement scripts remain unchanged.

## Milestones

1. Preserve linked IR and diagnostics for the four audited workloads at O0/O3. Trace each rejected
   shape from its named producer through the first consumer and record whether it belongs here.
2. Define recursive resource-array and finite module-constant contracts using existing classifiers
   and builder operations. Add focused positive and adjacent-negative tests before widening use.
3. Run the two retained workloads after each invariant change. Follow newly exposed failures only
   while they remain inside this bounded fixed aggregate-value family.
4. Promote stable lanes, run the selected prefix and both exact corpora, refresh the representative
   matrix, complete design/ledger/report records and self-review, format, audit, and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
focused producer/consumer evidence; correct O0/O3 differential execution for every promoted
workload; deterministic rejection of adjacent invalid shapes; zero old-correct regression; frozen
identity 452/427 and discovery identity 82/72; separate census/Pareto artifacts; selected-prefix
success; representative PTX assembly for SM70, SM80, and SM90; formatting attempt; `git diff
--check`; artifact integrity; and an exact staged-file audit excluding `external/slang-binaries/`.

## Failure and Recovery

Changes are additive and independently testable. If a selected source aggregate lowers to
flattened bindings or another non-addressable representation, record that producer contract and
leave it rejected rather than synthesizing storage. If a valid shape needs a materially different
feature, narrow the slice to the proved shared invariant and re-rank the remainder. Never recognize
fixture/type names, walk arbitrary graphs, infer missing layouts, or patch serialized LLVM IR.

## Artifacts and Hand-Off

Keep raw IR, PTX, and logs under ignored `build/nvvm-census` paths. Retain a completed plan only if
the slice yields a committed result under the user's workflow exception. Distill durable resource
aggregate representation rules into `docs/design/nvvm-backend.md`, exact coverage into the ledger
and separate census artifacts, and all producer/input-shape decisions into the Slice 168 report.
