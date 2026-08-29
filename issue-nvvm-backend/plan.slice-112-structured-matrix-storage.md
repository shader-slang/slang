# Carry layout-lowered matrices through structured buffers

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM consumes the existing LLVM buffer-storage pass's canonical physical
representation for selected row-major matrices nested directly in structured buffers. The existing
`tests/compute/structured-buffer-of-matrices.slang` and
`tests/compute/matrix-layout-structured-buffer.slang` fixtures gain direct runtime and PTX lanes
only when their unchanged outputs and documented 64-/48-byte element strides pass.

## Progress

- [x] (2026-08-29) Probed the two existing structured-matrix fixtures after Slice 111 and recorded
  their shared first stop at the conventional global resource field.
- [x] (2026-08-29) Instrumented the early LLVM storage pass and proved that it already discovers
  each structured-buffer matrix and produces the canonical physical resource element before the
  final direct-NVVM preflight.
- [x] (2026-08-29) Generalized the selected physical-array struct, field-address, layout, and
  sequential-pointer contracts without a matrix-shaped builder callback or source-type inference.
- [x] (2026-08-29) Added the exact bit-reinterpret operation exposed as the next real IR stop,
  focused producer/consumer coverage, and direct runtime/PTX lanes for both fixtures.
- [x] (2026-08-29) Formatted, rebuilt the Release host/provider targets, passed focused and fixture
  validation, assembled both PTX modules, passed the complete NVVM prefix 381/381, and updated
  durable status.
- [x] (2026-08-29) Completed the representation-layer self-review and `git diff --check`; the
  slice is ready for its requested commit.

## Surprises and Discoveries

- Both initial row-major probes stopped at E52017 `conventional global parameter field address`.
  That surface diagnostic did not reveal whether the earlier storage pass had already rewritten
  the resource element, so producer instrumentation was required before assigning ownership.
- The direct pipeline invokes `legalizeMatrixTypes`, but that pass does not rewrite matrix types
  nested in HLSL structured-buffer resource types. The earlier LLVM buffer-storage pass is the
  existing producer that can translate those elements while the logical matrix is still visible.
- Temporary instrumentation at `lowerBufferElementTypeToStorageType` showed that the early LLVM
  pass does mutate both resources. The final IR uses
  `RWStructuredBuffer<[PhysicalType] struct { Array<Vec<T,C>,R> }>` followed by resource element,
  struct-field, dynamic array-row, and dynamic vector-lane addresses. The original diagnostic was
  therefore an emitter classification gap, not a producer or pass-order bug; the policy prototypes
  were reverted completely.
- The first operation after admitting that storage graph was the ordinary CUDA GenericAsm for
  `asint(float)`. The semantic catalog had no exact reinterpretation operation. Forward-only ABI
  revision 19 adds one typed bit-reinterpret operation; the provider uses LLVM `bitcast`, or the
  identity value when signed and unsigned descriptors share the same signless LLVM type.
- The initial fake-provider failure was its own model gap: its struct-field callback enumerated
  parameter, local, and load roots, but not the newly valid resource element pointer. Reusing its
  existing pointee-type resolver made the fake accept any modeled pointer to the selected scalar
  struct and removed those positional root special cases.

## Decision Log

- Decision: make Slice 112 a structured-buffer storage-lowering slice, not a direct matrix-resource
  emitter slice.
  Rationale: the final matrix resource is not a valid LLVM value/pointee representation, while the
  shared storage pass already owns physical element layout and pack/unpack conversion. The emitter
  should receive an explicit physical aggregate and ordinary scalar/vector/array operations.
  Date/author: 2026-08-29, Codex.
- Decision: keep the existing early LLVM storage producer unchanged.
  Rationale: instrumentation proved it already emits the intended physical type. Changing its
  policy would double-own an emitter classification issue and the experimental overrides did not
  describe the actual failing boundary.
  Date/author: 2026-08-29, Codex.
- Decision: add bit reinterpretation as a generic typed value operation rather than recognizing
  `asint` in the emitter or provider.
  Rationale: CUDA GenericAsm supplies the exact overload descriptor, while equal width/lane count
  and different integer/floating kinds define the reusable semantic contract. The same API covers
  `asint`, `asuint`, and `asfloat` without source spelling in LLVM emission.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Implementation is complete. The direct emitter consumes the storage pass's physical wrapper and
threads immutable ownership through nested array/vector element pointers. Both existing fixture
runtime lanes and PTX checks pass with their unchanged 64-byte and 48-byte strides. CUDA 12.9.86
`ptxas -arch=sm_70` accepts the 971-byte and 2,017-byte PTX modules and emits 2,920-byte and
3,304-byte cubins. Release host/provider builds and focused ABI/emitter tests pass, and the complete
NVVM unit prefix passes 381/381.

The final helper/special-case inventory retained four principled changes. The physical-array
classifier consumes only the storage producer's `[PhysicalType]` plus sole fixed-array invariant.
The resource field branch accepts only an exact `RWStructuredBuffer` element pointer to that type.
The recursive sequential-pointer helper replaces two narrower helpers and carries existing
pointer type/layout/access facts rather than rediscovering a source matrix. Typed bit
reinterpretation is a generic descriptor relation and does not inspect GenericAsm text in the
provider. The fake-provider field callback was simplified to its existing pointee-type source of
truth instead of adding another root-kind case. No new AST/IR equivalence, syntax reconstruction,
fallback, producer mutation, or matrix/layout-name check remains. A focused negative proves that a
logical matrix-element write stops before provider discovery, while read-only logical access
passes through the physical resource graph.

## Context and Current Pipeline

Slice 111 extended the established CPU-via-LLVM early storage lowering to the direct route before
`legalizeMatrixTypes`. That fixed constant-buffer matrix layout because the storage pass produced a
sole-array `[PhysicalType]` wrapper and an unpack graph. Structured-buffer matrix element types,
however, remain nested in `IRHLSLStructuredBufferTypeBase`; general matrix legalization deliberately
does not rebuild those resource types.

`lowerBufferElementTypeToStorageType` already discovers HLSL structured buffers and has an LLVM
policy whose `shouldLowerMatrixType` always returns true. Instrumentation established that it
already mutates both motivating resources. This slice leaves that producer unchanged and admits
only its resulting canonical physical aggregate through existing resource view, typed pointer,
load, field extraction, fixed-array, and sequential-value machinery.

## Scope and Non-Goals

In scope are direct `RWStructuredBuffer` elements that lower to one selected physical aggregate
containing a fixed array of selected numeric vectors; Float32 and 32-bit integer row-major matrix
fixtures; read-only logical matrix access through an otherwise read-write view;
dynamic resource element, row, and lane indices; and exact existing CUDA element strides.

Out of scope are emitter-side matrix types, arbitrary nested structs containing matrices,
column-major or non-square shapes whose physical stride is not exactly representable by LLVM,
writing a logical matrix back through a structured buffer, non-32-bit elements, append/consume
buffers, general padded aggregates, and changing fixture data or expectations.

## Architecture and Invariants

- `lowerBufferElementTypeToStorageType` remains the sole owner of logical/physical matrix storage
  conversion and stride selection.
- Matrix resource types must not reach direct preflight; the emitter admits only canonical physical
  aggregate, array, vector, scalar, resource-view, and pointer relations.
- A reusable physical-array aggregate classifier is structural: `[PhysicalType]`, exactly one
  selected fixed-array field, and an implicit or exact natural LLVM stride. It must not inspect a
  source matrix, type name, dimensions, or layout option.
- Resource loads preserve exact element type, access, pointer layout/address space, and natural
  alignment. No padded physical aggregate is reinterpreted as a layout-compatible one.
- Existing scalar/vector/copyable-struct resource behavior and ordinary CUDA-source routing remain
  unchanged.

## Interfaces and Dependencies

The existing real provider already represents structs containing fixed arrays, typed resource
pointers, aggregate loads, field extraction, and dynamic sequential array/vector selection. The
storage work therefore reuses those APIs. The first newly exposed GenericAsm requires forward-only
builder ABI revision 19's generic typed bit-reinterpret operation; no matrix, resource, or
source-builtin callback is added.

CUDA 12.9 libNVVM, `ptxas`, the Release provider, and the local CUDA runtime provide external
evidence. Existing CPU/CUDA/Vulkan fixture lanes remain regression evidence.

## Milestones

1. Instrument the early LLVM storage pass and dump the post-pass/final IR. If the pass already
   produces canonical physical storage, revert every policy prototype and fix the first real
   consumer instead.
2. Support the resulting physical aggregate through generic type, resource, pointer, load, field,
   and sequential contracts. Stop at the first unrelated shape rather than widening arbitrary
   aggregate storage.
3. Add focused fake/real coverage that distinguishes a physical array wrapper from a flat copyable
   struct. Run both existing fixtures with their unchanged inputs and promote direct runtime/PTX
   lanes only when outputs and strides agree.
4. Update the backend design, capability ledger, and this plan; format, validate, self-review, and
   commit all slice files together.

## Validation and Acceptance

Acceptance requires Release provider/compiler/unit builds; focused storage-policy, resource-view,
aggregate load/extract, and adjacent rejection tests; exact new fixture runtime/PTX lanes; retained
native lanes; CUDA 12.9 `ptxas -arch=sm_70`; the full `slang-unit-test-tool/nvvm` prefix; pinned
clang-format; and `git diff --check`. Record outputs, resource strides, PTX/cubin sizes, and every
remaining boundary.

The self-review inventories the producer policy, any classifier rename/widening, alignment helper,
resource resolver, memory operation, and fake-provider change. For each, trace the exact physical
type from `lowerBufferElementTypeToStorageType` to its consumer and remove any source matrix,
dimension, generated-name, or layout-option check.

## Failure and Recovery

If the shared pass produces a padded/nested type outside the selected aggregate contract, preserve
the dump under ignored `build/slice112-*`, record the first stop, and narrow promotion. Revert a
failed policy prototype rather than teaching the emitter to transpose or reinterpret matrix
storage. Do not reset unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep IR/PTX/cubin probes under ignored `build/slice112-*`. Distill settled producer ownership and
runtime evidence into `docs/design/nvvm-backend.md` and the capability ledger, then commit this plan
with implementation as explicitly requested.
