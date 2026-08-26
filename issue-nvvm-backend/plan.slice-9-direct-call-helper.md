# Lower a direct signed-i32 helper call through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 9 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts retained, statically named helper functions with
the same canonical signed-`i32` scalar subset already demonstrated by Slices 7 and 8. The helper may
take signed-`i32` parameters, return signed `i32`, and be called directly from the selected raw CUDA
kernel. The concrete acceptance source is:

```slang
int increment(int value)
{
    return value + 1;
}

int incrementTwice(int value)
{
    return increment(increment(value));
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = increment(value) + incrementTwice(value);
}
```

The linked module must contain exactly the selected kernel and helpers reachable from it. Emission
declares all accepted functions before lowering any body, then lowers the helper's valued return and
the kernel's direct call using canonical `IRFunc`, `IRCall`, and `IRReturn` operands. It does not
turn every module function into a kernel or entry point, and it does not rediscover helpers from
names or reconstructed syntax.

Real acceptance compares direct NVVM and NVRTC PTX for the same source, assembles both with
`ptxas`, and launches both through the CUDA driver for positive and negative inputs. The expected
result is `2 * value + 3`, including `5 -> 13` and `-2 -> -1`. Fake acceptance pins declaration
order, signatures, function ownership, all four call callee/argument/result relationships, both
valued returns, the final store, and the fact that only `computeMain` receives the NVVM kernel
annotation.

Indirect calls, function values, recursion, declarations without bodies, void helpers, pointer
helper parameters/results, multiply and other new arithmetic, other scalar types, and richer
address spaces/aggregates remain deterministic unsupported boundaries. Those exclusions keep this
slice focused on function ownership and the smallest non-void helper ABI; Slice 10 remains the
address-space/aggregate/shared-memory boundary recorded in the durable roadmap.

## Progress

- [x] (2026-08-26) Completed and committed Slice 8 as `slice 8`.
- [x] (2026-08-26) Re-read `.agent/PLANS.md`, the Slice 8 handoff, durable design and capability
  ledger, current emitter/provider API, call graph utilities, and existing direct-call rejection.
- [x] (2026-08-26) Pinned the exact post-link IR for a two-helper transitive DAG, confirmed that
  ordinary optimization retains all four calls without `noinline`, and observed the unused
  multiplication helper pruned before preflight.
- [x] (2026-08-26) Appended and implemented one coherent V2 direct-call/non-void-return capability
  with no separate function cursor.
- [x] (2026-08-26) Extended whole-module preflight and multi-function emission without weakening
  earlier subsets.
- [x] (2026-08-26) Added negotiation, provider validation/no-mutation, fake graph, real
  differential, `ptxas`, runtime, and retained-boundary tests.
- [x] (2026-08-26) Formatted with pinned clang-format 17, rebuilt provider Release and host Debug,
  passed the full 60/60 NVVM prefix and established preservation regressions, inspected the DLL,
  audited the input shapes, and updated the durable design and capability ledger.
- [x] (2026-08-26) Committed the 13 tracked Slice 9 files as `slice 9` and
  began the bounded Slice 10 pointer-offset plan.

## Surprises and Discoveries

- Observation: a separate provider operation for a current function is unnecessary.
  Evidence: `declareFunction` and `createBlock` already establish ownership, and every mutating
  instruction operation uses the function owning the block selected by `setInsertBlock`.
  Consequence: adding a second ambient function cursor would create two sources of truth and a
  mismatch hazard. The appended ABI should express only the missing semantic operations.

- Observation: the existing repository call-graph helper is entry-point-oriented rather than an
  emitter-specific ordering API.
  Evidence: `buildEntryPointReferenceGraph` maps global values to referencing entry points and
  follows `IRCall::getCallee()`, but it is designed for all entry points and other global operands.
  Consequence: reuse its canonical callee relationship, but do not force a broad mapping into the
  direct emitter if a small explicit traversal from the one selected entry point is clearer and
  enforces this slice's restrictions. Record the final choice after the exact IR audit.

- Observation: the final optimized IR retains a transitive helper DAG without a source-level
  `noinline` attribute.
  Evidence: `incrementTwice` contains two exact calls to `increment`, and `computeMain` contains one
  call to each helper. An unrelated `unusedMultiply` definition is absent from the final linked IR.
  Consequence: use this graph as acceptance. It proves declaration-before-body and transitive
  reachability without accepting and then silently dropping `IRNoInlineDecoration`.

- Observation: every retained ordinary helper has a canonical export/linkage decoration and name
  hint in the final IR.
  Evidence: both helpers are module-global `IRFunc(Int, Int)` definitions with a unique mangled
  linkage name; calls name those exact IR functions as operand zero. The kernel remains
  `IRFunc(Void, Ptr(device Int), Int)` with the entry-point name `computeMain`.
  Consequence: helper LLVM symbols use the canonical mangled linkage name, while the selected
  kernel retains its entry-point ABI name. No syntax/name lookup is required.

- Observation: the barrier intrinsic now reaches a more precise unsupported boundary than it did
  in Slice 8.
  Evidence: final linked IR represents `GroupMemoryBarrierWithGroupSync()` as a direct call to a
  retained void helper. Slice 9 recognizes the direct callee before rejecting its result type, so
  E52017 names `'helper function result type'` rather than the older generic `'call'`.
  Consequence: update both the file expectation and routing/hash assertion to the semantic boundary;
  do not add a barrier-specific exception.

## Decision Log

- Decision: Slice 9 accepts direct calls to one or more transitively reachable, defined,
  non-recursive helpers whose parameters and result are signed i32; the selected kernel remains
  void and retains its existing raw pointer/i32 launch ABI.
  Rationale: this is the smallest useful function boundary and independently proves call
  ownership, declaration-before-body ordering, argument ABI, and valued returns without expanding
  the scalar or pointer ABI.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a later slice deliberately accepts external declarations or indirect calls.

- Decision: append an all-or-nothing V2 capability containing a signed-i32 direct-call operation
  and a signed-i32 valued-return operation; do not change frozen V1 or earlier V2 prefixes.
  Rationale: both operations are required to demonstrate the non-void helper ABI, while old
  providers must continue to run exact Slice 8 programs and reject only call-shaped programs at
  capability negotiation.
  Date/Author: 2026-08-26, Codex.
  Revisit when: an incompatible ABI rather than append-only operations is required.

- Decision: do not add ambient current-function state.
  Rationale: the current insertion block already has exactly one parent function. Provider
  validation can derive caller ownership from it and compare the callee/arguments/result locally.
  Date/Author: 2026-08-26, Codex.
  Revisit when: an operation with no insertion block and function-local semantics is introduced.

- Decision: retain helpers by reachability from the one selected linked entry point and never mark
  a helper as a kernel.
  Rationale: the `IRCall` callee is the semantic source of truth. Treating every module function as
  an entry point would change ABI and retain unrelated code; looking up a helper by name would
  duplicate linking and overload resolution.
  Date/Author: 2026-08-26, Codex.
  Revisit when: exported device functions become a deliberate product requirement.

- Decision: multiplication stays unsupported in Slice 9.
  Rationale: the durable roadmap names this slice as direct calls and non-void helper ABI. The
  accepted helper uses already-supported addition, so adding another arithmetic opcode would not
  strengthen the call-ABI evidence and would blur the next operation/type boundary.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a later bounded scalar-operations slice is planned.

- Decision: the provider operation permits a same-module function declaration, while host
  preflight requires every retained helper to be a definition.
  Rationale: the provider ABI should remain suitable for future external/libdevice declarations;
  this slice's language subset deliberately owns and emits complete helper bodies. Keeping the
  policy in preflight avoids hard-coding a temporary frontend restriction into LLVM construction.
  Date/Author: 2026-08-26, Codex.
  Revisit when: external declarations become an accepted direct-NVVM language feature.

## Outcomes and Retrospective

Slice 9 now emits the complete direct-call closure rooted at the sole selected CUDA kernel. The
accepted linked module contains `computeMain`, `increment`, and `incrementTwice`; all functions are
declared before any body, the four `IRCall` instructions use their exact `IRFunc` operands, the two
helper `IRReturn` instructions carry signed-i32 values, and only `computeMain` is annotated as a
kernel. An unreachable helper containing multiplication is pruned by linking and never enters
preflight or provider emission.

The append-only V2 prefix adds exactly `emitIntegerCall` and `emitIntegerReturn`. Exact earlier
prefix sizes remain accepted, every partial scalar-function prefix is rejected, failed calls
sanitize their output handle, and the stable builder identity now includes
`scalar-functions=0|1`. Provider operations validate module/function/type/arity/dominance and the
unterminated insertion point before changing LLVM state. The insertion block remains the sole
source of caller ownership; no second current-function cursor was introduced.

The formatted Release provider and Debug host builds passed. The authoritative NVVM prefix passed
60/60, including verified provider IR, NVVM/NVRTC differential PTX, assembly of both routes with
CUDA 12.9 `ptxas`, and CUDA-driver results `5 -> 13` and `-2 -> -1` for both routes. The established
preservation set passed CUDA emission option parsing 1/1, routing/hash 2/2, unsupported-IR 1/1,
sampler default/explicit NVRTC 3/3, true pass-through 2/2, and CUDA runtime dispatch 1/1. The first
routing/hash run exposed its stale Slice 8 expectation for generic `'call'`; changing it to the
canonical void-helper-result boundary made the corrected run pass 2/2 without a production-code
change. The final provider-independent unsupported table additionally pins pointer helper
parameters and pointer helper results at their precise pre-discovery E52017 boundaries.

The old Slice 8 host exercised the new provider through its shorter table and passed its complete
55/55 NVVM prefix. `dumpbin` shows only `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2` exports. The DLL has only `KERNEL32.dll` as an ordinary dependency and
`SHELL32.dll`/`ole32.dll` as delay-load dependencies, with no process-visible LLVM DLL dependency.
Pinned clang-format reports no modified files and `git diff --check` passes.

The principled-change audit found no downstream representation repair. `_getNVVMFunctionName`
reads the entry-point name or canonical mangled helper name already stored on linked IR;
`_validateNVVMHelperTarget`, `_visitNVVMFunction`, and `_collectNVVMFunctions` follow exact
`IRCall` callee operands and reject accidental alternatives instead of matching them;
`_validateNVVMFunctionUses` proves functions occur only in callee position;
`_validateNVVMFunctionNames` checks uniqueness without inventing names; and
`_validateNVVMFunction` applies existing per-function CFG, dominance, SSA, and type invariants to
the accepted closure. These helpers survive because linked IR is the semantic source of truth and
the emitter owns whole-module acceptance. The provider call/return operations are ABI-boundary
construction primitives, not fallbacks, and their validation precedes mutation. Test-only fake
handle/index helpers and scalar-function module builders merely record the same ownership/value
graph for assertions; the store record includes the owning kernel and pointer parameter zero. The
legacy single-function parameter view delegates to that canonical map, and the refactor's dead
function-zero lookup was removed. These fixtures do not affect production representation. No
syntax is rebuilt, no custom semantic equivalence is introduced, and no generic operand walk
rediscovering context is used.

## Context and Current Pipeline

`source/slang/slang-emit.cpp` links and optimizes the selected program, preserves direct-NVVM raw
CUDA signatures, removes transient CUDA export pins, and restores keep-alive only on exact selected
entry points. It calls `validateNVVMSupportedIR` before optional builder discovery, negotiates the
shape-dependent builder capability, and then calls `emitNVVMIRFromLinkedIR`.

Before this slice, `source/slang/slang-emit-nvvm.cpp` assumed `linkedIR.entryPoints[0]` was the only
emitted function. It validated one function with a dominator tree, then declared its LLVM signature,
creates all blocks and phis, emits ordinary instructions/terminators, and attaches phi incoming
edges after the complete CFG exists. Slice 9 must preserve that four-phase per-module discipline
while generalizing ownership to the reachable helper set. Function declarations must all exist
before call instructions; each function needs its own block set, dominator tree, body order, and
IR-to-provider maps, while module-owned scalar types can remain shared.

`source/compiler-core/slang-nvvm-ir-builder-api.h` freezes V1 and publishes append-only coherent V2
prefixes. The last Slice 8 field is `addIntegerPhiIncoming`. The host wrapper validates complete
prefixes and includes capability bits in the builder identity used by shader-cache keys. The LLVM
14 provider owns every LLVM object in a per-module state, and `setInsertBlock` selects the unique
current insertion block. Slice 9 appends operations without exposing LLVM classes across the ABI.

The existing unit-test fake records exact provider calls. Real lanes use the optional Release
LLVM 14 provider, CUDA libNVVM/NVRTC, CUDA 12.9 `ptxas`, and the CUDA driver when a suitable device
is present. Existing Slice 8 tests currently reject the motivating direct call as E52017 before
builder discovery; that case becomes positive while barrier calls and multiplication remain
pre-discovery E52017 coverage.

## Plan of Work

First, dump or otherwise inspect the exact final linked IR for the motivating source. Record the
callee identity, function decorations/linkage, helper and kernel signatures, `IRCall` operand
shape, valued `IRReturn`, physical/global ordering, and whether normal linking already removes an
unreachable multiplication helper. Use those facts to define one deterministic traversal rooted
at the selected entry point. Reject non-`IRFunc` callees, declarations, cycles, unsupported
signatures, and out-of-set references before builder discovery.

Second, append the coherent direct-call prefix to `SlangNVVMBuilderAPI_V2`. The final audit is
expected to publish two operations: emit a same-module direct call in the current unterminated
insertion block, and terminate that block with a same-module signed-integer value. The provider
must validate module ownership, caller/callee function ownership, exact signature/argument count
and types, dominance of arguments/return value, the current integer result type, and insertion
state before mutating LLVM IR. Slang preflight owns the void-kernel/non-void-helper role split.
Host wrappers sanitize output handles on failure and return
`SLANG_E_NOT_AVAILABLE` for an older prefix. Negotiation rejects every partial/incomplete new
prefix, accepts exact older prefixes, accepts larger tables, and adds a stable capability bit to
the version string.

Third, refactor direct-NVVM preflight around an explicit per-function context. Construct the
reachable helper set by following only direct `IRCall` callees starting at the selected entry point
and defensively detect cycles in unexpected final linked IR. Ordinary source recursion is rejected
earlier by `checkForRecursiveFunctions` as E55201. Validate every signature and every body before
discovery.
Kernel parameters retain pointer/i32 launch rules and void result. Helpers accept only signed-i32
parameters/results. Calls require exact argument count/type and dominate their uses; valued returns
occur only in helpers and match their declared signed-i32 result. Preserve all existing block,
SSA, CFG, pointer-access, and dominance validation independently for each function.

Fourth, emit all function declarations and parameters before any block/body, then create blocks
and phi placeholders for each function, emit each body in its semantic body order, and finally
attach each function's phi incoming edges. Lower `IRCall` through the already-declared callee handle
and map the result to the canonical call instruction. Lower helper `IRReturn` through the valued
return operation. Mark only the selected entry point as a kernel and serialize once after every
function body is complete.

Fifth, extend tests. ABI tests cover exact old prefix compatibility, every partial/incomplete new
prefix, larger tables, output sanitization, and identity. Provider tests build a helper/caller
module and reject missing insertion points, wrong call argument count/type, cross-module values or
callees, non-dominating arguments/returns, mismatched return type, calls after termination, and
duplicate/post-termination returns without mutation. Fake direct-route coverage checks exact graph
and helper retention/pruning. Real coverage compares NVVM/NVRTC PTX, `ptxas`, and runtime results.
Retain deterministic E52017 for barrier, multiplication, and unsupported pointer helper
signatures. Function-value/cycle checks remain defensive final-IR validation; ordinary recursion
and unresolved externals are rejected upstream as E55201 and E45001. Verify old providers reach
E52016 after discovery but before module creation.

Finally, run pinned clang-format 17 on changed C/C++ files, rebuild provider Release and host Debug
outside the sandbox, run the full NVVM unit prefix and established routing/NVRTC/runtime
regressions, inspect exports/dependencies, run `git diff --check`, and perform the required helper
and input-shape self-review. Update `docs/design/nvvm-backend.md` and the capability ledger with
only demonstrated claims. Keep this ExecPlan untracked, commit tracked files exactly as `slice 9`,
and immediately start Slice 10 unless a genuine blocker remains.

## Concrete Steps

Run from `C:\src\slang` using Windows-native tools. All CMake builds and tests run outside the
sandbox as required by `AGENTS.md`.

    git.exe status --short
    build\Debug\bin\slangc.exe <probe> -target ptx -entry computeMain -emit-cuda-via-nvvm -dump-ir

After implementation and formatting:

    cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm -- /m
    cmake.exe --build build --config Debug --target slang-test
    $env:SLANG_NVVM_BUILDER_PATH = 'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
    build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm

Run focused option/routing/hash, unsupported-IR file, NVRTC sampler/pass-through, and CUDA runtime
regressions using the exact test paths established by Slice 8. Then inspect the provider with
`dumpbin.exe /exports` and `/dependents`, verify pinned formatting in diff mode, and run:

    git.exe diff --check
    git.exe status --short
    git.exe add <tracked Slice 9 files>
    git.exe commit -m "slice 9"

## Validation and Acceptance

Slice 9 is complete only when all of the following are true:

- the motivating source reaches direct NVVM and its provider graph contains two helper declarations,
  one kernel declaration, two signed-i32 valued returns, four direct calls, and a store of their
  combined result;
- only the selected kernel is annotated as a kernel, and an unrelated multiplication helper is
  absent;
- an exact Slice 8 provider still runs Slice 8 shapes but call-shaped input reaches E52016 after
  discovery and before module creation;
- malformed provider prefixes and invalid call/return operations fail deterministically without
  exposing handles or leaving partially mutated IR;
- real direct NVVM and NVRTC agree on kernel ABI/global-memory semantics, both PTX outputs assemble,
  and runtime results agree for positive, negative, and zero inputs;
- barrier, multiplication, and unsupported pointer helper signatures remain deterministic
  pre-discovery E52017 boundaries; ordinary recursion and unresolved externals remain owned by
  their upstream E55201/E45001 checks, with defensive function-value/cycle validation retained for
  unexpected final IR;
- the full NVVM prefix and established routing/NVRTC/runtime regression set pass after formatting;
- verifier diagnostics are empty on accepted modules, provider exports remain only V1/V2 getters,
  no process-visible LLVM DLL dependency appears, formatting is clean, and `git diff --check`
  passes.

## Idempotence and Recovery

The probe, builds, tests, formatter diff check, binary inspection, and status commands are safe to
repeat. Provider and host builds are incremental. The optional provider path is set only in the
current PowerShell process. Do not delete or reset the user's worktree. If a test exposes a wrong
IR assumption, update this plan and fix the producer/representation boundary rather than adding a
consumer fallback. This plan and prior slice plans remain untracked and must never be staged.

## Artifacts and Notes

The exact accepted linked IR has helper signatures `Func(Int, Int)`, kernel signature
`Func(Void, Ptr(device Int), Int)`, direct `IRFunc` operand zero on each `IRCall`, and valued
`IRReturn` terminators in both helpers. Four calls remain after ordinary linking without a
`noinline` attribute; the unused multiply helper is absent. The verified provider module contains
three definitions, four `call i32` instructions, two `ret i32` instructions, and one kernel
annotation. Direct NVVM and NVRTC PTX both assembled and produced `13` for input `5` and `-1` for
input `-2` on the available CUDA device. Final counts and the helper/input-shape audit are recorded
in Outcomes above. Temporary Slice 9 probes were removed before commit.

## Interfaces and Dependencies

Expected append-only private C ABI, subject to the final builder audit:

    emitIntegerCall(module, callee, arguments, argumentCount, outValue)
    emitIntegerReturn(module, value)

Both operations use opaque same-module handles. The call inserts into the current unterminated
block, validates the direct callee's exact integer function type and argument dominance, and returns
its integer SSA value. The return validates the current function's integer result and terminates
the current block. `setInsertBlock` remains the sole ambient insertion/function ownership state.
