# Separate structured-buffer storage from semantic aggregate values

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires this experimental branch's slice plan to be committed with its implementation, which is
an exception to the repository's default active-plan lifetime policy.

## Purpose and Observable Result

After this slice, direct NVVM represents selected structured-buffer elements with the exact CUDA
external storage layout even when their ordinary LLVM value representation has a different size,
alignment, or field shape. Loads reconstruct the canonical semantic value; stores decompose it
back into physical storage. One recursive compiler-owned conversion covers fixed arrays,
three-lane vectors, Boolean scalar/vectors, ordinary finite structs, and established physical
matrix wrappers.

The bounded target population is five healthy-MVP workloads that share this representation gap:
`bugs/gh-8121.slang`, `cuda/make-matrix.slang`, `bugs/gh-7441.slang`, and
`compute/dynamic-dispatch-{16,17}.slang`. Every workload that becomes correct at direct O0 and O3
is promoted. Later independent blockers remain measured rather than broadening this slice.

## Progress

- [x] (2026-08-30) Committed Slice 142 as `210172794` with three both-mode gains, zero losses, and
  a final 421/421 selected prefix.
- [x] (2026-08-30) Decomposed all twelve healthy-MVP aggregate/pointer/layout rows by first shape.
  Five field-address rows have four distinct producer families; the three explicit
  structured-buffer layout rows join two conventional-global resource-field rows in one larger
  external storage-representation cohort.
- [x] (2026-08-30) Traced representative final IR. The resource view retains its canonical
  semantic element type while `slang-emit-nvvm-type-lowering.cpp::_lowerRawBufferType` currently
  lowers the data pointer using `NVVMTypeUse::Value`; validation therefore rejects any CUDA/LLVM
  layout mismatch before emission.
- [x] (2026-08-30) Defined one bounded recursive structured-buffer storage algebra and exact
  pointer-stride/field-offset proof for the five measured element families.
- [x] (2026-08-30) Lowered raw structured-buffer data and element pointers to the physical storage
  type; added
  shared preflight and emission recipes that convert storage values to/from canonical semantic
  values through existing generic builder operations.
- [x] (2026-08-30) Updated focused fake graph expectations, retained the adjacent incompatible
  layout negative, promoted five real differential workloads, regenerated the fixed census/Pareto
  and representative metrics, and completed the self-review and final validation.

## Surprises and Discoveries

- The top-level `aggregate-pointer-layout-transport` cluster is intentionally coarse. Its twelve
  healthy rows contain two entry parameters, two global parameters, five field addresses, and
  three structured-buffer layout failures. The five field addresses do not share one base
  producer, so admitting every field address would erase important ownership distinctions.
- `const-ref.slang` reaches `get_field_addr` from an exact Slice 142
  `BorrowInParam<Thing, Read, Generic, DefaultLayout>`. That is one valid helper-reference consumer,
  but it represents only one row and is not the largest remaining aggregate invariant.
- `groupshared-struct-with-interface.slang` addresses fields of a module-scope groupshared struct;
  `cbuffer-float3-offsets-unaligned.slang` addresses nested parameter-group storage. They require
  shared-aggregate and constant-buffer representations respectively and stay outside this slice.
- The structured-buffer cohort includes two diagnostics reported as `struct field address`.
  Conventional global collection first addresses the resource view field; admission rejects its
  aggregate element representation before any data access. Those rows have the same root cause as
  the three explicit `structured-buffer element layout` failures.
- Existing generic provider operations can construct/extract aggregates and vectors, compare and
  convert selected integers, and load/store typed pointers. No concrete provider callback gap is
  visible; ABI revision 29 remains unchanged.
- The provider and CUDA agree on the size and field offsets of established
  `Thing { uint, float, half4 }` storage but report preferred root alignments of eight and four.
  Pointer stride and field offsets are the physical facts; each load/store carries the conservative
  CUDA alignment explicitly. Requiring equal preferred root alignment would reject valid storage.
- Matrix legalization's final authoritative census shape is a `PhysicalType` wrapper around an
  explicit-stride array of compact three-lane vectors. Its canonical `MakeArray` producer must
  cross the vector value/storage boundary before the wrapper is constructed; the wrapper itself
  is already the physical value.
- The fake provider intentionally collapses all integer widths into one fake scalar handle, so it
  cannot faithfully distinguish i1 semantic Boolean values from i8 external storage. The existing
  fake aggregate graph proves recursive reconstruction, while promoted `gh-7441` runtime lanes
  provide the authoritative byte-Boolean and bool-vector coverage.
- A first full-census pass exposed an old-correct `StructuredBuffer<MyImpl>` where `MyImpl`
  contains a texture handle. That is a valid, established ordinary-value representation family,
  not part of the new numeric/Boolean storage algebra. Explicit type-use classification restored
  it, and the representative bindless-texture gate proves the distinction.

## Decision Log

- Decision: Target the five-row structured-buffer physical storage cohort rather than all five
  field-address diagnostics.
  Rationale: The diagnostic label groups four producer families, while external buffer storage is
  one representation boundary that blocks five real workloads across two labels. This answers the
  guiding question with the largest coherent invariant.
  Date/author: 2026-08-30, Codex.
- Decision: Preserve canonical semantic IR types and introduce a compiler-owned physical provider
  representation at the external storage boundary.
  Rationale: The upstream producer is correct: a structured-buffer load returns the declared
  Slang element value. CUDA storage layout is target-specific and belongs to lowering. Rewriting
  the semantic IR type or pretending its LLVM value layout matches CUDA would break aggregate
  equality and pointer stride.
  Date/author: 2026-08-30, Codex.
- Decision: Reuse generic builder operations and revise ABI 29 only if a focused prototype proves
  one conversion primitive cannot be expressed.
  Rationale: Arrays/structs use aggregate extract/construct, vectors use lane extract/construct,
  and Boolean storage can use UInt8 conversion plus nonzero comparison. The provider already owns
  those typed operations.
  Date/author: 2026-08-30, Codex.
- Decision: Compare final storage size and every nested field/array offset, while using the
  canonical CUDA alignment on memory operations instead of requiring equal preferred root
  alignment.
  Rationale: Provider-preferred aggregate alignment may be stronger without changing pointer
  stride or any addressable byte. The retained `Thing` fake graph and promoted runtime workloads
  prove this exact case.
  Date/author: 2026-08-30, Codex.
- Decision: Keep resource-containing structured-buffer elements on their established ordinary
  value representation and select the new storage use only for the finite numeric/Boolean family.
  Rationale: `collectGlobalUniformParameters` and resource lowering intentionally preserve
  resource handles in a structured-buffer element. The representative bindless-texture workload
  was correct before this slice and proves that this valid family must remain distinct.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

All five bounded workloads become correct at direct O0 and O3 and receive promoted lanes:
`bugs/gh-7441`, `bugs/gh-8121`, `compute/dynamic-dispatch-{16,17}`, and `cuda/make-matrix`.
The fixed 452-workload census gains exactly those five identities in both modes with no old-correct
loss: O0 reaches 346 correct and O3 reaches 351. Against 427 healthy MVP references,
O0/O3/both correctness is 344/348/344 (80.6%/81.5%/80.6%). `bugs/gh-5776` additionally advances
from its helper-signature stop to a later canonical struct-field-address stop and remains a
measured failure.

The admitted storage algebra is recursive finite numeric/Boolean scalar/vector, fixed array, and
nonempty struct storage plus the established one-field physical array wrapper. Boolean leaves are
i8; numeric vector3 and bool3 are scalar arrays; bool2/bool4 use naturally aligned i8 vectors.
Every explicit array stride and direct struct offset is proven against CUDA. Resource-containing
elements retain their established ordinary-value representation. Provider ABI revision 29 is
sufficient and unchanged.

The leading healthy-MVP blockers are now preflight-other (11), residual target markers (10),
atomic/wave operations (10), and a three-way tie at eight for aggregate/pointer/layout transport,
helper ABI, and wave/reconvergence GenericAsm. The original aggregate cluster falls from twelve to
eight because one unrelated helper row exposes a later struct-field blocker. Future prioritization
should decompose these tied populations by canonical producer before selecting the next slice.

## Context and Current Pipeline

Consider:

```slang
struct Payload
{
    float3 value;
    uint flags;
}

StructuredBuffer<Payload> input;
Payload item = input.Load(0);
```

`collectGlobalUniformParameters` stores the canonical `StructuredBuffer<Payload>` view in the
synthesized `GlobalParams` block. Resource lowering produces a typed structured-buffer load whose
result is the canonical `Payload` semantic value. `getNVVMSupportedRawBufferType` records that
semantic element type in `NVVMRawBufferType::structuredElementType`.

Before this slice, `NVVMTypeLoweringContext::_lowerRawBufferType` lowered that element with
`NVVMTypeUse::Value` and built a global pointer to the result. For `float3`, the provider's
ordinary LLVM vector layout is not the CUDA external storage layout. Similarly, LLVM i1 does not
represent CUDA's one-byte Boolean storage. `_hasNVVMCompatibleRawBufferElementLayout` detects the
mismatch and reports `structured-buffer element layout`; a conventional-global resource field can
fail earlier when `_getNVVMStructFieldAddress` asks whether the view itself is supported.

The direct backend already had a separate `NVVMTypeUse::Storage` map and compact three-lane array
representation for parameter groups. Structured-buffer loads and stores did not use it, and there
was no general recursive storage/value conversion. This slice makes that distinction an explicit
reusable external-buffer contract.

## Scope and Non-Goals

In scope:

- exact structured-buffer elements composed recursively from selected integer/floating leaves,
  Boolean scalar/vectors, fixed arrays, finite structs, compact three-lane numeric vectors, and
  existing one-field physical array structs;
- a physical type chosen solely from canonical element type plus CUDA layout rules;
- exact pointer-stride, explicit-array-stride, and struct-field-offset validation against the
  provider representation before module construction, with CUDA alignment carried by each memory
  operation;
- recursive storage-to-value and value-to-storage conversion shared by structured-buffer direct
  loads and writable element-pointer loads/stores;
- existing focused fake graph expectations, real differential, negative incompatible-layout, and
  representative `ptxas` tests;
- five-row O0/O3 probe, exact promotions, fixed census/Pareto, and representative metrics.

Out of scope:

- constant/parameter-block layout widening, groupshared structs, borrowed-helper field addressing,
  raw entry-point aggregate ABI, existential global parameters, or arbitrary field-address
  admission;
- runtime-sized arrays, recursive structs, explicit strides that cannot be represented exactly,
  new resource/pointer external-storage representations beyond the established ordinary-value
  family, BFloat16/FP8/cooperative matrices, or layout guessing from source syntax;
- changing upstream semantic aggregate identity, reconstructing syntax, fixture checks,
  byte-copy fallbacks, downstream patches for malformed IR, or provider ABI revision 30 without a
  demonstrated operation gap.

## Architecture and Invariants

- The final structured-buffer element type remains the semantic source of truth. The compiler
  derives, but never substitutes into canonical IR, one provider storage representation.
- Storage classification is finite and recursive. Every array count and struct field is explicit;
  cycles, unsized aggregates, unsupported leaves, and unrepresentable explicit strides fail
  preflight.
- Physical layout is proven, not assumed. Provider pointer stride, every explicit array stride,
  and every direct struct field offset must equal CUDA layout. Memory operations carry CUDA's
  explicit conservative alignment, so a stronger provider-preferred root alignment is allowed
  only when it changes no addressable byte. A conversion recipe cannot make an incompatible
  pointer stride legal.
- Raw view data pointers and writable element pointers use the physical storage type. Ordinary SSA,
  helper parameters/results, locals, phis, calls, and aggregate operations continue using the
  canonical value type.
- A load converts physical storage to the exact semantic result; a store performs the inverse.
  Recursive struct/array field order comes from canonical field keys/declarations, never source
  reconstruction or positional witness data.
- Boolean storage is UInt8 with load-side `!= 0` and store-side explicit integer conversion.
  Three-lane numeric storage is a three-element scalar array. Other selected leaves retain their
  proven natural representation.
- Preflight records the complete generic operation closure before provider discovery. Emission
  consumes the same compiler-owned recipe and cannot opportunistically fall back.

## Interfaces and Dependencies

Expected compiler changes are concentrated in
`source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` and
`source/slang/slang-emit-nvvm.cpp`. A small descriptor may expose the physical storage family and
recursive layout facts to both validation and emission. The shared semantic catalog changes only
if Boolean storage conversion proves an already-implemented generic operation is not admitted.

The forward-only builder interface in `source/compiler-core/slang-nvvm-ir-builder.h` and the
isolated LLVM provider should remain at ABI revision 29. A new callback is justified only by a
focused canonical operation that cannot be composed from existing type, aggregate, vector,
comparison, conversion, load, store, and pointer operations.

Existing focused sources and fake graph assertions live in
`tools/slang-unit-test/unit-test-nvvm-support.h` and `unit-test-nvvm-emitter.cpp`. Promoted corpus
lanes provide the authoritative real differential coverage beside their native CUDA directives;
representative `ptxas` coverage remains in the MVP metrics workflow.

## Milestones

1. Inventory the five final element types, operations, and CUDA/provider layout deltas. Add a
   focused failing source for one compact-vector struct, one Boolean aggregate, one fixed array,
   and one physical matrix wrapper.
2. Define the exact recursive structured-buffer storage classifier and layout calculator. Prove
   the five shapes before broadening any type-lowering use.
3. Lower raw structured-buffer view data pointers and writable element pointers to storage types.
   Keep pointer representation caches distinct from ordinary semantic-value pointers.
4. Add one recursive storage/value conversion recipe used by direct structured-buffer loads and
   pointer-based loads/stores. Collect every required Boolean conversion/comparison operation
   before provider mutation.
5. Build host/provider and run focused fake/real/negative tests. Probe all five rows at O0/O3;
   record later blockers and narrow any shape that fails exact layout or provider validation.
6. Promote exact successes, regenerate fixed census/Pareto and representative metrics, self-review
   every classifier/conversion/layout branch, format, validate the selected prefix, document, and
   commit.

## Validation and Acceptance

All builds and tests run outside the sandbox. Acceptance requires focused positive physical-type
and recursive-conversion graph tests; deterministic rejection for an incompatible explicit stride
or unsupported leaf; O0/O3 real differential runtime and `ptxas`; all newly promoted CUDA lanes;
an exact five-row probe; the fixed 452-workload census with zero old-correct regression;
representative metrics and direct O3 SM70/80/90 assembly; Release host/provider builds; the
complete selected prefix; pinned formatting; and `git.exe diff --check`.

## Failure and Recovery

If a type becomes physically representable but reaches an unrelated operation, record that later
producer and leave it for its owning cluster. If CUDA/provider field offsets cannot be made equal
with the bounded storage leaves, reject the type rather than inserting guessed padding. If a
conversion fails provider verification, inspect the typed aggregate graph and physical pointer
type; do not reinterpret raw bytes or weaken the layout proof. All raw probes remain under ignored
`build/nvvm-census/slice143-*` and are safe to regenerate.

## Self-Review

Inventory every new classifier, layout branch, pointer-representation key, recursive conversion
case, and semantic-catalog widening. For each, record the exact canonical input, producer, why it
is valid, and which test fails without it. Perform a revert drill on the storage/value conversion
and physical pointer lowering. Reject source/fixture names, syntax reconstruction, duplicate
aggregate representations, silent padding guesses, compatibility fallbacks, arbitrary operand
walks, and any shape admitted only because it moves a diagnostic.

## Artifacts and Hand-Off

Commit the completed plan with implementation, promoted fixtures, fixed Slice 143 census TSV and
cluster JSON, five-part report, and durable design/capability updates. Keep raw IR, PTX, cubins,
focused probes, and metrics below `build/nvvm-census/`. The hand-off must re-rank aggregate/layout,
preflight-other, atomic/wave, residual-marker, helper-ABI, and wave/reconvergence clusters by
healthy-MVP impact.
