# Lower Float32 matrices through generic aggregate values

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM backend accepts selected ordinary Float32 matrix construction,
component-wise matrix/scalar and matrix/matrix arithmetic, and constant row/column reads. It does
so by requesting Slang's existing matrix legalization and carrying the resulting arrays of row
vectors through generic first-class aggregate construction/extraction operations. The existing
Float builtin-operator fast-path shader should pass direct CUDA runtime and direct PTX lanes through
its matrix section, then stop only at the next independent mixed-type or half boundary.

## Progress

- [x] (2026-08-29) Reproduced the Float builtin suite stopping at `makeMatrix` after Slice 92.
- [x] (2026-08-29) Traced the canonical matrix producer through `legalizeMatrixTypes`: CUDA skips
  the pass, while its existing lowering represents `matrix<T,R,C>` as `Array<Vector<T,C>,R>` and
  rewrites component-wise operations row-by-row.
- [x] (2026-08-29) Made direct NVVM explicitly request all-matrix legalization without changing CUDA-source or
  other target policy.
- [x] (2026-08-29) Replaced the struct-only value-extraction callback with generic aggregate construction and
  constant extraction in exact forward-only builder ABI revision 9.
- [x] (2026-08-29) Admitted the legalized fixed-array value shapes in direct preflight, availability validation,
  type lowering, fake-provider traces, and real-provider emission.
- [x] (2026-08-29) Added focused positive and negative builder/emitter coverage and registered completed shader
  runtime/PTX lanes.
- [x] (2026-08-29) Formatted, built, ran focused/full/CUDA validation, assembled PTX, self-reviewed, updated durable
  docs and this plan, and commit the slice.

## Surprises and Discoveries

- The matrix representation and component-wise rewrite already exist at the principled producer
  boundary. The gap is target policy: `targetLegalizesMatrixTypes` deliberately returns false for
  CUDA, because CUDA source has native matrix handling, but the experimental direct PTX path shares
  that target identity and needs LLVM-compatible aggregates.
- The legalizer's physical matrix type is already accepted by the fixed numeric array type
  classifier when its row is a Float32 vector. The missing operations are value-form `makeArray`
  and array `getElement`; adding a matrix type or matrix callback to the LLVM shield would duplicate
  upstream semantics.
- Simple straight-line matrix source is optimized into independent row vectors before emission.
  A branch selecting between two matrices was necessary to retain the fixed row array as a
  first-class value and prove aggregate phi transport at the real provider boundary.
- The real provider initially limited generic function/phi values to scalars and vectors. The
  branch-sensitive shader exposed that restriction. Recursively recognizing nonempty fixed arrays
  and structs in the provider's existing physical-value predicate admits the LLVM aggregate shape
  without widening the compiler's selected helper-signature policy.

## Decision Log

- Decision: add an explicit `lowerAllMatrixTypes` policy to the existing matrix legalization pass
  and set it only when `TargetProgram::shouldEmitNVVMDirectly()` is true.
  Rationale: the target-specific IR pass owns matrix representation, while CUDA source and NVRTC
  must retain their established native-matrix path. Direct emission should consume legalized IR,
  not patch `makeMatrix` locally.
  Date/author: 2026-08-29, Codex.
- Decision: expose generic aggregate construction and constant element extraction, and retire the
  struct-only value-extraction callback in the same exact ABI revision.
  Rationale: both LLVM arrays and structs use `insertvalue`/`extractvalue`; an array-specific or
  matrix-specific API would repeat the bring-up scaling problem. There is no backward-compatibility
  requirement for this experimental ABI.
  Date/author: 2026-08-29, Codex.
- Decision: retain the separate vector constructor/extractor.
  Rationale: LLVM vectors are not aggregates and use `insertelement`/`extractelement`; their index
  may be dynamic, whereas aggregate extraction is intentionally constant and structurally bounded.
  Date/author: 2026-08-29, Codex.
- Decision: register one dedicated branch-sensitive matrix shader instead of adding a direct lane
  to the complete existing Float builtin fixture.
  Rationale: the existing fixture proceeds into an independent half-conversion boundary after its
  matrix section. A dedicated test gives truthful completed runtime/PTX evidence for matrices,
  while the full fixture remains a useful probe that now stops at `floatCast`.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Slice 93 is complete. Direct NVVM now runs the canonical producer-side matrix legalizer, then
transports the resulting fixed arrays of Float row vectors as ordinary first-class aggregate
values. The LLVM shield exposes only generic array/struct construction and extraction; it contains
no matrix API. Exact ABI revision 9 removes the former struct-only extraction callback.

The real-provider builder test verifies four `insertvalue` operations and one `extractvalue` in
both LLVM 14 and NVVM-2.0-compatible text and covers null, foreign, wrong type/count, unavailable,
out-of-range, and post-termination rejection without mutation. The fake emitter proves a Float2
row type, at least two aggregate constructions, branch/array-phi transport, constant row one and
column one extraction, and the final Float store.

`tests/cuda/nvvm-float-matrix-values.slang` passes 2/2 through direct CUDA execution and direct PTX
checking with `8, 15`. Its direct PTX is 760 bytes, retains the branch decision as `selp.f32`, and
CUDA 12.9 `ptxas -arch=sm_70` produces a 2,792-byte cubin. The complete NVVM prefix passes 365/365;
the adjacent existing Float suite passes all 3 available lanes with one D3D12 lane ignored. A
manual direct probe of that complete fixture now stops at `floatCast`, rather than `makeMatrix`.

Self-review inventory:

- `MatrixTypeLoweringOptions::lowerAllMatrixTypes` survives as the explicit producer-policy
  discriminator between direct NVVM and CUDA-source/NVRTC emission. It changes the existing
  canonical representation producer instead of compensating in emission.
- `_getAggregateElementCount` and `_getAggregateElementType` survive as the real provider's single
  physical mapping for the LLVM array/struct callbacks. They do not infer Slang or matrix meaning.
- `_getNVVMAggregateConstruction` and `_getNVVMAggregateElement` survive as exact final-IR shape
  resolvers. They require the existing canonical numeric-array classifier, explicit ordered
  operands, exact element types, and a bounded constant index; they add no fallback equivalence or
  operand-graph search.
- The fake provider's aggregate helpers survive only as boundary instrumentation for the same
  physical contracts. The historical struct-value instrumentation names were removed with the
  callback. No checked semantic value is rebuilt as syntax, no malformed producer shape is
  patched, and no failure-only compatibility path remains.

## Context and Current Pipeline

Consider the existing shader section:

    float2x2 ma = float2x2(a, b, b, a);
    float2x2 mb = ma + 1.0;
    float2x2 ms = ma + mb;
    outputBuffer[10] = mb[0][0];
    outputBuffer[11] = ms[1][1];

Final CUDA-target IR retains `makeMatrix` because `targetLegalizesMatrixTypes` excludes CUDA.
Direct preflight in `source/slang/slang-emit-nvvm.cpp` therefore reports `makeMatrix` before the
provider is called. The existing `legalizeMatrixTypes` pass can instead turn the value into an
array of Float2 rows, turn each add into row extraction plus the already-supported vector add, and
turn the result back into an array. Constant matrix row access becomes array element extraction;
the following column access remains the established vector element extraction.

The LLVM shield already creates fixed array types for byte-address values and already extracts
struct fields with `extractvalue`. Exact ABI revision 9 will express those common LLVM aggregate
operations once. Direct preflight will accept only fixed numeric arrays whose complete producer and
consumer shapes are canonical `makeArray` and constant `getElement`; it will not infer matrix
semantics from arbitrary arrays.

## Scope and Non-Goals

In scope:

- direct-NVVM all-matrix legalization using the existing target IR pass;
- fixed Float32 matrix values lowered to arrays of two through four selected row vectors;
- generic LLVM aggregate construction and constant extraction for exact fixed arrays and existing
  struct-value consumers;
- selected component-wise Float32 matrix arithmetic already defined by the legalizer and typed
  scalar/vector operation families;
- direct runtime, PTX, fake-provider, and real-provider evidence.

Out of scope:

- matrix multiplication, transpose, determinant, dynamic aggregate indexing, matrix memory layout,
  matrix function ABI, Boolean/integer matrix values, Float64/half matrices, or cooperative matrices;
- changing CUDA-source, NVRTC, SPIR-V, Metal, or host matrix representation;
- reconstructing matrix semantics in the LLVM provider or adding matrix-specific callbacks.

## Architecture and Invariants

- The target IR matrix legalizer is the only source of truth for physical matrix decomposition.
- Direct NVVM receives no matrix types or matrix-producing operations for the selected path.
- A provider aggregate is an LLVM array or struct with an exact ordered element sequence. Every
  constructor element has the declared physical element type and is available at the insertion
  point; extraction uses a statically bounded index.
- Fixed matrix arrays remain first-class SSA values. The emitter validates dominance/availability
  directly and does not spill them to memory or walk operand graphs to rediscover rows.
- Unsupported aggregate kinds, element types, dynamic indices, dimensions, or unavailable values
  fail before provider mutation and produce no output handle.

## Interfaces and Dependencies

Advance `SLANG_NVVM_BUILDER_ABI_REVISION` to 9. Replace `emitStructFieldValue` with required
`emitAggregateElementExtract`, and add required `emitAggregateConstruct`, updating the facade, real
LLVM provider, fake provider, ABI completeness checks, and focused tests together. Add a matrix
lowering options record to `slang-ir-legalize-matrix-types.h` and pass the direct-NVVM policy from
`slang-emit.cpp`. No public Slang API or libNVVM API changes.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. Builds and tests run outside the sandbox per repository instructions.

## Milestones

1. Parameterize the existing matrix pass and verify direct final IR contains arrays/vectors rather
   than matrix types/operations.
2. Implement exact ABI revision 9 generic aggregate construction/extraction with strict real/fake
   validation and replace all struct-only value-extraction callers.
3. Classify, preflight, validate, and emit canonical fixed-array `makeArray`/constant `getElement`
   values without widening byte-address memory rules.
4. Add economical builder and direct-emitter tests covering valid Float row arrays and invalid
   aggregate type/count/index/availability/module shapes.
5. Add a dedicated direct CUDA/PTX matrix fixture, use the existing Float builtin suite to measure
   the next boundary, and record representative compatible LLVM and PTX evidence.
6. Format, build, run the complete NVVM prefix and changed shader prefix, assemble PTX, perform the
   input-shape/special-case audit, update durable documents and this plan, and commit.

## Validation and Acceptance

Acceptance requires focused real-provider normal/compatible assembly checks for aggregate
`insertvalue`/`extractvalue`, fake-provider ordered type/value/index traces, negative no-mutation
coverage, the complete `slang-unit-test-tool/nvvm` prefix, direct runtime and PTX lanes for a
branch-sensitive matrix shader, CUDA 12.9 PTX assembly, pinned clang-format 17,
and `git diff --check`.

## Failure and Recovery

First inspect the final linked IR after matrix legalization: any retained matrix type or operation
is a producer-policy failure, while a rejected `makeArray`/`getElement` is a direct aggregate
boundary failure. Generated IR/PTX/cubin probes stay under ignored `build/`. All host/provider ABI
changes land atomically in this forward-only slice. Never reset unrelated work or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

The recorded evidence is: `makeMatrix` before and `floatCast` after; fixed arrays of Float row
vectors in final IR; four `insertvalue` and one `extractvalue` in both provider text forms; ordered
fake aggregate construction/extraction and array phi traces; runtime output `8, 15`; 760-byte PTX;
2,792-byte cubin; 365/365 full-prefix tests; 2/2 matrix shader lanes; and the self-review inventory
above. The settled representation is distilled into `docs/design/nvvm-backend.md`, and the durable
test evidence is recorded in the capability ledger.
