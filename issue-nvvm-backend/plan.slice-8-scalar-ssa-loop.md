# Lower signed i32 constants, phis, and a finite loop through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 8 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts the canonical signed-`i32` SSA program structure
that Slice 7 deliberately preserved but rejected: executable integer constants, non-entry basic-
block parameters paired with branch arguments, and a finite loop with loop-carried values.

These three ordinary Slang sources define the accepted boundary:

```slang
[CUDAKernel]
void addOne(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = value + 1;
}
```

```slang
[CUDAKernel]
void selectValue(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    int selected;
    if (x < y)
        selected = x;
    else
        selected = y;
    *destination = selected;
}
```

```slang
[CUDAKernel]
void sumToLimit(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int limit)
{
    int sum = 0;
    for (int i = 0; i < limit; ++i)
        sum += i;
    *destination = sum;
}
```

Selecting each function as `computeMain` for PTX with `-emit-cuda-via-nvvm` preserves the Slice 7
raw ABI. The first program materializes the exact signed i32 constant `1`. The second preserves the
merge value as a Slang IR block parameter and emits one LLVM `phi i32` with the two arm values and
predecessors. The third preserves two loop-header parameters, materializes `0` and `1`, emits two
LLVM phis, and lowers the canonical `IRLoop` entry edge plus its existing backedge without
reconstructing stack variables or C-like scopes.

Real acceptance compares the same sources through direct NVVM and NVRTC, requires matching raw
parameter widths and global-memory semantics, and assembles both routes with `ptxas`. CUDA-driver
runtime evidence checks `41 + 1 == 42`, both merge arms and equality, and loop limits
`0 -> 0`, `5 -> 10`, and `7 -> 21` through both routes.

Direct calls, non-void helper functions/returns, multiplication, other integer operations/types,
pointer phis, and richer CFG terminators remain unsupported. Calls and multiplication form the
stable next boundary for Slice 9; this slice does not expand the selected function into a call
graph or invent a helper ABI.

## Progress

- [x] (2026-08-26 19:48Z) Completed Slice 7, committed it as `slice 7`, and recorded
  deterministic E52017 boundaries on `integer_constant`, `branch argument`, `loop`, and `call`.
- [x] (2026-08-26 19:48Z) Re-read `.agent/PLANS.md`, the durable design/ledger, committed Slice 7
  implementation, and the append-only builder/provider and direct-emitter boundaries.
- [x] (2026-08-26 19:48Z) Completed initial read-only research of the canonical constant, merge-phi,
  and loop shapes and chose a coherent constants-plus-SSA capability with no pipeline changes.
- [x] (2026-08-26 20:06Z) Reconfirmed the exact linked IR for add-one, merge, and finite-loop
  sources plus CFG predecessor, branch-argument, and `IRLoop` APIs. The producer already supplies
  canonical constants, block parameters, actual target edges, and structured loop metadata.
- [x] (2026-08-26 20:49Z) Appended and implemented the coherent V2 scalar-SSA capability with exact
  constants, explicit-block phis, delayed incoming edges, strict negotiation, and cache identity.
- [x] (2026-08-26 20:49Z) Extended complete Slang IR preflight and four-phase emission for signed
  i32 constants, block parameters/branch arguments, canonical loops, and CFG-safe body ordering.
- [x] (2026-08-26 21:17Z) Added exact fake/provider/public-route negatives and positive graphs,
  including the observed six-block/two-phi finite sum loop.
- [x] (2026-08-26 21:27Z) Passed real NVRTC/NVVM differential PTX, CUDA 12.9 `ptxas`, and
  CUDA-driver runtime evidence on the RTX 5090.
- [x] (2026-08-26 21:46Z) Rebuilt outside the sandbox, applied/verified pinned clang-format 17,
  passed 55/55 NVVM tests plus established regressions, inspected the provider binary, completed
  the helper/input-shape audit, and updated the durable design and capability ledger.

## Surprises and Discoveries

- Observation: Slice 7 already preserves canonical SSA for direct NVVM.
  Evidence: it bypasses `eliminatePhis`, `simplifyNonSSAIR`, and C-like variable-scope correction;
  the merge source reaches preflight as a non-entry `IRParam<Int>` with argument-bearing incoming
  branches, and the loop source retains `IRLoop` plus header parameters.
  Consequence: Slice 8 needs no pipeline representation pass. It extends the legality and builder
  boundaries rather than re-enabling phi elimination or reconstructing local variables.

- Observation: the loop's structured operands are not all executable LLVM CFG edges.
  Evidence: initial source/API research shows `IRLoop` inherits the actual target/argument edge,
  while its break and continue blocks record structured control-flow metadata used by Slang IR
  consumers.
  Consequence: validate all structured block operands for ownership, but emit only the canonical
  actual target edge. Reconfirm this using the concrete linked IR and CFG predecessor APIs before
  implementation.

- Observation: phi incoming values may be defined by the loop backedge after the header phi must
  already exist.
  Evidence: the loop header's `i` and `sum` block parameters feed the compare/body, while updated
  `i + 1` and `sum + i` values are produced in a later block and branch back to the header.
  Consequence: declaration and body emission cannot remain a single pass. Create blocks and phi
  placeholders before bodies, emit bodies/terminators next, and attach incoming pairs only after
  every value and predecessor terminator exists.

- Observation: the provider can place a phi more safely when the destination block is explicit.
  Evidence: a current-insertion-block API would make phi creation depend on host ordering and
  provider builder state, while LLVM accepts insertion directly before `getFirstNonPHI()` in the
  named block.
  Consequence: `emitIntegerPhi` takes the destination block handle. This makes placement and
  ownership validation local to the operation and avoids an accidental stateful ordering contract.

- Observation: the exact linked loop preserves one actual entry edge and one actual backedge while
  retaining break/continue as structured metadata.
  Evidence: the entry terminator is `loop(header, break, continue, 0, 0)`; header parameters are
  `(i, sum)`; the continue block ends in `unconditionalBranch(header, i + 1, sum + i)`; CFG
  successor enumeration treats only the loop target as the entry edge.
  Consequence: emit `IRLoop` through the existing unconditional branch operation and derive phi
  incoming pairs through `IRUnconditionalBranch::getArgs()`, which already accounts for the loop's
  three leading structured operands.

- Observation: the exact source loop retains a separate exit-to-break block.
  Evidence: the public fake graph observed six blocks and four unconditional edges: entry to
  header, body to continue, continue back to header, and exit to break. The final store is in the
  break block, not the conditional's false target.
  Consequence: acceptance asserts the semantic six-block graph and both loop-carried phis instead
  of relying on the provisional five-block probe assumption.

- Observation: reachable body emission needs semantic ordering, while unreachable blocks remain
  legal physical IR.
  Evidence: raw sibling order can place an independent branch arm in either callback order, but
  reverse postorder guarantees a reachable dominator precedes its consumer. Rejecting unreachable
  blocks would have narrowed Slice 7 unnecessarily.
  Consequence: `_getNVVMBodyOrder` uses reachable reverse postorder and appends physical unreachable
  blocks in their prior order. Validation and emission consume the same list.

## Decision Log

- Decision: Slice 8 accepts only signed i32 executable constants, signed i32 non-entry block
  parameters, exact argument-bearing unconditional/loop edges, and canonical `IRLoop`.
  Rationale: these operations complete the smallest useful SSA structure around Slice 7's existing
  scalar arithmetic/control flow and prove a finite loop without expanding the type or call ABI.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a later slice deliberately adds other scalar types, switches, or calls.

- Decision: append one all-or-nothing V2 `ScalarSSA` block rather than V3.
  Rationale: constants and phis add operations but do not break handle ownership, typed pointers,
  serialization, or existing function semantics. Every prior V2 minimum remains immutable.
  Date/Author: 2026-08-26, Codex.
  Revisit when: an incompatible pointer/ownership/table reset is required.

- Decision: publish `getIntegerConstant`, `emitIntegerPhi`, and `addIntegerPhiIncoming` as the
  complete new prefix.
  Rationale: constant construction, phi placeholder creation, and delayed incoming attachment are
  three distinct LLVM lifecycle points. Combining them would require all backedge values before
  the phi exists or would expose temporary arrays/ownership across the ABI.
  Date/Author: 2026-08-26, Codex.
  Revisit when: exact provider/API audit finds a smaller equally coherent interface.

- Decision: `emitIntegerPhi` names its destination block instead of using the current insertion
  block.
  Rationale: the destination is semantic input, not ambient builder state. An explicit handle lets
  the provider validate module/function ownership and insert before the first non-phi instruction
  without depending on which body the host most recently selected.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the opaque builder ABI adopts an explicit instruction cursor for all operations.

- Decision: keep capability negotiation shape-dependent.
  Rationale: Slice 6 empty kernels, Slice 4 scalar memory, and Slice 7 phi-free control flow remain
  usable with their exact old prefixes. A program containing an executable constant or phi requires
  the complete ScalarSSA prefix. A bare zero-argument `IRLoop` needs only the already-published
  branch operation; ordinary loop-carried state naturally raises the requirement through its
  constants, block parameters, and branch arguments.
  Date/Author: 2026-08-26, Codex.
  Revisit when: packaging intentionally raises the global minimum.

- Decision: lower Slang IR block parameters directly to LLVM phis and derive incoming pairs from
  canonical predecessor edges and branch arguments.
  Rationale: these are the existing semantic source of truth. Rebuilding stack variables,
  searching operand graphs, or inferring values positionally outside each target parameter would
  duplicate or corrupt the canonical representation.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a valid canonical edge form not represented by the existing CFG APIs is observed.

- Decision: direct calls and non-void helpers remain Slice 9.
  Rationale: calls require retaining and validating helper definitions, function signatures,
  valued returns, and call-site ABI. That is independently demonstrable and should not be hidden
  inside the phi/loop increment.
  Date/Author: 2026-08-26, Codex.
  Revisit when: implementation evidence proves a helper is unavoidable for the three accepted
  sources.

## Outcomes and Retrospective

Slice 8 completed the planned signed-i32 SSA boundary without changing the producer pipeline. The
direct route now consumes executable `IRIntLit`, lowers non-entry `IRParam` values to LLVM phis,
pairs canonical branch/loop arguments with those phis after the complete CFG exists, and emits the
six-block `sumToLimit` graph with two loop-carried values. Direct multiplication and calls remain
deterministic E52017 boundaries before builder discovery.

Both required builds passed outside the sandbox:

```text
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build --config Release \
    --target slang-llvm-nvvm -- /m
cmake.exe --build --preset debug --target slang-test -- /m
```

The post-format `slang-unit-test-tool/nvvm` prefix passed 55/55. Focused negotiation, invalid
provider operations, exact fake graphs, old-provider capability failure, real LLVM loop, same-source
PTX, `ptxas`, and runtime lanes all passed. The final preservation set passed option parsing 1/1,
routing/hash 2/2, unsupported barrier-call file 1/1, sampler NVRTC lanes 3/3, pass-through 2/2, and CUDA
runtime dispatch 1/1.

CUDA 12.9.86 `ptxas` accepted all six sources from NVRTC and direct NVVM. On an RTX 5090 with
compute capability 12.0 and driver 610.62, add-one, merge less/greater/equal, and `sumToLimit(0/5/7)`
matched across routes; the sums were `0`, `10`, and `21`. Pinned clang-format 17 reports no remaining
changes and `git diff --check` is clean. `dumpbin` reports only
`slang_getNVVMBuilderAPI_V1`/`slang_getNVVMBuilderAPI_V2`; dependencies remain `KERNEL32.dll` plus
delay-loaded system DLLs, with no LLVM DLL.

The main correction during integration was representational discipline: physical sibling order was
not accepted as semantic dominance order. Reachable ordinary bodies now use reverse postorder in
both validation and emission, while unreachable blocks retain the old physical ordering. The other
correction was test evidence: the exact final loop has a separate exit and break, so the fake graph
was raised from a provisional five blocks to the observed six rather than weakening the source.

## Context and Current Pipeline

Slice 8 preserves raw `[CUDAKernel]` parameters and canonical SSA through `linkAndOptimizeIR`, then
`validateNVVMSupportedIR` proves the signed-i32/device-pointer/SSA subset before optional builder
discovery. `emitNVVMIRFromLinkedIR` declares the function and all blocks, creates phis, emits
ordinary bodies in the shared CFG order, attaches incoming edges, marks the function as a kernel,
serializes verified LLVM 14 bitcode, and hands the existing LLVMIR kernel artifact to
`NVVMDownstreamCompiler` for architecture/options/libNVVM/PTX policy.

Consider `selectValue`. Its principled target-independent shape is:

```text
entry(destination, x, y):
    condition = less(x, y)
    ifElse(condition, trueBlock, falseBlock, mergeBlock)
trueBlock:
    unconditionalBranch(mergeBlock, x)
falseBlock:
    unconditionalBranch(mergeBlock, y)
mergeBlock(selected):
    store(destination, selected)
    return_val(void_constant)
```

The `selected` block parameter is the semantic merge value. Its position pairs with argument zero
on every predecessor edge. Slice 7 rejected `branch argument` before builder discovery; Slice 8
preserves this source of truth and emits:

```llvm
merge:
  %selected = phi i32 [ %x, %true ], [ %y, %false ]
  store i32 %selected, i32 addrspace(1)* %destination, align 4
  ret void
```

For `sumToLimit`, the desired Slang IR has two header block parameters initialized by constants
zero, a signed less-than condition, an exit edge, and a backedge carrying `sum + i` and `i + 1`.
The provider creates the header phis before the body uses them but cannot attach the backedge
incoming values until the body and its terminator have been emitted.

The committed V2 provider already owns i32 types, raw AS1 pointer parameters, load/store,
add/sub/signed-less-than, structural blocks, conditional/unconditional branches, void returns,
kernel annotation, verification, and serialization. V2 is explicitly append-only and publishes
coherent diagnostic, scalar-memory, and scalar-control-flow minimum sizes.

## Scope and Non-Goals

In scope:

- executable signed i32 literals representable exactly in 32 bits;
- non-entry signed i32 block parameters;
- exact signed i32 branch arguments paired with target block parameters by index;
- merge phis and loop-carried signed i32 phis;
- canonical `IRLoop` actual target edge and structured block ownership validation;
- placeholder phi creation followed by delayed incoming attachment;
- one append-only V2 ScalarSSA prefix and strict partial-prefix rejection;
- complete legality before builder discovery/module creation;
- no mutation on invalid constant/phi provider calls;
- exact fake public-route graphs and old-provider capability rejection;
- real LLVM verification, same-source NVRTC/NVVM PTX semantics, `ptxas`, and optional runtime;
- preserved default/explicit NVRTC, pass-through, loader/hash, Slice 6/7, and CUDA runtime results;
  and
- durable design, ledger, and working-log updates.

Non-goals:

- direct calls, helper declarations/definitions, recursion, non-void function signatures, or
  valued returns;
- multiplication, division, remainder, shifts, bitwise operations, casts, or unsigned comparison;
- bool, i8/i16/i64, uint, half/float/double constants, parameters, phis, or arithmetic;
- pointer, aggregate, vector, matrix, resource, witness, existential, or interface phis;
- switches, generic multi-way terminators, exception edges, or irreducible CFG repair;
- allocas, local memory reconstruction, phi elimination, or C-like scope correction;
- additional address spaces, pointer arithmetic/GEP, shared memory, globals, or atomics;
- builtins, thread/block IDs, barriers, launch bounds, or libdevice;
- multiple selected entry points, OptiX, RDC/LTO, debug metadata, or public NVVM target enums; and
- byte-for-byte LLVM/PTX equality or performance thresholds.

## Architecture and Invariants

`TargetProgram::shouldEmitNVVMDirectly()` remains the only representation query. Slice 8 adds no
pipeline conditions: raw CUDA ABI and SSA retention are already the correct producer shape.
`LinkedIR.entryPoints` remains the exact selected-function identity. The validator owns the full
accepted-subset proof and returns the minimum builder capability. The emitter maps only validated
canonical IR. `NVVMIRBuilder` owns provider negotiation/lifetime. The LLVM 14 provider owns all
opaque handles and enforces its own context/module/function/CFG contract before mutation.

The V2 append-only layout becomes:

1. frozen diagnostic prefix;
2. frozen scalar-memory prefix;
3. frozen scalar-control-flow prefix; and
4. new coherent ScalarSSA prefix through `addIntegerPhiIncoming`.

A reported size inside any capability block is malformed. At or above ScalarSSA minimum, all three
new functions are mandatory. Future larger tables are clamped to the host structure. Builder
identity gains `scalar-ssa=0|1` so shader caches cannot confuse providers with different lowering
capabilities.

Planned provider signatures use only fixed-width payloads and existing opaque handles:

```c
getIntegerConstant(module, integerType, int64_t value, outValue)
emitIntegerPhi(module, targetBlock, integerType, outValue)
addIntegerPhiIncoming(module, phi, value, predecessorBlock)
```

`getIntegerConstant` accepts only a same-context scalar integer type for which `value` is exactly
representable, clears output first, and returns an LLVM `ConstantInt`. It does not require an
insertion block and mutates no module instruction list.

`emitIntegerPhi` requires a same-module destination block and inserts before its first non-phi
instruction (or at its end when it contains only phis). The type must be a same-context scalar
integer. Failure clears output and inserts nothing.

`addIntegerPhiIncoming` validates completely before mutation: the phi belongs to the module and
function, every block in that function has a terminator, the predecessor belongs to that function,
its terminator has exactly one actual edge to the phi block, the value has the exact phi type and is
usable at the predecessor terminator, and that predecessor has not already supplied an incoming
value. Constants and function arguments are allowed; instructions must dominate the predecessor
terminator. Failure leaves the phi unchanged. Rejecting parallel edges is intentional because the
narrow Slang subset supplies phi arguments only through a single unconditional/loop edge.

Slang validation accepts executable `IRIntLit : Int`, non-entry `IRParam : Int`, matching branch
arguments, and `IRLoop`. It validates a branch argument at its predecessor terminator, not as if it
were used in the target block. Every predecessor of a parameterized block must supply exactly one
argument per parameter, and every argument-bearing edge must match the target parameter count and
type. Structured loop break/continue blocks must belong to the function even though only the loop
target is emitted as the actual branch.

Emission is phased:

1. create the exact function, raw parameters, and every LLVM block;
2. create a phi placeholder in the explicit destination block for every non-entry block parameter
   before ordinary bodies;
3. emit ordinary instructions and actual branch/loop terminators, recording all canonical values;
4. walk canonical predecessor edges and attach each branch argument to the corresponding target
   phi using the already-emitted predecessor block and value handles; then annotate/verify/serialize.

Signed-i32 literals are materialized and identity-cached on first use in phase 3 or 4. Constants do
not depend on an insertion point, so eagerly scanning operands would add ordering machinery without
strengthening the SSA lifecycle invariant.

### Input-shape and special-case audit

Final production-helper inventory:

- `_asExecutableI32Constant` survives. It recognizes only exact `IRIntLit : Int` values in signed
  i32 range; it does not promote layout constants or rebuild syntax. Add-one and loop fake/real
  tests fail without it.
- `_getBlockParamCount` and `_validateBranchArguments` survive. They state the canonical positional
  edge invariant through `IRUnconditionalBranch::getArgs()` and target `IRBlock::getParams()`.
  Merge and loop fake graphs prove exact counts, types, values, and predecessors.
- `_getNVVMBodyOrder` survives. It is the single ordering source for validation and emission:
  reachable reverse postorder preserves dominance, then unreachable physical blocks preserve the
  Slice 7 boundary. The old conditional test proves sibling callback order is not semantic.
- `_getNVVMI32Type` and `_getLoweredNVVMValue` survive. They lazily create the one module type and
  identity-cache exact literals; they do not search arbitrary operands or create a second Slang
  value representation. Fake handle identity and real LLVM verification own these helpers.
- `_supportsScalarSSA` survives as the one coherent ABI-prefix classifier. Partial/null/future and
  exact-old-prefix negotiation tests own it.
- `_hasCompleteCFG` and `_isValueUsableOnIncomingEdge` survive at the provider boundary. Phi inputs
  have edge semantics distinct from ordinary insertion-point uses. Focused tests remove each gate
  in effect by presenting incomplete, non-predecessor, duplicate, foreign, and non-dominating
  shapes, then prove the final module still contains exactly the two valid incoming pairs.
- The `IRLoop` switch cases survive only to validate structured break/continue ownership and emit
  the inherited actual target edge. No provider loop operation or alternate loop representation was
  added.
- The four emission phases survive because loop backedge values do not exist when header phis are
  created. Constants remain lazy because they have no CFG lifecycle.

No new fallback, custom IR/`Val` equivalence, syntax reconstruction, stack-variable repair,
arbitrary operand search, inferred predecessor, ignored branch argument, hardcoded generic index,
or silent default remains. The only rejected alternatives were phi elimination/local reconstruction,
an ambient-insertion-point phi ABI, early incomplete-CFG dominance, and physical-order-as-dominance;
each would weaken an existing semantic source of truth.

## Interfaces and Dependencies

In `source/compiler-core/slang-nvvm-ir-builder-api.h`, append the three fixed-signature function
types and fields after Slice 7's `emitConditionalBranch`, then publish
`SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE`. Keep V1 and every prior V2 minimum byte-for-byte
frozen. Extend the strict-C probe in
`source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c`.

In `source/compiler-core/slang-nvvm-ir-builder.h/.cpp`, add coherent-prefix validation,
`supportsScalarSSA()`, sanitized wrappers, and `scalar-ssa=` identity. Old providers retain their
existing wrappers and capabilities.

In `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`, implement LLVM 14 `ConstantInt`, `PHINode`
creation, and delayed `addIncoming` with complete-before-mutation context/module/function/type/CFG/
dominance/duplicate checks. Reuse the existing `DominatorTree` ownership logic at the predecessor
terminator. Export lists and provider packaging remain unchanged.

In `source/slang/slang-emit-nvvm.h/.cpp`, append `ScalarSSA` capability classification and extend
the validator/emitter phases. Reuse existing IR CFG helpers and typed casts. Do not modify the
shared pipeline unless concrete probing disproves the committed Slice 7 producer invariant.

In `source/slang/slang-emit.cpp`, require `supportsScalarSSA()` only when validation classifies that
capability; otherwise leave orchestration unchanged.

In `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`, extend fake handles/recording and add
negotiation, invalid-operation, exact graph, real provider, differential PTX, `ptxas`, runtime, old
provider, and unsupported-next-boundary coverage. Reuse `cuda-driver-test-util.h`; do not create a
second CUDA loader.

Update `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md` with the frozen Slice 8 boundary and evidence.

External validation uses the existing independently built Release LLVM 14.0.6 provider, CUDA 12.9
libNVVM/NVRTC/`ptxas`, and optional CUDA driver/device. Explicit broken provider configuration is a
failure; genuinely missing optional GPU/toolkit prerequisites may ignore only their real lanes.

## Milestones

1. Reconfirm canonical SSA and loop ownership.
   Inspect the final linked IR for all three sources and the implementations of `IRLoop`,
   predecessor enumeration, branch arguments, and dominator checks. Record exact op/API evidence
   here. Promotion requires that constants, phis, and loop edges are canonical producer shapes and
   need no pipeline repair.

2. Publish one coherent ScalarSSA provider capability.
   Append the ABI prefix, host negotiation/wrappers/identity, C probe, and LLVM 14 operations.
   Tests cover old exact prefixes, partial/null/future tables, output sanitation, representability,
   missing insertion block, phi placement, type/module/function mismatch, non-predecessor and
   duplicate incoming blocks, non-dominating values, terminated blocks, and no mutation.

3. Extend complete Slang IR preflight and phased lowering.
   Accept exact i32 constants, non-entry i32 parameters, matching branch args, and canonical loop;
   create blocks/phis before bodies and incoming pairs after bodies. Unsupported shapes still fail
   before builder discovery. Exact fake graphs prove values, parameter indices, constants, phi
   targets/incoming predecessors, backedge values, branches, and final verified bitcode handoff.

4. Prove real output through both PTX routes.
   Compile identical add-one, merge, and loop sources through NVVM and NVRTC. Compare stable ABI
   widths and global-memory semantics. Require real LLVM verification and matching-architecture
   `ptxas` for both. Do not require PTX phi/backedge mnemonics because optimization may remove the
   merge or derive a closed-form sum.

5. Prove runtime semantics.
   Launch both routes with one thread using the shared dynamic driver helper. Check add-one `42`,
   select less/greater/equal cases, and loop limits `0`, `5`, and `7`. Include negative inputs where
   they distinguish signed behavior. Ignore only absent driver/device/toolkit prerequisites.

6. Preserve established routes and hand off calls.
   Run the full NVVM prefix plus routing/default NVRTC/pass-through/barrier/CUDA runtime regressions.
   Keep calls and multiplication deterministic E52017 before builder discovery. Inspect provider
   exports/dependencies, format with pinned clang-format 17, run `git diff --check`, complete the
   helper/input-shape audit, and update design/ledger and this plan.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox. Use Windows-native tools from `C:\src\slang`.

```text
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build --config Release `
    --target slang-llvm-nvvm -- /m
cmake.exe --build --preset debug --target slang-test
```

Provider-independent focused tests (exact names finalized during implementation):

```text
$env:SLANG_NVVM_BUILDER_PATH = $null
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarSSAAPI
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangScalarSSAUsesDirectPipeline
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangNegotiatesScalarSSACapability
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission
```

Real provider, differential, assembly, and runtime:

```text
$env:SLANG_NVVM_BUILDER_PATH = `
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderBuildsScalarSSALoopKernel
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangRealScalarDifferentialPTX
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangRealScalarPtxasAccepts
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangScalarRuntimeMatchesNVRTC
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Established regressions:

```text
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/parseCUDAEmissionMethods
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/cudaEmissionMethod
build\Debug\bin\slang-test.exe -skip-api-detection `
    tests/cuda/nvvm-unsupported-ir
build\Debug\bin\slang-test.exe -skip-api-detection `
    tests/cuda/sampler-comparison-state-unused
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/coverageCudaRuntimeDispatch
```

Acceptance requires:

- exact raw ABI and fake graph for all three sources;
- old Slice 7 provider retaining prior capabilities and rejecting ScalarSSA before module creation;
- every partial/incomplete ScalarSSA prefix rejected and every failed output sanitized;
- invalid provider constant/phi calls causing no instruction/incoming mutation;
- complete Slang legality before builder discovery for call/mul and other named next shapes;
- verified real LLVM bitcode containing the intended constants/phis/backedge before libNVVM;
- same-source NVRTC/NVVM parameter widths and global-memory semantics;
- `ptxas` acceptance for both routes when available;
- matching runtime values on a configured GPU, or a documented ignore only for absent optional
  prerequisites;
- unchanged Slice 6/7, default/explicit NVRTC, true pass-through, routing/hash/loader, barrier, and
  existing CUDA runtime results;
- unchanged two-export provider allowlist and no process-visible LLVM DLL dependency;
- pinned clang-format 17 changed-line/new-file success, `git diff --check`, and no generated
  PTX/cubin/log artifacts; and
- a completed helper/special-case inventory with no SSA-to-stack repair, inferred incoming edge,
  ignored structured operand, or silent fallback.

## Failure and Recovery

All production behavior remains gated by explicit `SLANG_EMIT_CUDA_VIA_NVVM`. Removing the three
appended V2 fields and restoring Slice 7 constant/branch-argument/loop rejection returns cleanly to
the prior boundary. V1 and earlier V2 prefixes stay immutable, so old providers remain compatible.

Provider operations validate completely before creating a constant/phi or adding an incoming pair.
Module scope cleanup destroys all LLVM state on failure. Slang preflight occurs before optional
builder discovery. An old valid provider receives E52016 only for a shape requiring ScalarSSA; an
unsupported Slang shape receives E52017 independently of machine setup. Explicit direct NVVM never
falls back through NVRTC.

Keep temporary PTX/cubin files in existing test-owned artifact storage. Do not remove or commit
unrelated `external/slang-binaries/`. Do not commit this active ExecPlan or the completed Slice 7
ExecPlan.

## Artifacts and Hand-Off

Keep this plan current with exact IR shapes, commands/counts, diagnostics, rejected alternatives,
tool/provider versions, runtime status, and the final input-shape audit. Distill stable architecture
into `docs/design/nvvm-backend.md`, capability evidence into
`docs/design/nvvm-backend-capability-ledger.md`, and the change narrative into the required
five-part PR description.

The next bounded plan begins with direct calls and non-void helper ABI. It must retain only helpers
reachable from the selected entry point, validate signatures and call operands before provider
discovery, and must not treat every module function as an entry point or reconstruct syntax from
checked semantic values.
