# Lower selected scalar-broadcast and Boolean operation families

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM emission preserves Slang's canonical scalar-broadcast value
operations instead of rejecting them because LLVM requires equal physical operand types. Selected
integer and Float32 vector arithmetic with a same-element scalar, selected integer vector/scalar
comparisons, and selected scalar/vector Boolean `&&`, `||`, and `!` should lower through the
existing typed value-operation descriptor. The existing `tests/compute/vector-scalar-compare.slang`
and signed/unsigned builtin-operator fast-path shaders should run through direct CUDA/PTX, and
their optimized PTX should assemble with CUDA 12.9 `ptxas -arch=sm_70`.

## Progress

- [x] (2026-08-29) Reproduced `vector-scalar-compare.slang` failing at
  `Vec(Int,2) = and(Vec(Int,2), Int)` with E52017 `and`.
- [x] (2026-08-29) Measured the signed, unsigned, and Float32 builtin-operator suites failing at
  vector/scalar `mul`, `shr`, and `mul` respectively.
- [x] (2026-08-29) Traced canonical operation production, typed descriptor resolution, provider
  operand validation, and the existing numeric-family emitter.
- [x] (2026-08-29) Extend the semantic catalog with bounded scalar-broadcast compatibility and selected Boolean
  logical families without adding operation IDs or callbacks.
- [x] (2026-08-29) Materialize LLVM vector splats inside the provider after exact scalar-handle validation and
  extend the fake provider's exact typed traces.
- [x] (2026-08-29) Generalize vector element extraction to an integer SSA handle, admit the exact
  scalar `all(bool)` identity helper, and contain libNVVM's dynamic Boolean-extract defect.
- [x] (2026-08-29) Add focused positive/negative coverage and register every existing shader that crosses the
  completed family without absorbing independent matrix, half, memory, or local-variable work.
- [x] (2026-08-29) Format, build, run focused/full/CUDA validation, assemble PTX, self-review, update durable
  evidence, and prepare the plan and implementation for one `slice 91` commit.

## Surprises and Discoveries

- The first diagnostic spelling `and` is the bitwise operation from `threadInGroup & 1`, not a
  Boolean-vector operation. Final linked IR intentionally retains one vector operand and one scalar
  operand; this is the builtin operator's canonical broadcast representation.
- `_resolveNVVMValueOperation` already records independent exact result and operand descriptors.
  The ABI therefore expresses vector/scalar operations without a new enum combination, feature
  constant, operation ID, or callback.
- LLVM arithmetic/comparison builders require equal physical operand types. The provider is the
  narrow boundary that must convert the descriptor's scalar-broadcast semantics into an LLVM
  vector splat after validating the original scalar value against its exact descriptor.
- The motivating comparison shader calls the standard `all(bool2)` helper after the integer
  vector/scalar operation and vector/scalar comparison. Ordinary signed operator coverage also
  combines extracted Boolean lanes with `&&`, so selected Boolean logic is part of the same
  demonstrable path rather than an unrelated extension.
- `all(bool2)` retains a canonical finite loop with a dynamic `getElement` index. Generalizing the
  existing extraction callback's index from a host integer to an integer value handle was enough;
  no new callback or operation ID was needed.
- CUDA 12.9 libNVVM emits invalid unoptimized PTX for a direct dynamic extraction from `<N x i1>`:
  `ptxas` reports an argument mismatch because `selp.u32` receives a byte register rather than a
  predicate. Constant-lane extracts plus typed selects preserve the fixed-vector semantics and
  assemble at `-O0`.
- The scalar helper called by the vector reduction is exactly `GenericAsm("bool($0)")` with
  `Func(Bool, Bool)`. This is a checked identity body, not a new execution intrinsic.

## Decision Log

- Decision: preserve vector/scalar operand descriptors through the builder ABI and form splats in
  the LLVM provider.
  Rationale: final IR and `SlangNVVMValueOperationDesc` already preserve the semantic source of
  truth. Rewriting the Slang value into a synthetic vector before the callback would erase the
  broadcast contract and duplicate LLVM-specific operand normalization in the compiler.
  Date/author: 2026-08-29, Codex.
- Decision: generalize the existing integer-binary, integer-compare, and Float32-binary families
  rather than add per-operation broadcast families.
  Rationale: compatibility is a type/lane relation orthogonal to add/subtract/multiply/etc. The
  existing family switch already selects the correct signed and operation semantics.
  Date/author: 2026-08-29, Codex.
- Decision: map canonical Boolean `And`, `Or`, and `Not` IR to the existing bitwise operation IDs
  with explicit Boolean family validation.
  Rationale: LLVM represents Boolean logic as `i1` and `<N x i1>` bitwise operations; the operation
  IDs describe the emitted semantic exactly, while type descriptors prevent integer/Boolean
  confusion.
  Date/author: 2026-08-29, Codex.
- Decision: change the forward-only vector-element extraction callback to take an integer value
  handle instead of adding a separate dynamic-index callback.
  Rationale: constant and dynamic extraction are the same structural LLVM operation. One value
  contract composes with existing parameters, constants, and phis and removes the static-index
  special case from the shielded interface.
  Date/author: 2026-08-29, Codex.
- Decision: expand only dynamic Boolean extraction into fixed constant extracts and selects in the
  provider.
  Rationale: the input LLVM shape is canonical and verified; the malformed `selp` is produced by
  CUDA 12.9 libNVVM. The provider is the boundary that owns compatibility with that consumer, and
  the bounded two- through four-lane expansion preserves an undefined out-of-range result.
  Date/author: 2026-08-29, Codex.
- Decision: return the existing parameter for exact scalar `GenericAsm("bool($0)")` helpers.
  Rationale: the checked CUDA-prelude body is an identity. Rebuilding it as a provider operation
  would add an enum for no semantic work and obscure the existing source of truth.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The selected scalar-broadcast relation now crosses one unchanged typed value-operation boundary,
and the provider performs the only required physical normalization after exact validation. The
same work removed the static-index limitation from the construction API and carried the real
`all(bool2)` helper path through dynamic Boolean extraction and its scalar identity helper.

The signed and unsigned builtin-operator suites compiled immediately after the integer/Float32
broadcast implementation; unsigned shifts exposed the principled mixed-signedness shift-count
rule. The comparison shader then exposed dynamic extraction and the libNVVM `-O0` Boolean-extract
defect in sequence. The first unoptimized module failed `ptxas` at `selp.u32 ..., %rs9`; the
provider expansion fixed the same module without changing source IR or disabling runtime coverage.
Float32 suite probing now stops at `cmpLT`, the deliberately separate floating-comparison family.

Final validation used the pinned clang-format 17 binary, Release host/provider builds, and the
CUDA 12.9 toolkit. The complete `slang-unit-test-tool/nvvm` prefix passed 363/363. The comparison
prefix passed 4/4 with one unavailable D3D12 lane ignored; the builtin-operator prefix passed 20/20
with four unavailable D3D12 lanes ignored. Optimized direct PTX assembled for `sm_70` into 2,920-,
3,816-, and 3,432-byte cubins for comparison, signed, and unsigned shaders respectively. The
comparison shader's unoptimized PTX also assembled after the Boolean extraction workaround.

The final special-case inventory is deliberately short. `haveSameElementType`,
`hasComponentWiseLanes`, and `isComponentWiseBinary` survive as the single shared descriptor
relation; removing them restores the first vector/scalar E52017 failures. The provider's splat
helper survives because LLVM requires physically equal operands after exact semantic validation.
The exact scalar Boolean GenericAsm identity survives because the CUDA prelude intentionally
produces that valid body, and removing it restores the `GenericAsm` stop. The dynamic Boolean
extract expansion survives because reverting it reproduces `ptxas`'s concrete `selp` argument
mismatch while the same verified LLVM input succeeds when optimized. No AST, IR, type, or witness
shape is reconstructed, and the accidental Boolean-vector-construction expansion found during
self-review was reverted.

## Context and Current Pipeline

Consider the motivating source:

    int2 threadInGroup = dispatchThreadID.xy;
    if (all((threadInGroup & 1) == 0)) { ... }

Builtin operator lowering intentionally produces:

    Vec(Int,2)  = bitAnd(Vec(Int,2), Int)
    Vec(Bool,2) = equal(Vec(Int,2), Int)
    Bool        = call all(Vec(Bool,2))

`_resolveNVVMValueOperation` in `source/slang/slang-emit-nvvm.cpp` converts each result and operand
type independently with `_getNVVMSemanticType`. `resolveValueOperationFamily` in
`source/compiler-core/slang-nvvm-semantic-catalog.h` currently requires both operands to be exactly
the result/vector type, so preflight rejects the first instruction before provider discovery.

When admitted, `NVVMIRBuilder::emitOperation` passes the unchanged exact descriptor and handles to
`_emitOperation` in `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`. `_emitNumericFamily` validates
each handle against its declared LLVM scalar/vector type. It must then splat only the scalar
operand to the operation's vector width before calling LLVM `CreateAdd`, `CreateAnd`, `CreateICmp`,
and related builders. The original scalar handle and descriptor remain the validation source of
truth.

## Scope and Non-Goals

In scope:

- signed/unsigned selected integer binary operations already in `IntegerBinary`, with scalar or
  same-element vector operands whose lane counts are one or the result lane count;
- selected Float32 binary operations already in `FloatBinary` under the same bounded rule;
- selected integer equality/order comparisons where scalar operands broadcast to the Boolean
  result lane count;
- selected Bool scalar/vector `And`, `Or`, and `Not`, including scalar broadcast for binary logic;
- integer-SSA indexing for selected fixed-vector extraction, the exact scalar Boolean identity
  helper reached by `all(bool2)`, and a provider-side workaround for libNVVM's dynamic i1-vector
  extraction defect;
- exact real/fake provider validation, focused invalid-descriptor coverage, and representative
  existing shader runtime/PTX/assembler lanes.

Out of scope:

- matrices, cooperative vectors, structs, arrays, half/double arithmetic, or implicit element-type
  conversion inside an operation;
- floating-point comparisons, vector/scalar conversion operations, reductions as new provider
  intrinsics, or arbitrary lane reshaping;
- division/remainder-by-zero policy changes, overflow/saturation decorations, fast-math flags, or
  a new public builder callback/ABI revision;
- local `var` support or other independent boundaries exposed by broader existing shaders.

## Architecture and Invariants

- The linked IR vector/scalar shape is canonical and remains unchanged. Result and operand
  descriptors retain exact kind, bit width, and lane count through preflight and emission.
- A broadcast-compatible operand has the same semantic element kind and bit width as the operation
  element and either one lane or exactly the operation lane count. At least one binary operand must
  already have the operation lane count; scalar/scalar cannot claim a vector result.
- A shift result and left operand remain exact. Its integer count may differ in signedness but not
  physical bit width, and has either one lane or the result lane count.
- Result lane count remains in the selected range one through four. The provider validates each
  incoming handle against its original descriptor before materializing any splat.
- Integer signedness continues to select signed divide/remainder/right-shift/order predicates;
  equality and bitwise operations remain sign-independent.
- Boolean descriptors never enter integer families, and integer descriptors never enter Boolean
  families. No implicit element-type conversion is performed.
- Unsupported descriptors stop at capability preflight or return `SLANG_E_INVALID_ARG` without
  provider IR mutation.

## Interfaces and Dependencies

No callback or operation enum is added. The forward-only construction interface changes
`emitVectorElementExtract` from a host integer index to an integer value handle; existing
`SlangNVVMValueOperationDesc::{resultType,operandTypes,operandCount}` fields express the contract.
Internal changes are expected in:

- `source/compiler-core/slang-nvvm-semantic-catalog.h` for shared family resolution;
- `source/slang/slang-emit-nvvm.cpp` for Boolean IR mapping and exact preflight/emission;
- `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` for validated LLVM splats and Boolean emission;
- `tools/slang-unit-test/` for exact fake-provider descriptors and focused positive/negative tests;
- selected existing shader directives and the durable NVVM design/capability ledger.

Validation uses the configured Release host build, the standalone provider at
`build/nvvm-builder-deps/slang-llvm-nvvm-build/Release`, CUDA 12.9 runtime/compiler components, and
`ptxas -arch=sm_70`. Builds and tests run outside the sandbox per repository instructions.

## Milestones

1. Add shared, documented semantic helpers for selected scalar/vector element compatibility and
   resolve integer, Float32, comparison, and Boolean operation descriptors table-wise.
2. Add provider-side exact operand normalization that splats only validated scalar operands to the
   resolved vector operation type, then emit existing LLVM arithmetic/comparison/logic operations.
3. Extend fake-provider recording and table-driven unit coverage across representative signed,
   unsigned, Float32, comparison, and Boolean shapes; reject mismatched kinds, widths, lanes, and
   scalar/scalar-to-vector descriptors without mutation.
4. Generalize vector extraction to integer SSA, carry the motivating `all(bool2)` helper, and
   contain the measured libNVVM dynamic-Boolean-extract translation defect.
5. Reprobe the motivating and operator-suite shaders, add direct runtime/PTX lanes only to shaders
   whose next boundary belongs to this slice, and record later independent boundaries.
6. Format, build host/provider, run focused and complete NVVM tests, run CUDA-scoped shader tests,
   assemble emitted PTX, update durable evidence, complete the input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires:

- table-driven semantic/provider coverage for both vector-scalar and scalar-vector representatives;
- focused direct-emitter traces proving exact heterogeneous operand descriptors reach the provider;
- negative descriptors covering element-kind/width/lane mismatch and vector result with two scalar
  operands, with no output handle or provider mutation;
- `tests/compute/vector-scalar-compare.slang` direct runtime/PTX success;
- direct success for the signed and unsigned builtin-operator suites if no independent boundary
  remains, while broader Float32 matrix/half boundaries are recorded rather than absorbed;
- the complete `slang-unit-test-tool/nvvm` prefix and CUDA-scoped changed-shader regressions;
- CUDA 12.9 `ptxas -arch=sm_70` acceptance and `git diff --check`.

## Failure and Recovery

The shared semantic resolver is used by compiler preflight, fake validation, and the LLVM provider;
a mismatch should first be reduced to one descriptor and checked consistently across all three.
Provider failures must be tested for mutation before keeping any normalization. Generated probes
remain under ignored `build/` paths and can be regenerated safely. If an existing shader reaches a
matrix, half, memory, local-variable, or intrinsic boundary after broadcast succeeds, record it for
a later slice instead of widening this one. Never reset unrelated work or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Retain exact focused descriptor traces, shader diagnostics before/after, runtime outputs,
representative PTX instructions, PTX/cubin sizes, full test counts, and the self-review/input-shape
audit in this plan. Distill settled operation-family contracts into `docs/design/nvvm-backend.md`
and shader/unit capability evidence into `docs/design/nvvm-backend-capability-ledger.md`. Per the
user's work-loop instruction, commit this completed plan with the implementation using first commit
line `slice 91`.
