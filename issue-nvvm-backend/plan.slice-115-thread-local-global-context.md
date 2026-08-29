# Lower thread-local globals into explicit kernel context

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct NVVM route runs Slang's established CUDA global-context lowering for
ordinary HLSL/Slang module variables. Initializers move into each entry point, plain thread-local
globals become fields of one local context, and reachable helpers receive the context explicitly.
A focused existing-style runtime fixture should pass direct runtime and PTX lanes with independently
initialized per-invocation state. The existing
`tests/compute/logic-no-short-circuit-evaluation.slang` probe should advance through its global,
initializer, helper-context, and field-address shapes to its next unrelated boundary.

## Progress

- [x] (2026-08-29) Traced Slice 114's `device scalar pointer` stop to an unlowered plain
  module-scope `static int result = 0`.
- [x] (2026-08-29) Compared the existing CUDA-source route and confirmed it creates a per-invocation
  `KernelContext`, initializes its `result` field to zero in `computeMain`, and passes its address
  to `assignFunc`; it does not emit a mutable device global.
- [x] Run the same established producer for direct NVVM with CUDA semantics, without changing the
  provider global-storage ABI or teaching the emitter to reinterpret the global.
- [x] Admit the producer's exact explicit thread-local context-pointer spelling at helper
  boundaries while retaining the same selected scalar-struct pointee contract.
- [x] Add focused positive coverage, validate runtime/PTX
  and `ptxas`, run the full NVVM prefix, document, format, self-review, and commit.

## Surprises and Discoveries

- An HLSL/Slang plain global variable is semantically thread-local on CUDA-like targets. The final
  direct IR's module-level `global_var Ptr<Int>` was therefore not a valid device-global input
  shape for emission; it was evidence that direct PTX skipped the upstream context pass.
- The provider's current `declareGlobalStorage` would be wrong twice: it creates a true LLVM global
  shared by threads and initializes internal definitions with `undef`. Supporting the diagnostic at
  the emitter would violate both storage lifetime and the source's explicit zero initializer.
- The established context pass gives a helper an explicit
  `Ptr<KernelContext, ReadWrite, ThreadLocal, DefaultLayout>` parameter but passes the compact
  entry-local `Ptr<KernelContext>` to it. This is intentional producer output. Both spellings map
  to the provider's typed generic pointer, but the initial helper classifier admitted only compact
  local pointers and `BorrowInOutParam`.
- After that exact pointer form was admitted, the original short-circuit fixture advanced to
  `GenericAsm("bool($0)")` inside scalar `all(int)` and `all(bool)` helpers. This is an independent
  intrinsic-lowering boundary, so a focused context fixture supplies this slice's end-to-end test.

## Decision Log

- Decision: fix the missing producer pass rather than add initialized mutable device globals.
  Rationale: CUDA-source emission already establishes the correct representation and observable
  behavior. Reusing it keeps the direct emitter simple and gives each invocation independent state.
  Date/author: 2026-08-29, Codex.
- Decision: invoke explicit-global-context lowering with `CodeGenTarget::CUDASource` policy even
  though the requested artifact target is PTX.
  Rationale: the policy describes CUDA execution semantics: plain globals move, groupshared and
  actual globals remain storage objects, and conventional CUDA global parameters stay global.
  The PTX enum's default policy would incorrectly move groupshared values as well.
  Date/author: 2026-08-29, Codex.
- Decision: accept only the producer's complete read-write, thread-local, default-layout context
  pointer alongside the existing compact local and mutable-borrow forms.
  Rationale: the source pointer distinction is intentional and exact, while all three selected
  scalar-struct forms share one typed generic LLVM representation. Broadly accepting decorated or
  arbitrary-address-space pointers would erase a meaningful preflight boundary.
  Date/author: 2026-08-29, Codex.
- Decision: add a focused per-invocation-state fixture rather than absorb `GenericAsm` into this
  slice.
  Rationale: scalar `all()` is unrelated to global representation. Keeping that measured stop for
  the next slice makes this slice's runtime evidence isolate initialization and context threading.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The direct PTX route now invokes the same two established producers as CUDA-source emission at the
existing global-context pipeline phase. The CUDA-source policy preserves the exact execution
classification: the focused plain global becomes an initialized per-entry context field, while the
complete NVVM regression set confirms groupshared and actual global storage remain intact.

The only emitter-side widening is the context pass's canonical helper pointer form. Fake-provider
coverage proves one local scalar context, zero provider global declarations, one explicit context
pointer parameter, and the entry-local pointer passed at the call. No builder ABI, provider
callback, global initializer, textual rewrite, or second global representation was added.

The focused runtime fixture returns `7, 8, 9, 10`, which distinguishes per-invocation initialization
from device-wide accumulation. Its PTX is 650 bytes; CUDA 12.9.86
`ptxas -arch=sm_70` emits a 2,792-byte cubin. The focused unit and both shader lanes pass, and the
complete NVVM prefix passes 383/383. The original short-circuit fixture now reaches only its
separate scalar-`all` `GenericAsm` boundary.

## Context and Current Pipeline

The shared link/legalization pipeline already called `moveGlobalVarInitializationToEntryPoints`
followed by `introduceExplicitGlobalContext` for CUDA source and LLVM-like targets. Direct NVVM is
selected under the PTX target and previously fell through the switch without either pass, so its
preflight saw the pre-context global pointer. Prior helper/local-aggregate slices already supported
the canonical context struct, compact local pointer, field address, initialization store, and typed
generic provider pointer. This slice closes the remaining exact explicit helper-parameter spelling.

## Scope and Non-Goals

In scope are ordinary non-`ActualGlobal` CUDA thread-local globals whose resulting explicit context
is already within the selected flat copyable-struct/local-pointer/helper ABI; initializers that
lower to established selected values; multiple reachable helper uses; and preserving groupshared,
conventional parameter, varying builtin, and actual-global handling.

Out of scope are true mutable device globals, new provider storage initializers, thread-group
globals, ray-tracing payload globals, nested/noncopyable context fields, multi-entry dispatch,
dynamic initialization outside existing lowering, and widening helper/local aggregate contracts
beyond the exact context-pointer form produced here.

## Architecture and Invariants

- `moveGlobalVarInitializationToEntryPoints` owns when and where source initializers execute.
- `introduceExplicitGlobalContext` owns classification of plain, groupshared, ray-tracing, actual,
  varying, and uniform globals. Direct NVVM supplies CUDA policy rather than duplicating it.
- No ordinary plain `IRGlobalVar` may remain in the direct emitter. Existing exact shared globals
  and conventional/execution globals retain their established representations.
- The direct emitter accepts only the context shapes already covered by generic type, local storage,
  field pointer, helper pointer, load/store, and call contracts.

## Interfaces and Dependencies

No builder ABI change is required. The slice changes shared pipeline selection and reuses the
existing provider/facade operations. The fake provider and final IR dumps establish the exact
producer-consumer trace; CUDA-source output supplies differential representation evidence; CUDA
12.9 runtime and `ptxas` supply external evidence.

## Milestones

1. Run initializer motion and explicit context lowering only on the direct route, using CUDA-source
   classification policy, at the established global-context phase.
2. Verify final IR contains a local flat context and explicit helper pointer rather than the plain
   global. Add focused fake coverage and retain exact rejection of other pointer shapes.
3. Add a focused runtime fixture, validate exact output and PTX, assemble with `ptxas`, run
   the complete NVVM prefix, update durable status, format, self-review, and commit.

## Validation and Acceptance

Acceptance requires final-IR evidence of producer ownership; Release compiler/unit/test builds;
focused fake-emitter coverage; focused exact runtime/PTX lanes; CUDA 12.9
`ptxas -arch=sm_70`; the full `slang-unit-test-tool/nvvm` prefix; pinned formatting; and
`git diff --check`.

Completed evidence:

- Release `slangc`, `slang-unit-test`, and `slang-test` targets build successfully; no provider
  rebuild is needed because its ABI and implementation did not change.
- `nvvmSlangThreadLocalGlobalUsesExplicitContext` passes with zero global declarations, one local
  context, the exact helper pointer parameter, and three initialization/mutation/output stores.
- Both lanes in `tests/cuda/nvvm-thread-local-global-context.slang` pass with runtime output
  `7, 8, 9, 10` and the expected PTX global store.
- The 650-byte PTX module assembles with CUDA 12.9.86 to a 2,792-byte cubin.
- The complete Release NVVM prefix passes 383/383, including established shared-memory and actual
  global atomic cases.

The completed self-review inventory contains three production changes:

- The `emitNVVMDirectly` pipeline gate survives because the PTX artifact target otherwise bypasses
  the two producers that own CUDA plain-global semantics. Removing it restores the plain
  module-level `Ptr<Int>` and the original `device scalar pointer` failure.
- The explicit thread-local branch in
  `asNVVMSupportedLocalScalarStructPointerType` survives because
  `IntroduceExplicitGlobalContextPass::findOrCreateContextPtrForFunc` creates exactly that complete
  pointer type. It requires read-write access, `ThreadLocal`, `DefaultLayout`, four operands, and an
  already selected scalar struct; no other decorated pointer is admitted.
- The helper-argument compatibility branch survives because
  `IntroduceExplicitGlobalContextPass::createContextForEntryPoint` creates a compact entry-local
  `var`, while `findOrCreateContextPtrForFunc` creates the complete helper parameter. It requires
  the compact caller pointer and exact pointee `isTypeEqual`; it does not define a custom structural
  equivalence.

The exact incoming plain global is canonical before the target-owned producer but is not a valid
emitter input. The producer, rather than emission, moves its initializer and replaces every use.
No new helper rebuilds syntax, walks arbitrary graphs, silently defaults, or introduces a
target-emitter special case for the global itself. The focused fake assertions are test-only
observations of the existing generic builder contract.

## Failure and Recovery

If context lowering changes existing groupshared/global-parameter behavior, revert the pipeline
gate and inspect the policy target before changing emitter code. If the generated context exceeds
the established aggregate ABI, keep the precise post-pass boundary rather than adding a
fixture-specific struct exception. Keep probes under ignored `build/slice115-*`; do not reset
unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep CUDA source, IR, PTX, cubin, and logs under ignored `build/slice115-*`. Distill settled pass
ownership and runtime evidence into the NVVM design and capability ledger, then commit this plan
with implementation as explicitly requested.
