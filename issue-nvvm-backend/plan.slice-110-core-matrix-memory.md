# Carry legalized float matrices through memory and generated helpers

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct NVVM path carries the existing matrix legalization's selected Float32
matrix representation—fixed arrays of Float32 vectors—through constant buffers, generated helper
functions, local storage, whole-array copies, and dynamic row/lane addresses. The square row- and
column-major compute fixtures gain the direct runtime/PTX coverage their measured semantics support.

## Progress

- [x] (2026-08-29) Measured the core square/non-square matrix fixtures after Slice 109 and captured
  final `column-major.slang` IR.
- [x] (2026-08-29) Assigned deterministic private LLVM symbols to reachable anonymous generated
  helpers while preserving entry/export names and collision checks.
- [x] (2026-08-29) Generalized the provider's array-only element-pointer operation to selected
  sequential array or vector storage and advanced the forward-only builder ABI.
- [x] (2026-08-29) Admitted numeric-array parameter groups, immutable parameter-group array element
  addresses, whole numeric-array loads/stores, and exact local vector-lane addresses.
- [x] (2026-08-29) Added focused fake/real provider positives and adjacent negatives without
  matrix-specific builder callbacks.
- [x] (2026-08-29) Promoted the proved existing matrix fixtures, built, ran focused/full tests,
  assembled PTX, self-reviewed, and updated durable status and this plan.

## Surprises and Discoveries

- Slice 109 removes the prior array `OutParam` stop. All of `column-major.slang`,
  `row-major.slang`, `default-major.slang`, and both non-square layout fixtures then stop first at
  `function name` because matrix legalization synthesizes two reachable `mul` helpers with name
  hints but no linkage/mangled-name decoration.
- Final `column-major.slang` IR has no matrix instruction. Its values are
  `Array(Vec(Float,4),4)`. A matrix-vector helper returns `float4`; a matrix-matrix helper writes an
  `OutParam(float4[4])`; both read `ConstantBuffer(float4[4])`.
- The matrix-matrix helper dynamically addresses an array row and then a vector lane. The provider
  already emits the structurally identical typed LLVM GEP for arrays, but deliberately rejected a
  vector pointee. This is an economical generalization of one construction operation, not a new
  matrix operation.
- `kernel-context-threading.slang`, `constant-buffer-memory-packing.slang`, and
  `structured-buffer-of-matrices.slang` stop earlier on conventional fields containing larger
  structs or matrix-bearing buffers. They are separate layout/aggregate families and are not
  evidence for this slice.
- Real direct execution distinguishes the two square fixtures. `row-major.slang` produces the
  expected `11, 22, 33, 1`, while `column-major.slang` emits and assembles valid PTX but returns
  `0` instead of its expected Boolean `1`. The column-major runtime lane therefore remains
  unregistered as an explicit semantic boundary rather than being treated as compile evidence.
- Both non-square fixtures emit and assemble, but their existing comments document target-specific
  packing policy. Compilation alone is not sufficient evidence to register new runtime lanes.
- Scalar-layout generic pointers are not uniquely resource-element pointers. The first matrix run
  exposed that the old type-only resource role also matched parameter-group and local sequential
  element pointers. Resource-element validation now requires its canonical structured/raw-buffer
  producer, leaving the pointer type as a relation check rather than a role discriminator.

## Decision Log

- Decision: synthesize physical names only for reachable internal functions that lack a canonical
  entry/export/mangled name.
  Rationale: generated specialization helpers are valid canonical IR and need no externally stable
  symbol. The NVVM emitter owns their private physical symbol and can assign it deterministically
  from the already-deterministic call closure, while still rejecting collisions.
  Date/author: 2026-08-29, Codex.
- Decision: replace `emitArrayElementPointer` with one forward-only sequential element-pointer
  operation for fixed arrays and vectors.
  Rationale: LLVM typed GEP is selected by the base pointee. Exposing matrix, array, and vector
  callbacks would duplicate structure already represented by the provider handle and repeat the
  scaling problem removed from the value-operation API.
  Date/author: 2026-08-29, Codex.
- Decision: generalize the existing parameter-group classifier to scalar-struct or selected
  numeric-array elements, without admitting arbitrary copyable structs.
  Rationale: legalized top-level matrices are numeric arrays with a proved LLVM/CUDA vector layout.
  Vector-bearing structs still require a field-offset/stride proof and remain a later boundary.
  Date/author: 2026-08-29, Codex.
- Decision: register row-major runtime plus both square PTX lanes, but not column-major runtime or
  either non-square runtime lane.
  Rationale: only the row-major runtime probe demonstrates correct execution. PTX verification and
  assembly establish construction validity but cannot substitute for the missing layout evidence.
  Date/author: 2026-08-29, Codex.

## Context, Scope, and Invariants

Matrix legalization already runs before direct preflight and is the semantic source of truth. This
slice consumes only its canonical fixed-array/vector IR. It does not reconstruct row/column-major
semantics, introduce a matrix type in the provider, or infer a matrix from an arbitrary array.

In scope are selected numeric-array constant buffers, invariant element loads, first-class whole
array loads/stores, local selected vector/array allocation, exact i32-indexed local array/vector
addresses, generated internal helper names, and the existing selected scalar/vector arithmetic and
control flow used by the core fixtures.

Out of scope are matrix-bearing structs, structured buffers of matrices, nested arrays, first-class
array helper parameters/results before parameter legalization, arbitrary vector pointers or pointer
escape, writable constant-buffer addresses, non-Float32 matrix elements, new matrix arithmetic,
and layouts whose field offsets/strides are not already represented by the selected array/vector
types.

The invariants are:

- entry/export/mangled names remain authoritative; only unnamed internal closure members receive a
  private generated name, and all physical names remain unique;
- the generalized provider GEP accepts only a typed pointer to a nonempty fixed array or fixed
  vector plus an integer index, validates ownership/dominance before mutation, and does not claim
  LLVM `inbounds` provenance;
- parameter-group array elements are immutable and may only feed loads;
- local sequential GEPs preserve exact base element, access, address space, and CUDA scalar layout;
- array/vector and dimension identity comes from final IR types, never from source matrix syntax.

## Interfaces and Implementation

Advance the exact builder ABI and rename the construction callback/wrapper to a sequential element
pointer. Generalize its LLVM and fake-provider validation and update existing array callers/tests.
Generalize parameter-group type lowering around an `IRType*` element restricted to the established
scalar struct or numeric array. Add emitter resolvers for immutable parameter-group array elements
and local sequential array/vector elements, plus whole-array memory validation. Add focused compiler
coverage that composes both generated-helper-like names and all memory roles, then register proved
existing shader lanes.

## Validation and Acceptance

Acceptance requires the Release provider/compiler/unit targets; focused fake-provider and malformed
operation tests; exact new matrix runtime/PTX lanes; standalone optimized PTX; CUDA 12.9
`ptxas -arch=sm_70`; the complete `slang-unit-test-tool/nvvm` prefix; pinned clang-format; and
`git diff --check`. Record exact counts, fixture outputs, PTX/cubin sizes, and remaining boundaries.

## Self-Review and Input-Shape Audit

Inventory the anonymous-name fallback, generalized parameter-group classifier, two pointer
resolvers, whole-array memory widening, and provider operation rename. Trace each from the exact
final `column-major.slang` producer. Confirm generated functions are intentionally anonymous,
constant-buffer arrays are canonical post-legalization storage, and vector GEPs are real local
memory rather than a missed SSA legalization. Revert each widening when practical and retain no
matrix-name, fixed-dimension, or source-syntax special case.

The final inventory contains five intentional generalizations and one role correction:

- anonymous names survive because matrix legalization intentionally produces reachable functions
  without linkage names; explicit names and `SLANG_globalParams` are collected first, and the fake
  test proves an exported `__slang_nvvm_internal_0` forces collision-free generated names;
- the parameter-group classifier survives because the producer is the canonical legalized
  `ConstantBuffer<Array<Vec<Float, 4>, 4>>`, not an alternative spelling reconstructed downstream;
- local and parameter-group pointer resolvers survive because each validates the exact producer,
  pointee, result, access, address-space, layout, and i32-index relation before mutation;
- whole-array memory survives because generated `mul` helpers perform canonical first-class array
  loads/stores after out-parameter legalization, with the existing numeric-array type as source of
  truth;
- the provider rename survives because typed LLVM GEP structurally owns both fixed-array and
  fixed-vector element addresses and rejects scalar or foreign pointers;
- the resource-pointer correction survives because a scalar-layout generic pointer type alone is
  shared by valid resource, parameter-group, and local producers; requiring the canonical resource
  producer removes the accidental type-role ambiguity rather than adding a matrix exception.

No new helper rebuilds AST syntax, walks arbitrary operand graphs, recognizes a matrix source type,
or hardcodes four rows/lanes. Removing any of the anonymous-name, parameter-group, whole-array,
array-row, or vector-lane paths restores the measured first stop in `column-major.slang`.

## Failure and Recovery

If LLVM verification, libNVVM, runtime layout, or `ptxas` rejects a selected shape, preserve
generated artifacts under ignored `build/`, record the exact boundary, and narrow fixture promotion
without weakening validation. Do not add a matrix callback, guess padding, mutate upstream IR just
for this emitter, reset unrelated work, or stage `external/slang-binaries/`.

## Outcomes and Retrospective

Implementation is complete. The selected representation scaled without a matrix-shaped callback:
one sequential pointer operation now covers fixed-array rows and fixed-vector lanes, while exact IR
types still define every accepted relation. The slice also removed an adjacent artificial boundary
for dynamic Half-vector lane stores.

The row-major square fixture is the first existing matrix-memory shader to run through the direct
backend. Column-major is intentionally PTX-only because the runtime probe exposed a remaining
semantic/layout discrepancy. Non-square fixtures remain probes rather than registered evidence.

Release `slangc` and `slang-unit-test`, plus the standalone Release provider, build successfully.
The exact new row-major CUDA runtime and PTX lanes and the column-major PTX lane pass. Their PTX is
1,435 and 2,951 bytes; CUDA 12.9.86 `ptxas -arch=sm_70` emits 3,048-byte and 3,688-byte cubins. The
focused matrix, Half-lane, ABI-negotiation, and malformed sequential-addressing tests pass, and the
complete NVVM prefix passes 380/380. The pinned clang-format completed and `git diff --check` is
clean.
