# Preserve matrix storage layout before direct NVVM legalization

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM preserves the selected row- or column-major memory representation
when a matrix is loaded from a constant buffer before that logical matrix becomes the backend's
row-array value. The existing `tests/compute/column-major.slang` direct CUDA lane produces its
unchanged expected result `1`; row-major behavior remains `11, 22, 33, 1`. Neighboring non-square
fixtures are promoted only if their documented CUDA packing semantics also pass unchanged.

## Progress

- [x] (2026-08-29) Reproduced the Slice 110 column-major runtime mismatch and traced its final PTX
  and IR.
- [x] (2026-08-29) Prototyped the established LLVM buffer-storage lowering before direct-NVVM
  matrix legalization and measured its canonical physical-wrapper/unpack graph.
- [x] (2026-08-29) Promoted the producer-side pipeline fix and added only the generic
  parameter-group, fixed-array value, and sequential-value contracts required by that graph.
- [x] (2026-08-29) Added focused representation coverage and exact square row/column-major plus
  compatible non-square row-major runtime/PTX lanes.
- [x] (2026-08-29) Built and formatted both components; ran focused/full validation and native
  regressions; assembled PTX; completed the input-shape self-review; and updated durable status.

## Surprises and Discoveries

- Slice 110 emits valid column-major PTX, but that PTX reads four contiguous Float4 values and uses
  them as logical rows. With the fixture's column-major buffer data, `r.x` becomes `1` instead of
  `11`, so the Boolean output is `0`.
- `specializeMatrixLayout` correctly changes the source matrix from unknown layout to layout value
  `2` (column-major). The information is lost later because `legalizeMatrixTypes` runs before the
  main `lowerBufferElementTypeToStorageType` invocation on direct NVVM. By then the buffer element
  is merely `Array<Vec<Float,4>,4>` and the storage pass can no longer generate a transpose-aware
  matrix conversion.
- CPU-via-LLVM already documents and solves this exact ordering problem: it runs
  `lowerBufferElementTypeToStorageType` with `BufferElementTypeLoweringPolicyKind::LLVM` before
  lowering all matrices. Direct NVVM also lowers all matrices for LLVM and currently misses that
  established producer-side path.
- The producer fix creates a `[PhysicalType]` struct whose sole field is the physical major-vector
  array. Its unpack helper reads that field as a whole and dynamically selects rows while building
  logical vectors. The first successive stops were the array-valued helper parameter, wrapper
  field, immutable element address, and dynamic fixed-array `getElement`; each is a generic
  producer/consumer relation rather than a matrix operation.
- LLVM has no dynamic `extractvalue`. ABI revision 18 therefore generalizes the vector extraction
  callback to sequential-value extraction and implements dynamic fixed-array selection as bounded
  constant `extractvalue` plus typed `select` operations. Starting from `undef` retains the source
  operation's undefined out-of-range behavior and remains compatible with libNVVM's LLVM 7 text
  reader.
- The non-square row-major fixture retains its documented packed CUDA behavior and passes direct
  runtime with `12, 16`. The non-square column-major physical type is
  `Array<Float3, 2, stride=12>`; LLVM's `<3 x float>` element has 16-byte natural alignment and
  allocation size. Treating it as an ordinary LLVM array would silently change storage layout, so
  it remains an exact adjacent stop pending a padded sequential-storage representation.

## Decision Log

- Decision: make Slice 111 a producer-side matrix-storage ordering slice, not an NVVM-emitter
  transpose special case.
  Rationale: matrix layout is present before `legalizeMatrixTypes` and absent afterward. The shared
  buffer-storage pass already owns conversion between physical major-vector storage and logical
  row vectors; the NVVM emitter should continue consuming canonical post-legalization IR.
  Date/author: 2026-08-29, Codex.
  Revisit if the LLVM policy mutates ordinary CUDA-source behavior or cannot simplify to generic
  direct-NVVM shapes without adding target-specific representation knowledge downstream.
- Decision: generalize the existing value-extraction callback in place and bump the forward-only
  ABI to revision 18.
  Rationale: the canonical graph dynamically indexes a first-class fixed array, and fixed vectors
  and arrays are both bounded sequential values. A matrix-specific callback or a parallel array
  callback would duplicate the same semantic operation and make the shielded LLVM interface less
  general.
  Date/author: 2026-08-29, Codex.
- Decision: accept explicit fixed-array strides only when they equal the selected element's natural
  LLVM stride.
  Rationale: the provider represents the value as an LLVM array and has no padding descriptor.
  Rejecting a 12-byte-strided Float3 array is required to prevent a 16-byte LLVM representation
  from changing the shader's storage ABI.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The existing producer-side LLVM storage policy was the right ownership boundary. It preserves
column-major intent until conversion code has been made explicit, after which the ordinary matrix
legalizer and generic direct emitter can remain layout-oblivious. The square column-major runtime
result changed from the Slice 110 mismatch `0` to the fixture's unchanged expected `1`; row-major
remains `11, 22, 33, 1`.

The slice also improved the builder economy: one forward-only sequential-value extraction contract
now covers vectors and arrays, and the fake provider uses the same name. No callback mentions
matrices, dimensions, major-ness, or transposition. The non-square probe exposed a real physical
layout boundary rather than an emitter omission, so it was recorded rather than masked.

## Context and Current Pipeline

`specializeMatrixLayout` in `source/slang/slang-ir-specialize-matrix-layout.cpp` resolves the target
option. `emit.cpp::emitEntryPoints` later invokes `legalizeMatrixTypes` with `lowerAllMatrixTypes`
for direct NVVM, which converts `matrix<T,R,C>` to `Array<Vec<T,C>,R>`. The main
`lowerBufferElementTypeToStorageType` pass runs after this point and therefore sees no matrix or
major-ness. `slang-emit-nvvm.cpp` correctly lowers the resulting array, but it cannot recover the
discarded storage-layout relationship.

The CPU-via-LLVM branch immediately before `legalizeMatrixTypes` invokes the same buffer lowering
with the LLVM policy specifically so matrix layout is converted while the logical matrix type is
still present. Its generated pack/unpack helpers represent the conversion explicitly in IR. The
prototype extends that established ordering to `emitNVVMDirectly`, then observes what remains after
matrix legalization, force inlining, and simplification.

## Scope and Non-Goals

In scope are Float32 matrices in constant buffers, explicit row/column target layout, the generic
array/vector/aggregate operations produced by existing storage lowering, and probes of both square
and non-square dimensions. Runtime lanes are promoted only when their physical arrays are exactly
representable by the established LLVM aggregate contract and preserve the fixture's existing CUDA
packing semantics.

Out of scope are an emitter-side matrix type or transpose operation, source-matrix recognition in
NVVM preflight, matrix-bearing arbitrary structs or structured buffers unless the shared storage
pass naturally reduces them to already-supported shapes, writable matrix storage, non-Float32
matrix elements, and changing fixture inputs or expected values.

## Architecture and Invariants

- `lowerBufferElementTypeToStorageType` is the only owner of logical-to-physical matrix buffer
  layout conversion.
- `legalizeMatrixTypes` remains the only owner of the backend's logical row-array representation.
- Direct preflight and emission see no matrix type and never infer layout from array dimensions.
- Row-major behavior and ordinary CUDA-source pass ordering remain unchanged.
- Generated storage-conversion helpers must be force-inlined or use generic helper contracts; no
  callback may encode matrix major-ness, dimensions, or transposition.
- Runtime evidence, not PTX validity alone, is required before registering a layout lane.

## Interfaces and Dependencies

The producer change extends the early buffer-lowering condition in `source/slang/slang-emit.cpp`.
The final canonical IR also requires a fixed-array helper value, the sole fixed-array field of a
physical parameter-group wrapper, and integer-indexed sequential value extraction. Builder ABI
revision 18 generalizes the existing vector extraction callback in place; it adds no parallel
matrix or array API. Explicit array strides are admitted only when equal to natural LLVM stride.

CUDA 12.9 libNVVM and `ptxas`, the Release direct provider, and the RTX 5090 runtime are the local
external evidence. `tests/compute/column-major.slang` and `row-major.slang` are the semantic source
of truth.

## Milestones

1. Extend the existing CPU-via-LLVM early storage-lowering condition to direct NVVM. Compile the
   column-major fixture and record the final direct preflight stop or successful PTX. Discard the
   prototype if it duplicates storage lowering, changes CUDA-source IR, or leaves matrix-layout
   decisions downstream.
2. If the prototype succeeds structurally, promote it and support any remaining canonical generic
   conversion graph at its true producer/consumer boundary. Add fake coverage that distinguishes
   physical major-vector storage from logical row-array values without naming a matrix operation.
3. Register exact column-major runtime coverage when output is `1`; rerun row-major runtime/PTX.
   Probe non-square row/column fixtures, but register them only when their existing packed/expected
   semantics pass unchanged.
4. Update `docs/design/nvvm-backend.md`, the capability ledger, and this plan with exact evidence;
   format, validate, self-review, and commit all slice files together.

## Validation and Acceptance

Acceptance requires a focused unit test for storage conversion, the exact new and existing matrix
runtime/PTX lanes, Release provider/compiler/unit builds, CUDA 12.9 `ptxas -arch=sm_70`, the full
`slang-unit-test-tool/nvvm` prefix, relevant non-NVVM matrix/lowering unit tests, pinned
clang-format, and `git diff --check`. Record PTX/cubin sizes and every fixture result.

The input-shape audit inventories the pipeline condition and every new helper, fallback, or special
case. Reverting the producer-side change must restore the column-major output `0`. No retained
emitter change may recognize source matrix syntax, a fixed 4x4 shape, or the target layout option.

Validation evidence on 2026-08-29:

- CUDA 12.9 Release provider, `slangc`, and `slang-unit-test` builds passed after pinned
  clang-format.
- Exact ABI negotiation/mismatch, generic aggregate builder, and physical matrix-storage fake
  tests passed. The builder assembly contains four `insertvalue`, three `extractvalue`, and two
  typed `select` operations, with no `poison` token.
- `column-major.slang` CPU, native CUDA, direct CUDA, and direct PTX lanes passed; direct output is
  `1`. `row-major.slang` CPU, direct CUDA, and direct PTX lanes passed with `11, 22, 33, 1`.
- `non-square-row-major.slang` CPU, native CUDA, and new direct CUDA lanes passed with its documented
  packed result `12, 16`. The direct 881-byte PTX assembled to a 2,920-byte cubin.
- Final row-major PTX/cubin sizes are 1,433/3,048 bytes. Final column-major sizes are
  2,951/3,688 bytes. All three modules passed CUDA 12.9.86 `ptxas -arch=sm_70`.
- The complete `slang-unit-test-tool/nvvm` prefix passed 380/380. `git diff --check` passed. A full
  `column-major.slang` prefix probe also ran the direct lanes successfully; its unrelated existing
  synthesized WGPU lane failed Dawn bind-group creation and is not direct-NVVM evidence.

Self-review inventory and input-shape audit:

- The pipeline condition survives. Its exact producer input is a specialized logical matrix in a
  constant buffer. That is canonical and intentionally precedes `legalizeMatrixTypes`; the shared
  `lowerBufferElementTypeToStorageType` pass, not the emitter, owns the physical/logical conversion.
  The Slice 110 runtime run without this condition produced `0`, while the same fixture now passes
  with `1`.
- `asNVVMSupportedParameterGroupStructType` survives. Its input is the sole-field
  `[PhysicalType]` wrapper produced by the shared storage pass. It classifies only that structural
  invariant and an already-selected numeric array; it neither reconstructs source syntax nor
  recognizes a matrix name or dimension.
- Explicit-stride handling in `asNVVMSupportedNumericArrayType` survives. A stride equal to natural
  LLVM element stride is the same representation with an explicit spelling. The rejected
  12-byte-strided Float3 column-major probe proves why a different stride is not canonical input for
  the current provider and must not be accepted as an equivalent LLVM array.
- The immutable wrapper-field/array-element resolver survives. The shared storage pass produces the
  exact chain `parameter group -> sole physical field -> fixed array element`; the existing struct
  key and pointer-type checks remain the semantic source of truth. No graph walk or fallback
  rediscovers matrix context.
- Generic fixed-array helper parameters and sequential-value extraction survive. The generated
  unpack helper passes the selected array by value and dynamically indexes it. ABI revision 18
  expresses the existing bounded sequential operation directly; the provider validates ownership,
  dominance, integer index, type, and constant range before mutation. No matrix-specific callback
  or compatibility alias remains.
- The fake provider's increased recorded-extraction capacity survives as test infrastructure: the
  canonical 4x4 unpack graph performs sixteen scalar extracts before existing entry operations. It
  does not affect production limits or classification.

## Failure and Recovery

If the early LLVM policy produces a graph outside the prototype's bounded generic contracts,
preserve the dump under ignored `build/`, record its first stop, and decide whether it forms one
coherent slice. Revert the one-line prototype rather than patching final array loads with a
transpose. Do not alter ordinary CUDA-source routing, weaken validation, reset unrelated work, or
stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep PTX/cubin and filtered IR probes under ignored `build/slice111-*`. Distill the settled pass
ordering and runtime boundary into `docs/design/nvvm-backend.md`, update the file-backed capability
ledger, and commit this plan with the implementation as explicitly requested by the user.
