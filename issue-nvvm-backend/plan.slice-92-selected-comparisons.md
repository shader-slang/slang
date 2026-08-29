# Lower selected Float32 and Boolean comparisons

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM backend accepts the ordinary scalar/vector Float32 comparison
family and Boolean equality/inequality through the existing typed operation descriptor. Canonical
flat Boolean-vector constructors used by existing Slang shaders also cross the established vector
construction API. The Float32 and Boolean builtin-operator fast-path shaders should advance through
these comparisons under direct CUDA/PTX; any next independent matrix, conversion, or memory
boundary is measured and left to a later slice.

## Progress

- [x] (2026-08-29) Reproduced the Float32 suite stopping at `cmpLT` and the Boolean suite stopping
  at `cmpEQ` after Slice 91.
- [x] (2026-08-29) Traced scalar floating predicates, parameterized descriptor resolution, provider
  broadcast normalization, and numeric-only vector construction.
- [x] (2026-08-29) Add bounded Float32 comparison and Boolean equality families without new operation IDs or
  builder callbacks.
- [x] (2026-08-29) Admit exact selected Boolean-vector construction and extend fake-provider type validation.
- [x] (2026-08-29) Add economical table-driven positive/negative coverage and focused emitter traces.
- [x] (2026-08-29) Reprobe the two existing shader suites and register only the direct lanes completed here.
- [x] (2026-08-29) Format, build, run focused/full/CUDA validation, assemble PTX, self-review, and prepare the slice commit.

## Surprises and Discoveries

- The exact scalar catalog already defines the required IEEE predicate policy: ordered equality and
  ordering, but unordered inequality. The parameterized family should share that policy rather than
  invent vector-specific semantics.
- The Boolean shader constructs `bool4` values explicitly before comparing them. This is a canonical
  flat constructor; the compiler currently rejects it only because `_getNVVMVectorConstruction`
  applies the older numeric-vector gate even though extraction and provider LLVM types already
  support Boolean vectors.
- A constant Float3 comparison initially gave false end-to-end confidence because final linking
  folded it. Making one scalar input depend on the output buffer preserved the real vector `fcmp`
  and exposed libNVVM rejecting LLVM 14's dynamic-splat `poison` token at line 15, column 44.
- LLVM 14 `CreateVectorSplat` prints `insertelement ... poison` followed by `shufflevector`.
  libNVVM's LLVM 7 reader does not know `poison`; constructing every bounded lane from `undef`
  compiles and produces three `setp.lt.f32` instructions in the measured PTX.

## Decision Log

- Decision: add `FloatCompare` and `BooleanCompare` as type-resolved semantic families while
  retaining the existing comparison operation IDs.
  Rationale: result/operand descriptors already express kind, width, and lanes. A separate callback
  or predicate-combination enum would duplicate that source of truth.
  Date/author: 2026-08-29, Codex.
- Decision: restrict Boolean comparisons to equality and inequality.
  Rationale: Boolean ordering is not a selected Slang builtin contract, while LLVM integer ordering
  would accidentally make unsupported descriptors appear meaningful.
  Date/author: 2026-08-29, Codex.
- Decision: generalize the existing vector-construction gate from numeric to selected ordinary
  value vectors instead of adding a Boolean-specific path.
  Rationale: the constructor shape and construction callback are element-type-generic already; only
  the compiler and fake provider retain the bring-up restriction.
  Date/author: 2026-08-29, Codex.
- Decision: materialize a dynamic scalar broadcast as two through four explicit inserts from
  `undef`, rather than textually replace `poison` or use LLVM 14 `CreateVectorSplat`.
  Rationale: the provider owns the physical normalization and can directly generate IR that is
  valid in LLVM 14 and LLVM 7. A broad textual token rewrite would operate after semantic context
  was lost, while every inserted lane precisely defines the intended splat.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Selected Float32 comparisons and Boolean equality/inequality now share the existing exact typed
operation boundary. Float32 vector/vector plus both scalar-broadcast directions preserve the six
scalar-catalog predicates, while Boolean vector/vector plus both broadcast directions are limited
to equality and inequality. The fake boundary records exact result and operand descriptors, and
the real provider's normal and compatible assembly contains all six `fcmp` predicates plus Boolean
`icmp eq/ne`.

The Boolean-vector construction restriction was an isolated bring-up gate rather than an upstream
representation defect. Generalizing it to the already-defined ordinary value-vector classifier
lets the unchanged constructor API carry exact Bool2/3/4 values. The existing Boolean builtin
operator suite consequently passes its CPU, Vulkan, direct CUDA runtime, and direct PTX lanes.

The runtime Float3 probe exposed a second, independent producer/consumer mismatch: LLVM 14 emitted
`poison` for a dynamic scalar splat, and libNVVM's LLVM 7 parser stopped before seeing the new
comparison. Replacing `CreateVectorSplat` with exact inserts from `undef` fixes the physical producer
without a textual token fallback. The CUDA vector suite passes 4/4 with 40 results; optimized direct
PTX contains three `setp.lt.f32` instructions and assembles into a 3,816-byte cubin. The Boolean PTX
is 1,216 bytes and assembles into a 3,048-byte cubin. The broader Float32 suite now stops at the
deliberately separate `makeMatrix` boundary.

Release host and standalone-provider builds pass. The complete NVVM prefix passes 363/363 after the
semantic/provider change; the final broadened real/fake comparison tests pass 1/1 each. Changed
shader prefixes pass 8/8 with one unavailable D3D12 lane ignored, and CUDA 12.9.86
`ptxas -arch=sm_70` accepts both final modules.

The final special-case inventory contains one shared predicate mapper and one bounded physical
broadcast constructor. `_getFloatingComparePredicate` survives because scalar catalog dispatch and
the dimensioned family must have one predicate source of truth. `_materializeBroadcastOperand`
survives because LLVM operations require physically equal operands and its explicit inserts avoid
an actually measured LLVM-14/LLVM-7 dialect mismatch. No AST/IR/type shape is reconstructed, no
fallback equivalence was introduced, and removing either family resolver restores the original
`cmpLT`/`cmpEQ` stops. Invalid Float lane shapes and Boolean ordering remain rejected with a null
output before provider mutation.

## Context and Current Pipeline

Consider these existing shader expressions:

    bool4 floatComparison = float4(a, b, a + b, a - b) < float4(a, b, a + b, a - b) * 2.0;
    bool4 booleanComparison = bool4(p, q, p, q) == bool4(q, q, p, p);

Builtin lowering produces component-wise compare instructions whose result is `Vec(Bool,N)` and
whose operands retain their exact scalar/vector types. `_resolveNVVMValueOperation` in
`source/slang/slang-emit-nvvm.cpp` builds one `SlangNVVMValueOperationDesc`.
`resolveValueOperationFamily` in `source/compiler-core/slang-nvvm-semantic-catalog.h` currently
recognizes integer comparisons but not the analogous Float32 or Boolean shapes. The provider's
`_emitValueOperationFamily` already validates original handles and splats selected scalar operands
before emission, but it lacks these two comparison cases.

The Boolean operands are produced by canonical `makeVector` instructions. The shared compiler
constructor resolver currently calls `asNVVMSupportedNumericVectorType`, while the adjacent element
extract resolver and the provider's semantic LLVM type mapper accept Boolean vectors. The fake
provider similarly recognizes integer and Float vector type handles but omits Boolean.

## Scope and Non-Goals

In scope:

- all six selected Float32 scalar/vector predicates with one- through four-lane results and
  component-wise scalar broadcast;
- Boolean equality and inequality for one- through four-lane values, including canonical scalar
  broadcast where final IR retains it;
- exact flat/splat/swizzle construction of selected ordinary Boolean vectors through the existing
  construction callback;
- real/fake provider validation and direct shader/PTX evidence.

Out of scope:

- Float64, half, matrices, cooperative vectors, arrays, structs, or mixed-element comparisons;
- Boolean ordering, implicit conversions, reductions, or new operation IDs/callbacks;
- unrelated matrix/conversion/storage boundaries exposed after the comparison suites advance.

## Architecture and Invariants

- Final Slang IR descriptors remain the semantic source of truth; no synthetic IR vector is built
  in the compiler to normalize broadcast.
- Float predicates exactly match the scalar catalog: `oeq`, `une`, `olt`, `ogt`, `ole`, and `oge`.
- A comparison result is Boolean with the operand operation lane count. Both operands have the same
  semantic element type and each has either one lane or that result width; two scalars cannot claim
  a vector result.
- Boolean constructors require exact Boolean scalar elements and two through four lanes. Numeric
  construction behavior remains unchanged.
- Unsupported descriptors fail capability resolution or return invalid/not-available without an
  output handle or provider mutation.

## Interfaces and Dependencies

No public API or callback changes. Internal changes are expected in the semantic catalog, LLVM and
fake providers, compiler vector-construction classification, table-driven builder/emitter tests,
the two existing shader directives, and durable NVVM design/capability documents. Validation uses
the configured Release host build, standalone provider, CUDA 12.9, and `ptxas -arch=sm_70`; builds
and tests run outside the sandbox per repository instructions.

## Milestones

1. Add the two semantic families using the existing shared component-wise descriptor relation.
2. Emit Float predicates from one shared mapper and Boolean equality through LLVM integer compare.
3. Generalize canonical value-vector construction and exact fake-provider Boolean type checks.
4. Extend table-driven builder and direct-emitter tests with valid scalar/vector/broadcast shapes
   and invalid kind/width/lane/predicate cases.
5. Reprobe existing Float32/Boolean operator suites, register completed direct lanes, and record the
   next independent diagnostics.
6. Format, build, run the full NVVM prefix and changed shader prefixes, assemble emitted PTX,
   perform the input-shape/special-case audit, update this plan and durable docs, and commit.

## Validation and Acceptance

Acceptance requires focused real-provider IR checks for all Float predicates and Boolean equality,
fake-provider exact descriptor traces, negative no-mutation coverage, direct compilation/runtime
for every completed existing shader lane, the complete `slang-unit-test-tool/nvvm` prefix, CUDA
12.9 PTX assembly, pinned clang-format 17, and `git diff --check`.

## Failure and Recovery

Reduce failures to one operation descriptor and compare the shared resolver, fake recorder, and LLVM
provider response. Generated probes stay under ignored `build/` paths. If a suite advances to a
matrix, conversion, memory, or unrelated helper boundary, record it instead of widening this slice.
Never reset unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record before/after diagnostics, exact descriptor traces, representative LLVM/PTX predicates,
runtime output, PTX/cubin sizes, test counts, and the self-review audit here. Distill settled type
contracts into `docs/design/nvvm-backend.md` and durable capability evidence into
`docs/design/nvvm-backend-capability-ledger.md`.
