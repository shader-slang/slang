# Fold CUDA type-layout queries for direct NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the existing `tests/cuda/cuda-layout.slang` comparison test runs through the
direct libNVVM route. Its 28 `__alignOf` queries cover 8/16/32/64-bit numeric scalars and CUDA
vectors with one through four lanes, including half, float, and double layout even though those
types are never runtime values in the shader.

The direct route treats exact CUDA `sizeof`/`alignof` type queries as compile-time layout
semantics. It asks Slang's CUDA layout rules for the result and emits an ordinary signed-i32
constant return; it does not expose GenericAsm text to LLVM, add type-specific builder callbacks,
or claim runtime half/double/vector support.

## Progress

- [x] (2026-08-28) Reproduced the post-Slice-73 `GenericAsm` stop in
  `tests/cuda/cuda-layout.slang` and captured the final linked helper matrix.
- [x] (2026-08-28) Traced every helper to exact `GenericAsm("alignof($[0])", type)` produced by
  CUDA specialization of `core.meta.slang::__alignOf<T>()`.
- [x] (2026-08-28) Defined one structural compile-time CUDA layout-query recognizer for numeric
  scalar/vector `sizeof` and `alignof` helpers, with deterministic adjacent-shape rejection.
- [x] (2026-08-28) Emitted query results through the existing integer-constant/value-return builder
  operations and added fake structural coverage for alignment values and helper-call topology.
- [x] (2026-08-28) Added the direct lane to the existing file test and collected real PTX,
  `ptxas`, and GPU runtime evidence.
- [x] (2026-08-28) Updated durable design/ledger records, formatted the modified C++ files,
  completed the input-shape self-review, and passed the final Release validation ladder.

## Surprises and Discoveries

- The first unsupported `GenericAsm` is not arbitrary inline assembly or a runtime operation. Each
  no-parameter signed-i32 helper terminates with `alignof($[0])` and carries the queried IR type as
  operand one.
- The CUDA core module deliberately uses downstream `alignof` instead of the language `alignof`
  instruction because CUDA vector layout is not Slang's context-free natural layout. For example,
  a three-lane vector retains scalar alignment while two- and four-lane vectors have multiplied,
  capped alignment.
- Slang already has `CUDALayoutRules`, including the three-lane and 16-byte-cap rules, and the
  target-specific peephole machinery uses the same `getSizeAndAlignment` contract. Reimplementing
  the table in either the NVVM provider or tests would create a second source of truth.
- The first real comparison exposed a pre-existing mismatch between the IR CUDA layout rules and
  the CUDA prelude: `__half3` and `__half4` are explicitly 4-byte aligned, and `__half3` has 8-byte
  padded size. The older IR rule computed alignment 2/8 and size 6/8. The AST CUDA layout rule
  already models the prelude correctly, so this slice repairs the shared IR producer rather than
  patching the direct emitter.

## Decision Log

- Decision: classify exact CUDA type-layout GenericAsm structurally, then compute it with
  `IRTypeLayoutRules::getCUDA()`.
  Rationale: the GenericAsm is canonical target-selected input, but its semantic result belongs to
  Slang's CUDA ABI layout rules rather than LLVM's native vector ABI. Parsing or forwarding the text
  would be both less safe and less accurate.
  Date/author: 2026-08-28, Codex.
- Decision: support the coherent numeric scalar/vector `sizeof` and `alignof` family rather than
  adding only the first alignment spelling.
  Rationale: both core helpers carry the same type operand and differ only in which field of the
  same CUDA layout result they select. One bounded classifier and test matrix is the scalable unit;
  aggregate/pointer layout has distinct ABI questions and remains excluded.
  Date/author: 2026-08-28, Codex.
- Decision: emit an existing signed-i32 constant and valued return, with no provider ABI revision.
  Rationale: the queried type is compile-time metadata, not an LLVM runtime type. Keeping it out of
  type lowering avoids falsely claiming half/double/vector execution and keeps the shielded builder
  interface generic.
  Date/author: 2026-08-28, Codex.
- Decision: make the shared IR CUDA layout rule mirror the existing AST rule and prelude for
  three- and four-lane half vectors.
  Rationale: direct runtime comparison proved the shared rule was not the CUDA ABI source of truth
  it claimed to be. Fixing that producer also protects OptiX payload and varying legalization,
  while an emitter-local exception would preserve the latent mismatch for every other consumer.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The exact compile-time query category is implemented without a builder ABI change. A representative
fake graph returned alignments `1,4,4,16` and sizes `8,32`, used six ordinary zero-argument helper
calls/stores, and emitted no typed provider semantic operation. An aggregate query retained E52017
before provider discovery.

The first real comparison failed only for half3/half4 and identified a latent producer mismatch:
the IR CUDA rule did not mirror either the CUDA prelude or the existing AST CUDA rule. Fixing the
shared IR rule made NVRTC and direct libNVVM agree on all 28 alignments. The shader passes 2/2 on
the GPU; direct PTX has the zero-parameter kernel, 16-byte conventional global symbol, and integer
stores, and CUDA 12.9 `ptxas` accepts it for `sm_70`. The Release host and standalone provider
builds pass, and the complete NVVM prefix passes 336/336.

Self-review inventory: `_getNVVMCUDATypeLayoutQuery` survives because it classifies a canonical
target-selected type-only terminator and delegates its result to the existing CUDA layout source of
truth. The half-vector branch survives in `CUDALayoutRules` as a producer-side repair proven by the
generated `__half3`/`__half4` declarations and the NVRTC comparison. No syntax is reconstructed,
no arbitrary operand graph is searched, and the aggregate negative proves the classifier remains
bounded. The next measured conventional boundary is the multi-field global-parameter graph in
`sampler-comparison-state-unused.slang`.

## Context and Current Pipeline

Consider the representative source expansion:

```slang
outputBuffer[base * 4 + 0] = __alignOf<float>();
outputBuffer[base * 4 + 2] = __alignOf<float3>();
```

`core.meta.slang::__alignOf<T>()` selects its CUDA target branch during specialization.
`expandIntrinsicCalls` retains a zero-parameter helper returning Int whose sole block ends in
`GenericAsm("alignof($[0])", T)`. The entry calls that helper and stores the result through the
already-established one-field `RWStructuredBuffer<int>` conventional global. The final module has
28 helpers for UInt8, UInt16, Int32, Int64, Half, Float, Double, and their two-, three-, and
four-lane vectors.

Direct preflight currently rejects the first helper because the GenericAsm semantic catalog is
reserved for runtime wave/execution operations. Once the compile-time layout query is classified,
all surrounding calls, constants, resource addressing, and stores are already supported.

## Scope and Non-Goals

In scope are exact no-parameter signed-i32 helpers with GenericAsm text `alignof($[0])` or
`sizeof($[0])`, one numeric scalar/vector type operand, CUDA layout computation, constant return
emission, the existing layout shader, and negative tests for malformed or broader query shapes.

Out of scope are arbitrary GenericAsm, textual assembly transport, value-form `$T0` queries,
language `sizeOf`/`alignOf` IR instructions, aggregates, arrays, matrices, pointers, resources as
queried types, runtime half/double values, multiple conventional fields, and the offset-query graph
in `cuda-array-layout.slang`.

## Architecture and Invariants

The core module and specialization pipeline own the exact CUDA GenericAsm producer. Direct NVVM
preflight recognizes only a sole-block helper whose result is signed i32, which has no parameters,
whose terminator has exactly string plus type operands, and whose type is a numeric scalar or a
two- through four-lane numeric vector.

Slang's CUDA layout rules are the single source of truth for size and alignment. A determinate,
positive result fitting signed i32 becomes an ordinary builder integer constant followed by the
existing valued return. The queried type never enters `NVVMTypeLoweringContext`, and the provider
never sees its source spelling or LLVM handle.

The existing runtime GenericAsm catalog remains authoritative for operations with value
descriptors. Compile-time layout queries are a separate structural category and do not add a fake
feature, semantic operation ID, capability preflight row, or provider callback.

## Interfaces and Dependencies

Change only the direct emitter, its test fixtures, the existing CUDA test directive, and durable
documentation. Reuse `getSizeAndAlignment`, `IRTypeLayoutRules::getCUDA()`,
`NVVMIRBuilder::getIntegerConstant`, and `NVVMIRBuilder::emitValueReturn`.

No builder ABI, libNVVM API, LLVM API, CUDA toolkit dependency, or public Slang interface changes.

## Milestones

1. Add and unit-test a structural query classifier that distinguishes alignment from size, extracts
   the type operand, validates the exact helper signature/type family, and computes CUDA layout.
2. Integrate the classifier into preflight and emission. Prove a bounded fake fixture returns
   representative scalar/vector alignment and size constants through ordinary helper calls, with
   no intrinsic or new builder operation. Keep the full 28-query matrix in its existing file test
   instead of duplicating it in the fixed-capacity fake harness.
3. Add an adjacent-shape negative fixture for an excluded aggregate query, retaining E52017 before
   provider discovery.
4. Add a direct CUDA lane to `tests/cuda/cuda-layout.slang`; validate real PTX, `ptxas`, and runtime
   comparison with the established CUDA route.
5. Update the design and capability ledger, format, run focused and full tests, perform the
   input-shape audit, and commit Slice 74.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- fake direct emission observes representative scalar/vector alignment and size constants as
  ordinary signed-i32 helper returns, calls, and stores;
- the existing file test observes the complete CUDA alignment sequence
  `1,2,1,4, 2,4,2,8, 4,8,4,16, 8,16,8,16, 2,4,4,4, 4,8,4,16, 8,16,8,16`;
- an excluded aggregate layout query fails preflight before provider discovery/mutation;
- the existing layout shader's CUDA/NVVM comparison lanes pass on the GPU;
- direct PTX is accepted by CUDA 12.9 `ptxas` for `sm_70` and contains the zero-parameter kernel,
  16-byte global parameter symbol, and integer global stores;
- the Release host and standalone provider builds pass, as do the full
  `slang-unit-test-tool/nvvm` prefix, formatting, and `git diff --check`; and
- `external/slang-binaries/` and generated build artifacts remain unstaged.

## Failure and Recovery

If CUDA layout calculation disagrees with NVRTC/runtime, compare the exact type and the result from
`CUDALayoutRules`; do not substitute LLVM DataLayout or hardcode a correction. If additional final
IR appears after the first query is admitted, record the producer and either include it only when
it belongs to this compile-time layout bundle or leave a deterministic next boundary.

All changes are isolated to the experimental direct route and one added test lane. Removing the
classifier restores the measured `GenericAsm` stop without affecting CUDA source/NVRTC behavior.

## Artifacts and Hand-Off

Keep diagnostic IR, direct/reference PTX, and `ptxas` outputs under ignored `build/` paths. Distill
the compile-time layout-query boundary, exact accepted types, validation evidence, and next corpus
stop into `docs/design/nvvm-backend.md` and the capability ledger. Complete this plan's living
sections and self-review, then commit it with Slice 74.
