# Admit selected vectors throughout the helper-function ABI

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM emission accepts the already-supported selected value vectors as
helper-function parameters and results, call arguments and results, return values, and basic-block
parameters. This should remove the common helper-parameter boundary measured in the existing
vector comparison and dot-product shaders. At least one existing shader from each family must run
through direct CUDA/PTX, optimized PTX must assemble with CUDA 12.9 `ptxas -arch=sm_70`, and
unsupported vector element types or lane counts must still stop before provider mutation.

## Progress

- [x] (2026-08-29) Measured five existing compare/dot shaders and found the same `helper function
  parameter` preflight boundary.
- [x] (2026-08-29) Audited type-role admission, call/return validation and emission, block
  parameters, the real LLVM provider, and the fake provider.
- [x] (2026-08-29) Generalized the selected helper-value source of truth and applied it consistently to signatures,
  calls, returns, and control-flow transport.
- [x] (2026-08-29) Made real and fake generic phi/function validation retain and compare exact selected vector
  types without combination enums.
- [x] (2026-08-29) Added focused positive and negative unit coverage, then absorbed only directly related canonical
  vector-transport boundaries exposed by the measured shaders.
- [x] (2026-08-29) Registered representative existing shaders for direct CUDA/PTX evidence,
  formatted, built, ran focused and full validation, assembled PTX, self-reviewed, and prepared the
  completed plan and implementation for one commit.

## Surprises and Discoveries

- The public builder already has generic function-type, call, and value-return operations, and the
  LLVM provider admits fixed integer/float vectors with two through four lanes there. No callback
  or ABI addition is needed for straight-line helper transport.
- The current `uint3` helper result exception exists for execution-register access, while every
  other vector is rejected by Slang-side role checks. The existing selected-value-vector
  classifier subsumes that exception and gives one forward-looking contract.
- Generic phi callbacks exist, but both the direct emitter and providers currently narrow them to
  scalar float/integer values. A complete helper-value contract should extend those callbacks to
  the same selected vectors rather than leave control flow as an accidental hole.
- The fake provider records exact function parameter handles already, but still classifies results
  and phis with scalar/`UInt3` enums. Those enums can be replaced or complemented by exact handles
  instead of adding every vector combination.
- The emitter's signed-i32 phi convenience methods already forward to the same generic provider
  callback. The fake provider had been delegating generic integer phis back into obsolete integer
  trace storage, which made old unit assertions look like a distinct provider path existed. Exact
  generic trace storage is the truthful model for scalar and vector phis alike.
- After helper transport was admitted, `vector-dot-unroll.slang` and `vector-dot-int.slang`
  compiled directly without any adjacent production fix. The other three probes exposed separate
  later boundaries at vector logical `and`, canonical `GenericAsm`, and local `var` storage, so
  they were not folded into this slice.

## Decision Log

- Decision: define helper values as `void` results or the existing selected scalar/vector value
  family, rather than adding individual vector signatures.
  Rationale: `asNVVMSupportedValueVectorType` is already the canonical value-type classifier and
  LLVM function types preserve exact vector element type and lane count.
  Date/author: 2026-08-29, Codex.
- Decision: include basic-block parameter/phi transport in this slice.
  Rationale: block parameters are the canonical SSA representation across branches. A helper ABI
  that accepts vectors but rejects the same value after ordinary control flow would be incomplete
  and would push future fixes into shader-specific lowering.
  Date/author: 2026-08-29, Codex.
- Decision: do not add a builder callback or per-combination feature constant.
  Rationale: the generic operations and exact type handles already express the required contract.
  Date/author: 2026-08-29, Codex.
- Decision: record all fake-provider phis through the exact generic callback rather than preserving
  the old signed-integer trace category.
  Rationale: there is one provider callback after the forward-only interface cleanup. Exact type
  handles distinguish Int, Bool, Float, and every selected vector without simulating a callback
  that no longer exists.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

`isNVVMSupportedValueType` now defines the selected helper/control-flow value family once. Helper
signatures, calls, returns, block parameters, branch arguments, type lowering, and provider phis
all preserve exact canonical types. The former UInt3 result exception and fake result/phi/call
combination classifications are gone; no public builder interface changed.

Focused source demonstrates Int4 call/result and conditional block-parameter transport, Float3
call/result transport, and comparison-produced Bool2 call/result transport. Double2 reports
E52017 before builder discovery or module mutation. A five-lane vector is rejected by the Slang
front end before backend selection. Removing selected-vector helper admission restores the common
`helper function parameter` failure in the motivating dot shaders, while removing generic vector
phi admission fails the focused Int4 conditional helper. These are canonical final-IR values, not
alternative producer spellings: `IRFunc` signatures provide the source-of-truth types, `IRCall`
arguments match them exactly, and positional branch arguments match exact block-parameter types.
The emitter therefore owns admission and exact transport; no syntax or semantic shape is rebuilt.

Validation evidence:

- Release `slang-test`/`slangc` and standalone Release provider targets build successfully.
- The focused scalar-SSA and vector-function traces pass 2/2; the complete
  `slang-unit-test-tool/nvvm` prefix passes 363/363.
- CUDA-scoped `vector-dot-unroll` and `vector-dot-int` regressions pass 12/12, with five unrelated
  API lanes ignored. The direct `vector-dot-int` runtime result is `-14, 28, 20, 5`.
- Direct PTX sizes are 1,854 bytes for `vector-dot-unroll`, 1,326 bytes for `vector-dot-int`, and
  478 bytes for the focused vector-helper source. CUDA 12.9.86 `ptxas -arch=sm_70` accepts all
  three and produces 3,432-, 3,048-, and 2,664-byte cubins respectively.
- The pinned clang-format 17 binary and `git diff --check` pass.

Self-review inventory: the production `isNVVMSupportedValueType` helper survives as the single
value-family classifier; the provider's existing `_isSupportedFunctionValueType` survives because
it validates native LLVM type/context identity at the ABI boundary; and the fake provider's exact
`_isFakeNVVMBuilderValueOfType` survives because the test double must reject mismatched handles.
The former UInt3 predicate and generic-to-integer-phi trace fallback were removed. No new fallback,
graph walk, reconstructed semantic value, target-name special case, or silent impossible-shape
guard remains.

## Context and Current Pipeline

Final linked IR for the measured shaders contains ordinary helper functions whose parameters are
canonical vectors such as `vector<float,3>` or `vector<int,4>`. `_validateNVVMHelperTarget` rejects
those signatures before a provider module exists even though `NVVMTypeLoweringContext` can lower
the same vector in the `Value` role and the provider's generic function operations can carry it.

Calls and returns currently validate only scalar SSA values. Non-entry block parameters similarly
accept selected integer scalars or float32, and emission chooses the frozen integer-phi operation
for everything except float32. These are consumer-side restrictions over otherwise canonical IR;
the producer is already expressing exact value types and positional branch arguments.

## Scope and Non-Goals

In scope:

- selected signed/unsigned integer vectors, bool vectors, and float32 vectors with two through four
  lanes as helper parameters/results;
- exact call, return, block-parameter, branch-argument, and phi transport for that family;
- exact type identity in real/fake provider validation;
- representative existing compare/dot shaders through direct runtime, PTX, and assembler lanes.

Out of scope:

- vector entry-point parameters/results or arbitrary device vector pointers;
- matrices, structs, arrays, half/double vectors, vectors with one or more than four lanes;
- dynamic vector indexing or new arithmetic/intrinsic semantics unrelated to transport;
- indirect calls, recursion, varargs, or changes to Slang's canonical call/block representation.

## Architecture and Invariants

- `asNVVMSupportedValueVectorType` remains the one vector classifier for helper signatures and SSA
  transport.
- Every call argument and return value must exactly match the canonical callee/function type and
  dominate its use.
- Branch arguments must exactly match positional block parameters; generic phis preserve that exact
  type and accept only usable incoming values from actual predecessor blocks.
- Signed-i32 convenience methods and generic builder methods share the same provider callbacks;
  exact type handles, rather than separate callback families, define the contract.
- Unsupported signature and transport shapes fail preflight before provider module creation.

## Interfaces and Dependencies

No public builder API change is planned. The existing generic `getFunctionType`, `emitCall`,
`emitValueReturn`, `emitPhi`, and `addPhiIncoming` operations gain a consistent selected-vector
contract in their implementations and test double. Production changes are limited to the direct
emitter/type lowering and the standalone `slang-llvm-nvvm` provider.

Validation uses the existing standalone provider under
`build/nvvm-builder-deps/slang-llvm-nvvm-build/Release`, the configured Release build, CUDA 12.9
runtime/compiler components, and `ptxas -arch=sm_70`.

## Milestones

1. Generalize helper signature and SSA validators around the existing selected value-vector
   classifier.
2. Route vector block parameters through generic phi emission and extend the real provider's exact
   generic phi checks.
3. Replace fake-provider scalar/`UInt3` assumptions with exact function/phi type handles and add
   focused trace assertions.
4. Reprobe the existing compare/dot shaders, implement only adjacent vector-transport gaps, and
   register representative direct lanes.
5. Format, build, validate, update durable capability evidence, complete the input-shape audit, and
   commit.

## Validation and Acceptance

Acceptance requires:

- focused positive coverage for selected integer, bool, and float vector helper transport,
  including a vector block parameter;
- focused negative coverage proving double vectors and unsupported lane counts stop before builder
  discovery or mutation;
- the complete `slang-unit-test-tool/nvvm` prefix;
- representative existing compare/dot shader runtime and PTX lanes;
- CUDA-scoped regression covering any changed test files;
- `ptxas -arch=sm_70` acceptance for emitted PTX;
- no public builder ABI additions and no regression in scalar helper functions.

All CMake builds and tests run outside the sandbox as required by repository instructions.

## Failure and Recovery

Changes are additive to selected helper-value admission and generic operation validation. Failed
builds/tests can be rerun after rebuilding the main compiler and standalone provider. If a measured
shader exposes unrelated arithmetic, matrix, memory, or intrinsic work, record that as a later
slice boundary rather than broadening this transport slice. Never reset unrelated work or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Keep final command counts, runtime results, representative PTX, assembler evidence, the exact
shader boundary measurements, and the self-review/input-shape audit in this plan. Update the
durable NVVM design/capability documents if they track these contracts. Per the user's work-loop
instruction, commit this plan only after completion with the implementation, using first commit
line `slice 90`.
