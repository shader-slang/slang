# Generalize 32-bit integer-vector construction and extraction

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM backend supports ordinary two- through four-lane signed and
unsigned 32-bit integer-vector values, including flat construction, scalar splat, constant-index
element extraction, multi-lane swizzle, established wrapping arithmetic, and sign-only integer
conversion. The existing `tests/cuda/dispatch-thread-id-extraction.slang` vector entry points must
compile through direct libNVVM instead of stopping at E52017 `CUDA execution-index component`,
their registered PTX checks must pass, and CUDA `ptxas` must accept the resulting PTX.

The implementation must be generic LLVM IR construction. It must not recognize the shader entry
name, bypass a vector by looking through its producer, or add CUDA execution-ID-shaped operations.

## Progress

- [x] (2026-08-28) Completed and committed Slice 80 as `33e15b4e7`.
- [x] (2026-08-28) Probed all five entries in
  `tests/cuda/dispatch-thread-id-extraction.slang`: scalar `computeMain`, `computeMain4`, and
  `computeMain5` compile; vector `computeMain2` and `computeMain3` stop at E52017
  `CUDA execution-index component`.
- [x] (2026-08-28) Inspected final linked IR. The unsigned entry contains a genuine
  `swizzle(Vec<UInt,3>, 0, 1) -> Vec<UInt,2>` followed by scalar swizzles. The signed entry adds
  `intCast(Vec<UInt,2>) -> Vec<Int,2>`.
- [x] (2026-08-28) Added exact generic vector construction to ABI revision 7 and covered real/fake
  validation plus normal and LLVM 7-compatible text.
- [x] (2026-08-28) Generalized 32-bit integer-vector typing, construction, extraction, swizzle,
  established arithmetic, and same-lane integer conversion without widening storage roles.
- [x] (2026-08-28) Registered focused negative, builder, emitter, existing-file direct PTX, and
  CUDA 12.9 `ptxas` evidence.
- [x] (2026-08-28) Formatted, built, passed 347/347 complete NVVM tests and the existing shader's
  3/3 lanes, updated durable documents, and completed the input-shape self-review.

## Surprises and Discoveries

- CUDA varying legalization does not flatten a `uint2 SV_DispatchThreadID` into unrelated scalar
  special-register reads. It constructs the canonical `uint3` execution expression, swizzles its
  first two lanes into a real `uint2`, and later extracts the lanes required by the body. The
  rejected shape is therefore an ordinary vector data-flow graph, not an execution-semantic
  special case.
- The provider's parameterized integer-binary family already accepts selected integer vectors with
  two through four lanes, and LLVM construction already supports those result types. Host type
  classification and missing vector construction are the narrower boundaries.
- LLVM integer vectors are signless. The signed entry's same-width `uint2 -> int2` conversion has
  the same physical value, but its semantic descriptor still must preserve signedness so later
  ordered operations choose the correct interpretation.

## Decision Log

- Decision: replace the two historical `uint3` and `int2` value classifiers with one bounded
  signed/unsigned 32-bit integer-vector classifier for lane counts two through four.
  Rationale: the canonical vector type already carries element signedness and lane count. One
  classifier removes the proof-of-concept pair without claiming narrow/wide integers, float
  vectors, Boolean vectors, or matrices.
  Date/author: 2026-08-28, Codex.
- Decision: add one `emitVectorConstruct` builder operation taking the exact vector type and its
  scalar elements, and retain the existing generic element-extract operation.
  Rationale: LLVM requires an aggregate value to materialize a multi-lane swizzle. One generic
  construction boundary covers `IRMakeVector`, `IRMakeVectorFromScalar`, and `IRSwizzle` without
  exposing Slang opcodes or CUDA semantics in the provider ABI.
  Date/author: 2026-08-28, Codex.
- Decision: construct a swizzle result from exact constant-index extractions rather than folding
  through its producer.
  Rationale: the linked IR swizzle is the semantic source of truth and may have any available
  vector producer. Looking through the current execution expression would be a shader-specific
  workaround and would fail as soon as an ordinary constructed vector reaches the same operation.
  Date/author: 2026-08-28, Codex.
- Decision: generalize integer conversion only when input and result are selected integer values
  with the same lane count.
  Rationale: LLVM truncation, extension, and same-width identity are naturally elementwise. The
  host still admits only scalar selected widths and 32-bit vectors, preserving the current public
  scope while keeping the provider operation mathematically complete.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Exact forward-only builder ABI revision 7 now constructs any fixed integer vector admitted by the
host from an exact scalar lane list. The LLVM provider validates the entire request before inserting
into `undef`; the fake provider records the same contract. The direct emitter uses that one generic
operation for canonical flat constructors, scalar splats, and multi-lane swizzles, while the
existing generic extraction operation handles scalar reads. No CUDA execution-shaped callback or
producer look-through was added.

The two formerly rejected entries in `tests/cuda/dispatch-thread-id-extraction.slang` now compile
through direct libNVVM. Its original CUDA-source lane and two new direct-PTX lanes pass 3/3. The
unsigned PTX reads both x and y execution-register dimensions; the signed case carries the
same-width `uint2 -> int2` semantic conversion and optimizes the unused y lane away. CUDA 12.9
`ptxas -arch=sm_70` accepts both modules. The standalone provider and Release host builds pass, as
does the complete NVVM prefix at 347/347.

The input-shape audit retained no representation workaround. `IRMakeVector`,
`IRMakeVectorFromScalar`, constant-index `IRGetElement`, and `IRSwizzle` are canonical producers
owned by this value-lowering layer. Dynamic indexing remains an explicit pre-provider boundary,
and device/resource vector storage remains at its earlier narrow proof rather than inheriting the
broader value role accidentally.

## Context and Current Pipeline

Consider the existing entry:

```slang
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain2(
    uint2 tid : SV_DispatchThreadID,
    StructuredBuffer<uint> src,
    RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.y];
}
```

`legalizeEntryPointVaryingParamsForCUDA` removes `tid` from the physical signature and creates
`blockIdx * blockDim + threadIdx` as `Vec<UInt,3>`. The final entry contains a two-lane `IRSwizzle`
of that expression, then one-lane swizzles used as the output and input indices. Slice 73's helper
`_getNVVMCUDAExecutionVectorElement` accepts only a one-lane extract whose base is exactly
`Vec<UInt,3>`, so `_validateNVVMFunction` rejects the producer before provider discovery.

The provider already exposes `getVectorType` and `emitVectorElementExtract`, and its dimensioned
integer operation family accepts vector add/subtract/multiply/bitwise operations. It lacks the
generic inverse operation that assembles scalar elements into a vector. The signed entry also
reaches the parameterized integer-conversion family after the swizzle is admitted, but that family
currently restricts conversions to scalars.

## Scope and Non-Goals

In scope are canonical `IRVectorType` values with signed or unsigned 32-bit integer elements and
literal lane counts two, three, or four; `IRMakeVector` from exact scalar elements;
`IRMakeVectorFromScalar`; constant-index `IRGetElement`; one- or multi-lane `IRSwizzle`; established
integer binary operations on these vectors; and elementwise integer conversion preserving lane
count. The existing CUDA dispatch-ID file is the end-to-end compatibility gate.

Out of scope are vector entry/ordinary helper parameters and results, device/resource vector
storage beyond the established signed-i32x2 pointer proof, dynamic indexing, shuffles across two
base vectors, vector phis, vector comparisons/Boolean vectors, shifts/division/remainder, float or
low/wide-integer vectors, matrices, and vector fields in aggregates. Those remain deterministic
pre-provider boundaries unless already established independently.

## Architecture and Invariants

One classifier returns a canonical vector only when its element is exactly Slang `Int` or `UInt`
and its literal lane count is in `[2,4]`. It reports signedness and lane count. Semantic provider
descriptors retain both even though LLVM integer vector types are signless.

Vector construction validates before mutation that the supplied type is a fixed vector, the
element count exactly equals its lane count, every input has exactly the element type, and all
values are usable at the insertion point. The provider begins with LLVM `undef` and inserts every
lane in order. No partially constructed value escapes on failure.

Every extraction validates an exact literal index within the base lane count and an exact scalar
result type. Every multi-lane swizzle validates an exact vector result whose lane count equals the
index count and whose element type equals the base element type. Emission extracts each requested
lane, then calls the same generic constructor used for ordinary vector construction.

## Interfaces and Dependencies

Revise `source/compiler-core/slang-nvvm-ir-builder-api.h` to exact forward-only ABI revision 7 and
append `emitVectorConstruct` to the construction interface. Update the facade, real provider, fake
provider, interface completeness test, and version identity. No older ABI remains loadable.

Update `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` with the common 32-bit integer-vector
classifier and use it for value type lowering while retaining the exact established memory role.
Update `source/compiler-core/slang-nvvm-semantic-catalog.h` so elementwise integer conversion
requires selected integer input/result values with matching lane counts.

Update `source/slang/slang-emit-nvvm.cpp` with generic vector construction/extraction descriptors,
shape and SSA validation, capability preflight through ABI completeness, and emission using only
generic builder operations.

## Milestones

1. Add ABI revision 7 vector construction with exact real/fake validation and LLVM 14 plus LLVM
   7-compatible assembly tests.
2. Consolidate integer-vector classification and semantic descriptors without broadening memory,
   helper-signature, or aggregate roles.
3. Validate and emit ordinary vector construction, splat, constant extraction, and swizzle through
   the generic construct/extract pair, including adjacent invalid-shape coverage.
4. Compile both existing dispatch-ID vector entries through real libNVVM, inspect PTX, run `ptxas`,
   and add file-backed direct static lanes where the fixture permits.
5. Run focused and complete NVVM tests, update durable capability/design documents, perform the
   input-shape self-review, and commit this plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- ABI revision 7 rejects null/mismatched vector type, wrong element count/type, unusable values,
  and missing output without mutating the module;
- fake-boundary evidence records exact source lanes and constructed result type;
- multi-lane swizzles use generic extracts plus construction, with no execution-ID callback or
  producer look-through;
- signedness-only `uint2 -> int2` preserves the physical value and semantic signed result;
- float vectors, Boolean vectors, dynamic indexing, lane-count mismatch, and unsupported vector
  roles remain E52017 before provider mutation;
- both existing vector dispatch-ID entries compile through libNVVM and direct PTX passes CUDA 12.9
  `ptxas -arch=sm_70`;
- the registered existing shader file passes its CUDA-source and direct-PTX lanes;
- standalone provider and Release host/test builds pass;
- focused tests and the complete `slang-unit-test-tool/nvvm` prefix pass;
- pinned clang-format 17 and `git diff --check` pass; and
- `external/slang-binaries/` and generated `build/` artifacts remain unstaged.

## Failure and Recovery

If LLVM 14 assembly verifies but libNVVM rejects vector insertion, isolate a two-lane i32
construction and inspect the LLVM 7-compatible text before changing the Slang representation. If
runtime output differs, compare the exact extraction index list and PTX lane/register flow before
adding any optimization. Do not fold through the execution-global producer to make the fixture
pass.

All ABI, provider, emitter, fake, test, and document changes are one forward-only slice and can be
reverted together. Generated probes remain under ignored `build/` paths.

## Artifacts and Hand-Off

Retain final linked IR, LLVM 14 assembly, LLVM 7-compatible assembly, direct PTX, `ptxas` output,
and runtime comparison output under ignored `build/` paths. Distill the generic vector
construction contract, supported type/operation matrix, existing-file result, remaining vector
boundaries, and exact validation totals into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md` before committing Slice 81.
