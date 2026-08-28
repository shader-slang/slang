# Generalize 32-bit numeric vectors and float byte-address access

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM treats two- through four-lane Float vectors as ordinary bounded
numeric values alongside the established Int/UInt vectors. Canonical flat construction, scalar
splat, constant extraction, and swizzle use the existing generic vector builder operations. Core
byte-address loads and stores admit exact 32-bit Int, UInt, and Float scalars and vectors, composing
with Slice 83's generic byte-offset pointer and load/store operations without a new builder ABI.

The existing `tests/compute/byte-address-buffer-aligned.slang` and
`tests/compute/byte-address-buffer-singlearg-float3-11592.slang` must compile through direct
libNVVM, execute through the CUDA comparison harness, expose representative Float3/Float4 PTX, and
pass CUDA 12.9 `ptxas -arch=sm_70`.

## Progress

- [x] (2026-08-29) Completed and committed Slice 83 as `69e9d6880` with 353/353 NVVM tests.
- [x] (2026-08-29) Probed aligned, Float3, aggregate, UInt64, and vector-arithmetic candidates.
- [x] (2026-08-29) Captured the aligned shader's canonical Float/Float4 byte loads and stores,
  Float4 construction, and constant element extraction; captured the independent aggregate,
  UInt64, and shift boundaries.
- [x] (2026-08-29) Added one exact selected 32-bit numeric-vector classifier and used it for value lowering,
  construction, extraction, and alignment.
- [x] (2026-08-29) Admitted selected 32-bit numeric scalar/vector byte loads and read-write stores through the
  established generic memory path.
- [x] (2026-08-29) Generalized fake-provider vector type/value fidelity and added focused topology plus adjacent
  rejection coverage.
- [x] (2026-08-29) Registered both existing shaders for direct runtime/PTX evidence and ran real libNVVM/`ptxas`.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, updated durable documents, self-reviewed,
  and prepared the completed slice for commit.

## Surprises and Discoveries

- The aligned shader's final linked graph is already scalarized according to the source alignment
  contract. Wide operations remain exact Float4 byte loads/stores; four-byte operations are four
  Float loads/stores joined by `makeVector` or split by constant `getElement`. No bitcast,
  arithmetic, helper-ABI, or aggregate operation is needed.
- The Float3 regression is the same value family at three lanes: Float constants form one Float3,
  the byte buffer round-trips it, and constant extraction feeds an established float structured
  buffer. Supporting it does not require general array or struct values.
- `byte-address-buffer-array.slang` first requests `Array<Float4, 2>` as a byte access, while
  `byte-address-buffer-64bit.slang` requests UInt64 storage followed by established-width
  arithmetic. Those are independent representation families and are not prerequisites for the
  selected scalar/vector work.
- `cuda-vector-binary-ops.slang` first stops at `shl`. Its shift/division/remainder family is
  independent of float vector construction and remains a measured future slice.
- LLVM optimization scalarizes or forwards some same-location wide memory operations. The focused
  fake fixture deliberately crosses wide and scalarized uses so construction and extraction stay
  observable, while the file-backed lanes validate optimized program behavior and final PTX.
- The aligned fixture previously allocated 16 bytes while its existing accesses reached byte 44.
  Expanding the backing buffer to 48 bytes makes the pre-existing test memory range valid; it does
  not alter the compiler representation or add a backend exception.

## Decision Log

- Decision: define one selected 32-bit numeric-vector family containing exact Int, UInt, or Float
  elements and literal lane counts two through four.
  Rationale: the existing provider's vector type, constructor, and extractor are element-type
  generic. Keeping an integer-only host descriptor would duplicate structurally identical paths and
  prevent canonical Float vectors from using the already-correct boundary.
  Date/author: 2026-08-29, Codex.
- Decision: broaden core byte-address values to exact selected 32-bit numeric scalars/vectors, not
  only Float.
  Rationale: LLVM integer signedness is semantic rather than physical, so Int and UInt use the same
  i32 memory representation, while Float uses the exact typed pointer already accepted by the
  generic builder. One width/lane-bounded classifier is the source of truth; no source spelling or
  reinterpretation is needed.
  Date/author: 2026-08-29, Codex.
- Decision: keep aggregate, 64-bit, narrow, Boolean, matrix, and runtime-alignment byte accesses out
  of this slice.
  Rationale: aggregates require recursive layout/value representation and UInt64 exercises a wider
  memory/arithmetic policy. Including them would mix distinct canonical shapes into a slice whose
  observable result is two existing Float-vector shaders.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Slice 84 established one bounded Int/UInt/Float vector family without changing builder ABI revision
8. Float vector construction/extraction and selected 32-bit numeric byte-address access now compose
from the existing generic type, pointer, load/store, constructor, and extractor operations. The
focused fake-provider run passes 4/4, including the retained UInt64 rejection before provider
mutation. The two existing file-backed shaders pass their direct runtime and PTX lanes 4/4.

Real direct emission produced both aligned Float4 and odd-lane Float3 PTX, and CUDA 12.9.86
`ptxas -arch=sm_70` accepted both modules. The standalone provider and Release `slang-unit-test`,
`slang-test`, and `slangc` targets build successfully. The complete NVVM prefix passes 354/354;
pinned clang-format 17 and `git diff --check` pass.

The final self-review inventory found four intentional generalizations and no compensating
producer repair: the broad vector classifier is the single canonical type-family mapping; the i32
classifier merely narrows it for integer semantics; the byte-access descriptor admits exact
canonical scalar/vector types; and the fake model records element kind at vector construction and
extraction so its consumers do not infer or reconstruct type identity. The test-buffer expansion
repairs an out-of-range fixture allocation. No new fallback, graph walk, source-spelling matcher,
or malformed-IR special case remains.

## Context and Current Pipeline

Consider the existing aligned source:

```slang
buffer0.StoreAligned(32, buffer0.LoadAligned<float4>(32));
buffer0.StoreAligned(32, buffer0.LoadAligned<float4>(8, 4));
buffer0.Store<float4>(8, buffer0.LoadAligned<float4>(32), 4);
```

Target legalization turns the first and third wide loads into canonical Float4 byte loads. The
middle source load becomes four canonical Float loads followed by `makeVector`. Scalarized stores
extract constant Float4 lanes; wide stores consume the Float4 directly. Slice 83's byte-offset
pointer already accepts any provider loadable pointee type, and the real LLVM provider's generic
vector constructor/extractor already validates element identity. The remaining restriction is the
host type/operation boundary and the integer-only fake model.

The Float3 regression constructs `(2.0, 3.0, 4.0)`, stores and reloads it through one
`RWByteAddressBuffer`, then extracts its three lanes into an established
`RWStructuredBuffer<float>`. This supplies runtime evidence for the odd-lane vector case rather
than relying only on Float4 assembly.

## Scope and Non-Goals

In scope are exact Int, UInt, and Float vectors with two through four lanes in ordinary value
roles; canonical `makeVector`, scalar splat, constant-index scalar extraction, and swizzle; exact
Int/UInt/Float scalar and vector byte-address loads; matching read-write stores; established
literal alignment policy; read-only invariant versus read-write ordinary loads; and the two named
existing shader files.

Out of scope are vector helper parameters/results, vector entry parameters, vector phis,
comparisons, arithmetic beyond the already-established integer family, dynamic extraction, vector
device-pointer parameters, structured-buffer vector elements, conventional vector fields,
arrays/structs/matrices, Float64 or narrow values, Boolean values, byte-address atomics/status
loads, runtime alignment, resource arrays, and bounds repair.

## Architecture and Invariants

One type classifier resolves only canonical fixed vectors whose element is exact Int, UInt, or
Float and whose literal lane count is in `[2,4]`. The existing integer-only classifier remains a
narrow semantic helper for integer operation families and execution vectors; broader value,
construction, extraction, alignment, and byte-access paths use the numeric classifier.

Construction validates that every direct lane has the exact scalar element type. Swizzles and
extractions validate the exact base/result relation and literal in-range indices. SSA validation
uses the scalar validator for direct elements and ordinary availability for already-produced
vectors. No matcher reconstructs source overloads or searches producer graphs.

The fake provider represents vector identity by both scalar element kind and lane count. Extracted
fake values retain their scalar kind, so a Float vector lane cannot accidentally satisfy an
integer consumer. This is test-model fidelity for the already-generic provider contract, not a new
production interface.

## Interfaces and Dependencies

Update `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` with one selected 32-bit numeric-vector
classifier and reuse it for value admission, lowering, alignment, and the byte-access classifier.
Update `source/slang/slang-emit-nvvm.cpp` so the existing vector descriptors and diagnostics are
numeric rather than integer-specific and so direct lane validation uses the established scalar
validator.

Update the fake provider and focused emitter tests under `tools/slang-unit-test/` to preserve Float
vector type/value identity. No builder ABI revision, facade callback, real-provider code, semantic
operation catalog row, or libNVVM text rewrite is expected.

Register direct CUDA comparison and PTX lanes in the two existing shader files. Keep all generated
LLVM/PTX/runtime artifacts under ignored `build/` paths.

## Milestones

1. Add the selected 32-bit numeric-vector classifier and generalize type/value lowering.
2. Generalize canonical vector construction/extraction and exact SSA validation.
3. Broaden core byte-address access to selected numeric scalars/vectors and retain access/alignment
   policy.
4. Generalize fake vector identity, add Float3/Float4 topology coverage, and retain aggregate or
   UInt64 pre-provider rejection.
5. Register both existing shaders, compile through real libNVVM, inspect PTX, run CUDA runtime and
   CUDA 12.9 `ptxas`.
6. Run focused and complete NVVM tests, update design/ledger records, self-review, and commit this
   plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- exact Float2-4 type lowering calls the existing generic vector type operation with Float32
  elements and the exact lane count;
- canonical Float vector construction/extraction uses only the existing generic builder calls and
  rejects dynamic/mismatched/unsupported shapes before provider discovery;
- exact Int/UInt/Float scalar/vector byte loads and read-write stores produce matching typed
  byte-offset pointers, alignment, and load flags in the fake trace;
- aggregate and UInt64 byte access remain deterministic E52017 controls before provider mutation;
- both named existing shader files pass their direct CUDA runtime lanes;
- direct PTX contains representative Float3/Float4 global loads/stores, passes FileCheck, and CUDA
  12.9 `ptxas -arch=sm_70` accepts both modules;
- standalone provider and Release host/test builds pass;
- focused tests and the complete `slang-unit-test-tool/nvvm` prefix pass;
- pinned clang-format 17 and `git diff --check` pass; and
- `external/slang-binaries/` and generated `build/` artifacts remain unstaged.

## Failure and Recovery

If libNVVM rejects Float3 typed memory, compare normal LLVM 14 text with compatible NVVM IR 2.0
text and determine whether the problem is the vector type or its alignment. Do not scalarize in the
provider: canonical source legalization already scalarizes operations whose alignment contract
requires it, while the admitted wide operation intentionally carries an exact vector type.

If the fake accepts a Float lane as an integer or vice versa, fix its vector/result type tracking at
the producer rather than adding a consumer exception. All host/fake/test/document changes are one
forward-only slice and can be reverted together; builder ABI revision 8 remains unchanged.

## Artifacts and Hand-Off

Retain final linked IR, direct PTX for Float3/Float4, CUDA runtime output, and `ptxas` artifacts
under ignored `build/` paths. Distill the selected numeric-vector contract, generic byte-access
extension, existing-file results, remaining aggregate/wide/shift boundaries, and exact validation
totals into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md` before committing Slice 84.
