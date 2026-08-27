# OR signed i32 values through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the prepared,
uncommitted successor working log for Slice 14 of the direct NVVM backend experiment. Do not begin
tracked implementation until Slice 13 has completed and been committed.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts the next smallest exact bitwise expression: bitwise
OR of two signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x | y;
}
```

The final linked Slang IR must contain one exact two-operand `kIROp_BitOr` whose operands and result
are signed `i32`; that result feeds the established device-pointer store. The provider emits LLVM
`or i32` only after validating the current unterminated insertion block and two available,
same-function scalar integer operands of identical LLVM type. Direct NVVM and NVRTC must agree on
raw parameter widths `[64, 32, 32]`, expose a 32-bit bitwise-OR operation and global store,
assemble through `ptxas`, and produce identical positive, negative-bit-pattern, and zero results at
runtime.

## Progress

- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the Slice 13 ExecPlan, and the current producer,
  preflight/emission, provider ABI, fake-provider, PTX-summary, `ptxas`, and runtime-test patterns.
- [x] (2026-08-27) Compared signed-i32 OR, XOR, NOT, shifts, division/remainder, pointer helper ABI,
  and local arrays; selected exact `kIROp_BitOr` as the smallest independently executable boundary.
- [x] (2026-08-27) Prepared this planning-only hand-off and parameterized probe without editing
  tracked files, building, testing, or probing.
- [x] (2026-08-27) After committing Slice 13 as `slice 13`, measured the final
  post-`simplifyIR` topology, direct E52017 `'or'` boundary, and explicit NVRTC
  `[64,32,32]`/`or.b32`/`st.global.u32` evidence using explicit output files.
- [x] (2026-08-27) Appended and implemented the coherent dedicated integer-bit-OR provider
  suffix, host negotiation/identity/wrapper, provider validation, and strict-C probes; the first
  Release provider build is green.
- [x] (2026-08-27) Extended direct-NVVM preflight, terminal capability gating, and canonical
  value-map emission for exact two-operand signed-i32 `kIROp_BitOr`; the Debug `slangc` build and
  first integrated direct compile are green.
- [x] (2026-08-27) Added the eight ABI, provider, fake-topology, capability-gate, PTX, `ptxas`,
  runtime, and negative-boundary tests. The post-format focused prefix passes 100/100.
- [x] (2026-08-27) Applied pinned `clang-format`, rebuilt both the Release provider and Debug
  `slang-test`, ran the complete validation matrix outside the sandbox, inspected the provider
  binary, and completed the helper/input-shape audit. The tracked Slice 14 changes are ready for
  the parent to commit as `slice 14`; the final commit hash remains pending.

## Surprises and Discoveries

- Observation: ordinary source bitwise OR has a direct canonical opcode and no signedness-dependent
  LLVM operation.
  Evidence: `source/slang/slang-lower-to-ir.cpp` maps `BuiltinOperationKind::BitOr` to
  `kIROp_BitOr`; `IRBuilder::emitBitOr` constructs that exact instruction; and
  `source/slang/slang-ir-insts.lua` gives it the textual name `or`. LLVM integer OR is signless.
  Consequence: admit exact ordinary `kIROp_BitOr` and emit `CreateOr`; do not accept
  `kIROp_ConstexprBitOr`, logical `kIROp_Or`, or another opcode as a fallback.

- Observation: OR is smaller than the adjacent candidates even though XOR is nearly identical.
  Evidence: BitNot needs a unary provider-validation and fake-value path; shifts must reconcile
  Slang's out-of-range constant-folding behavior with LLVM poison for counts at least the bit
  width; signed division and remainder have zero and `INT_MIN / -1` undefined-behavior boundaries;
  pointer helpers cross call/return and access-qualified type ABI; local arrays add allocation and
  generic-address-space semantics. XOR is equally direct but is independently executable and
  remains useful negative coverage.
  Consequence: implement OR alone. Do not bundle XOR merely because its tests would look similar.

- Observation: route-sensitive CLI probes require an explicit output artifact.
  Evidence: Slice 13 measured that omitting `-o` allowed stdout compilation to follow NVRTC even
  when the direct selector was present; the same command with `-o` reached the direct preflight.
  Consequence: every Slice 14 direct and NVRTC CLI probe names a distinct `-o` path. API routing
  tests remain the authoritative selector/hash proof.

- Observation: the frozen Slice 7 ADD/SUB callable and the dedicated Slice 13 AND callable cannot
  be widened to include OR.
  Evidence: `SlangNVVMIntegerBinaryOp_2` publishes only ADD/SUB and its provider rejects unknown
  values; Slice 13 negotiates AND through its own terminal function pointer.
  Consequence: append a dedicated `emitIntegerBitOr` field. Do not extend either old operation's
  input domain or invent a generic bitwise spelling that duplicates the AND source of truth.

- Observation: a substring search for `or` could accidentally match the tail of `xor`.
  Evidence: the established PTX classifier already requires an instruction-token boundary and an
  unambiguous integer width.
  Consequence: add an entry-scoped, token-boundary 32-bit OR classification and retain XOR as a
  negative source. Do not weaken the classifier to arbitrary substring presence.

- Observation: the measured promotion source exactly preserves the planned canonical topology.
  Evidence: every repeated dump contains
  `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)`, one
  `let %result : Int = or(%x, %y)`, and `store(%destination, %result)`. With a deliberately
  missing builder path, explicit direct output reports E52017 `'or'`, proving preflight stops before
  provider discovery. Explicit NVRTC succeeds with `[64,32,32]`, `or.b32`, and `st.global.u32`.
  Consequence: implement exact ordinary `kIROp_BitOr`; no producer change, spelling fallback,
  syntax reconstruction, or signedness adaptation is needed.

- Observation: the first integrated direct compile preserves the exact NVRTC OR/store semantics.
  Evidence: both outputs have `[64,32,32]`, `or.b32`, and `st.global.u32`. Direct NVVM targets
  `sm_70` and uses the address-space-1 parameter directly; NVRTC's local `sm_75` output includes
  its expected `cvta.to.global.u64`. The launch widths and relevant operation/store semantics are
  identical.
  Consequence: retain semantic entry-summary comparison, both assembler lanes, and runtime truth
  tables instead of demanding textual PTX equality.

- Observation: the completed executable matrix confirms the measured prototype at every route.
  Evidence: direct NVVM and NVRTC both report parameter widths `[64,32,32]`, an entry-scoped exact
  `or.b32`, and a global 32-bit store; both outputs assemble with `ptxas`; and both runtime routes
  produce `0x5a | 0x3c == 0x7e`, `-16 | 3 == -13`, and `0 | -1 == -1`.
  Consequence: the exact opcode classifier is neither a textual-equality requirement nor a
  substring false positive, while assembly and runtime cover executable semantics.

- Observation: provider linkage remains isolated after adding the new callback.
  Evidence: binary inspection exposes only the V1 and V2 builder API getters. The provider's
  ordinary import is `KERNEL32.dll`; delayed imports are `SHELL32.dll` and `ole32.dll`, with no
  LLVM DLL dependency.
  Consequence: Slice 14 preserves the statically linked optional-provider boundary.

## Decision Log

- Decision: Slice 14 is exact signed-i32 bitwise OR, not XOR, NOT, shifts, division/remainder,
  pointer helper ABI, or local arrays.
  Rationale: OR is one stable ordinary opcode with exact signless LLVM semantics and reuses the
  complete binary-integer validation, scalar ABI, value map, store, PTX, and runtime machinery.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the post-Slice-13 probe does not retain exact `kIROp_BitOr`.

- Decision: append `emitIntegerBitOr` as a dedicated V2 operation.
  Rationale: widening the frozen ADD/SUB enum would silently change an old callable's domain, and
  reusing the dedicated AND field would create a second opcode-selection contract. One terminal
  function pointer negotiates and dispatches OR atomically.
  Date/Author: 2026-08-27, Codex.
  Revisit when: an ABI audit finds an existing versioned operation whose published domain already
  includes OR.

- Decision: preserve Slice 13's expected 280-byte x64 prefix and publish a 288-byte Slice 14
  terminal prefix.
  Rationale: the only appended member is one 64-bit function pointer. Sizes 281 through 287 are
  partial and malformed; 280 remains a complete Slice 13 provider; future-larger tables are
  accepted and clamped to the locally known 288 bytes.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the completed Slice 13 build or strict-C offset probes contradict those sizes.

- Decision: use ordinary LLVM `IRBuilder::CreateOr` after complete validation.
  Rationale: exact signed-i32 Slang OR and LLVM's signless scalar integer OR have identical
  bit-pattern semantics and no exceptional inputs.
  Date/Author: 2026-08-27, Codex.
  Revisit when: verified LLVM or direct/NVRTC runtime evidence exposes a representation mismatch.

## Outcomes and Retrospective

Slice 14 is complete and ready to commit. The post-format focused NVVM suite passes 100/100. The
preservation matrix passes, in command order, 1/1 `parseCUDAEmissionMethods`, 2/2
`cudaEmissionMethod`, 1/1 `tests/cuda/nvvm-unsupported-ir`, 3/3
`tests/cuda/sampler-comparison-state-unused`, 2/2 `tests/cuda/cuda-compile`, and 1/1
`coverageCudaRuntimeDispatch`. Both the Release `slang-llvm-nvvm` provider and Debug `slang-test`
targets rebuilt successfully after formatting, and the pinned `clang-format` check is clean.

The ABI evidence freezes the 280-byte Slice 13 prefix and completes the 288-byte Slice 14 prefix.
Exact-old, every partial size 281--287, full-null, full, future-larger, uninitialized, and output-
sanitization cases all behave as specified. The verified LLVM module contains exactly one
`or i32` feeding the store, and invalid calls neither retain stale outputs nor mutate the module.
The fake route proves that kernel parameters 1 and 2 are the OR operands and that the returned
value is stored without dispatching an adjacent callback. The exact Slice 13 table still compiles
AND, then gates OR after one discovery and before module creation or libNVVM use.

Differential evidence is `[64,32,32]`, exact entry-scoped `or.b32`, and a global unsigned 32-bit
store for both direct NVVM and NVRTC. Both PTX artifacts pass `ptxas`. Both runtime routes agree on
`0x5a | 0x3c == 0x7e`, `-16 | 3 == -13`, `0 | -1 == -1`, and
`0x55555555 | 0x0f0f0f0f == 0x5f5f5f5f`. Binary inspection finds only the V1
and V2 exported getters, `KERNEL32.dll` as the ordinary dependency, and delayed `SHELL32.dll` and
`ole32.dll`; there is no LLVM DLL dependency.

The helper/special-case inventory found no new compiler fallback, custom semantic equality,
operand-graph walk, syntax reconstruction, or producer repair. The surviving additions are the
dedicated ABI support predicate/wrapper and provider callback, the fake callback/value-index and
module-construction test helpers, and one entry-scoped PTX summary bit. The partial-prefix check is
an append-only ABI invariant, and the terminal capability gate is the established pre-module
negotiation invariant; neither compensates for malformed IR. The audited input is the canonical
two-operand signed-i32 `kIROp_BitOr` produced by ordinary lowering, and removal of its exact
preflight/emission case restores the focused unsupported-op failure. Therefore the producer shape
is intentional and this emitter/provider boundary owns the new operation.

Final commit: `slice 14`.

## Context and Current Pipeline

The motivating `x | y` expression is semantically resolved as `BuiltinOperationKind::BitOr`.
`source/slang/slang-lower-to-ir.cpp` selects `kIROp_BitOr` and emits an ordinary instruction with
the result type and two operands. `IRBuilder::emitBitOr` is the named canonical constructor used by
other IR producers. The parameterized source prevents constant folding; the promotion probe must
confirm that linking, optimization, and repeated `simplifyIR` retain one signed-i32
`kIROp_BitOr` feeding `store(destination, result)`.

`source/slang/slang-emit.cpp` links and optimizes the selected CUDA entry point, calls
`validateNVVMSupportedIR`, discovers the optional LLVM 14 provider only after semantic preflight,
and checks the maximum `NVVMIRCapability` before creating a builder module.
`source/slang/slang-emit-nvvm.cpp` validates the finite direct-call closure in dominance order,
declares functions and parameters, maps canonical Slang IR values to provider handles, emits each
body, verifies and serializes once, and hands LLVM bitcode to libNVVM.

Slice 13's `SlangNVVMEmitIntegerBitAnd_2` is the intended append-only predecessor.
`_validateI32Value` is the source of truth for exact signed-i32 constants and available SSA
values. The first instruction pass owns exact opcode, operand count, and result-type admission;
the second pass owns operand type, availability, and dominance; the body-emission switch consumes
the canonical value map. Slice 14 adds one operation at each existing boundary. It does not change
AST checking/lowering, reconstruct syntax, repair malformed IR, or introduce another value
representation.

Before this slice, exact `kIROp_BitOr` should reach the validator's default first-pass diagnostic.
Because the opcode's stable textual spelling is `or`, the expected stop is E52017
`direct NVVM lowering does not support ... 'or'`, with no builder load request. This is a
promotion hypothesis until it is measured after Slice 13 commits; do not implement a downstream
fallback if the final shape differs.

## Scope and Non-Goals

In scope:

- exact ordinary `kIROp_BitOr` with exactly two signed-i32 operands and a signed-i32 result;
- parameters, exact representable constants, loads, add/subtract/multiply/AND results, phis, calls,
  and other already-supported signed-i32 producers as operands when available and dominant;
- one appended provider operation, terminal capability, stable identity bit, and sanitized host
  wrapper;
- fake topology, verified LLVM, direct/NVRTC differential PTX, both `ptxas` lanes, and runtime
  bit-pattern evidence.

Explicitly out of scope:

- `kIROp_BitXor`, `kIROp_BitNot`, shifts, logical AND/OR, new comparisons, or select changes;
- `kIROp_ConstexprBitOr` or logical `kIROp_Or` as alternate accepted spellings;
- unsigned, bool, 8/16/64-bit, arbitrary-precision, vector, matrix, or aggregate bitwise values;
- widening the raw kernel/helper ABI or changing ADD/SUB/multiply/AND semantics;
- division, remainder, overflow or saturation variants, casts, bitfields, atomics, reductions,
  waves, resources, pointer masking, local/shared/global declarations, thread builtins, barriers,
  or libdevice;
- performance claims beyond semantic PTX classification and successful assembly/runtime.

## Architecture and Invariants

Capability selection remains monotonic. Add terminal `NVVMIRCapability::ScalarIntegerBitOr`
after Slice 13's `ScalarIntegerBitAnd`. An exact Slice 13 provider remains valid and compiles every
previously published program; a bit-OR program reaches E52016 after provider discovery but before
builder-module creation or libNVVM use.

Append exactly one `SlangNVVMEmitIntegerBitOr_2` pointer to `SlangNVVMBuilderAPI_V2`. Preserve
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE`, expected to be 280 bytes on x64, and
publish `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE`, expected to be 288 bytes.
Require:

- `offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitOr)` equals the frozen Slice 13 minimum;
- size 280 initializes successfully with `supportsScalarIntegerBitOr() == false`;
- every size greater than 280 and less than 288 is rejected as a partial suffix;
- size at least 288 requires a non-null `emitIntegerBitOr` member;
- a future-larger table is accepted, copied only through 288 bytes, and reports local size 288.

The provider identity appends `scalar-integer-bit-or=0|1`, so shader-cache identity differs when
the capability differs. Exact old prefixes report zero; a coherent full/future prefix reports one.

Slang preflight accepts only exact two-operand `kIROp_BitOr` with signed-i32 result. Both operands
pass `_validateI32Value`; the result joins the existing available-value set and emission map and
may feed any existing signed-i32 consumer. No custom equality, opcode fallback, operand-graph walk,
or syntax reconstruction is permitted.

The host wrapper clears its output before dispatch and passes a private cleared slot. It also
clears after a failed provider call and converts success-without-handle to failure. Unsupported or
uninitialized builders return the established error without exposing a stale handle.

Provider `_emitIntegerBitOr` clears a non-null output first, obtains a live current unterminated
insertion block with `_getValidInsertionBlock`, and validates both handles through
`_areMatchingIntegerValues`. Thus operands must be scalar LLVM integers of exactly equal type,
belong to the same module/context and current function, and be available/dominant at the insertion
point. Only after every check passes may it call `state->builder.CreateOr(left, right)` and publish
the result. Invalid calls add no LLVM instruction.

## Interfaces and Dependencies

Append after Slice 13 in `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitOr_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue);
```

Add:

- table member `emitIntegerBitOr`;
- `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE`;
- host `supportsScalarIntegerBitOr()` and `emitIntegerBitOr(...)`;
- identity component `scalar-integer-bit-or=0|1`;
- terminal `NVVMIRCapability::ScalarIntegerBitOr` and its pre-module gate.

No public Slang API changes. The implementation retains the optional statically linked LLVM
14.0.6 provider, libNVVM/NVRTC, CUDA `ptxas`, and CUDA-driver environment gates already used by
the focused suite.

## Milestones

1. After committing Slice 13, run the retained parameterized probe through final linking with
   `-dump-ir` and direct NVVM, always with an explicit `-o`. Confirm the exact function signature,
   one two-operand signed-i32 `kIROp_BitOr`, its store consumer, expected E52017 `'or'`, and zero
   builder discovery in a focused fake test. Compile the same source through explicit NVRTC and
   record `[64,32,32]`, 32-bit OR, and global-store PTX. Promotion requires this stable canonical
   shape. If it folds, changes opcode, or introduces a cast, investigate the producer and revise
   the slice instead of adding a downstream spelling fallback.

2. Freeze the provider suffix in `source/compiler-core/slang-nvvm-ir-builder-api.h`, add strict-C
   minimum-size and capability-order probes in
   `source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c`, and update coherent host negotiation,
   support query, identity, sanitized wrapper, and provider getter. Prove exact Slice 13, partial
   281--287, full-null, full, and future-larger behavior plus uninitialized, unsupported,
   invalid-input, success-null, and failure-after-write output clearing.

3. Implement provider `_emitIntegerBitOr` in
   `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` by reusing `_getValidInsertionBlock` and
   `_areMatchingIntegerValues`, then calling `CreateOr`. Add a verified positive module with
   exactly one `or i32` feeding the store. Add invalid/no-mutation cases for null module/output,
   no or terminated insertion block, pointer/non-integer operand, mismatched integer width,
   foreign module/context/function, same-block use after insertion, and sibling/non-dominating
   instruction values.

4. Extend `source/slang/slang-emit-nvvm.{h,cpp}` and the capability gate in
   `source/slang/slang-emit.cpp`. Add exact first-pass admission, terminal capability requirement,
   second-pass `_validateI32Value` checks, available-value registration, and body emission through
   the canonical value map and `builder.emitIntegerBitOr`. Keep preflight and emission switches
   structurally parallel.

5. Extend the fake provider and fixtures in
   `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`. Prove the operation receives kernel
   parameters 1 and 2 in the entry block; its result is the store value; and no old integer-binary,
   multiply, AND, load, branch, call, phi, pointer-offset, or array operation is used. Gate a
   bit-OR source against an exact Slice 13 provider after discovery/before module creation, while
   proving bit AND still works on that provider.

6. Preserve deterministic adjacent boundaries. Keep signed-i32 XOR at E52017 `'xor'`, bitwise NOT
   at `'bitnot'`, shifts at `'shl'`/`'shr'`, division/remainder at `'div'`/`'irem'`, and pointer
   helper/local-array fixtures at their established semantic boundaries before builder discovery.
   Keep raw unsigned/wide integer OR at the existing `'entry-point parameter'` boundary. Do not
   use semantically invalid floating-point bitwise fixtures merely to manufacture diagnostics.

7. Compile the parameterized source through direct NVVM and NVRTC. Extend `PTXEntrySummary` with
   entry-scoped, token-boundary 32-bit bitwise-OR classification that cannot confuse `xor`; compare
   `[64,32,32]`, OR, and global-store semantics; assemble both outputs; and launch both routes for
   representative bit patterns such as `0x5a | 0x3c == 0x7e`, `-16 | 3 == -13`, and
   `0 | -1 == -1`.

8. Apply pinned formatting, rebuild, run the complete focused and preservation matrices outside
   the sandbox, inspect exports/dependencies, perform the required helper/input-shape audit, update
   durable docs, and commit only tracked Slice 14 files as `slice 14`.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. Every CMake build and test must run outside the
sandbox as required by `AGENTS.md`.

Prototype commands after Slice 13 is committed:

```text
$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvvm -dump-ir `
  -o issue-nvvm-backend\probe.slice14.direct.ptx `
  issue-nvvm-backend\probe.slice14.i32-bit-or.slang
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvrtc `
  -o issue-nvvm-backend\probe.slice14.nvrtc.ptx `
  issue-nvvm-backend\probe.slice14.i32-bit-or.slang
```

Build and focused test commands:

```text
cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm
cmake.exe --build build --config Debug --target slang-test

$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Follow the established eight-test pattern with these focused contracts:

- `nvvmIRBuilderNegotiatesScalarIntegerBitOrAPI`;
- `nvvmIRBuilderRejectsInvalidIntegerBitOrOperations`;
- `nvvmIRBuilderBuildsIntegerBitOrKernel`;
- `nvvmSlangIntegerBitOrUsesDirectPipeline`;
- `nvvmSlangNegotiatesScalarIntegerBitOrCapability`;
- `nvvmSlangRealIntegerBitOrDifferentialPTX`;
- `nvvmSlangRealIntegerBitOrPtxasAccepts`;
- `nvvmSlangIntegerBitOrRuntimeMatchesNVRTC`.

Slice 13's actual baseline was 92/92. These eight independent tests complete the measured,
post-format Slice 14 focused prefix at 100/100.

Re-run the established preservation matrix:

```text
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/parseCUDAEmissionMethods
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/cudaEmissionMethod
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/nvvm-unsupported-ir
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/sampler-comparison-state-unused
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/coverageCudaRuntimeDispatch
```

Acceptance requires:

- measured final linked topology: exact two-operand signed-i32 `kIROp_BitOr` feeding the store;
- measured pre-change E52017 `'or'` before builder discovery;
- dedicated ABI field with a complete 288-byte Slice 14 x64 prefix and exact frozen 280-byte
  Slice 13 compatibility;
- strict partial/null/future negotiation, stable identity bit, and complete output sanitization;
- provider invalid calls clear outputs and add no LLVM instruction;
- verified LLVM contains exactly one `or i32` whose result feeds the global store;
- fake topology proves exact parameter operands, result/store flow, and absence of unrelated calls;
- an exact Slice 13 provider compiles prior bit-AND work but gates bit OR after discovery and before
  module or libNVVM creation;
- signed XOR/NOT/shifts/division/remainder and unsigned/wide OR boundaries remain deterministic;
- direct NVVM/NVRTC expose `[64,32,32]`, 32-bit bitwise-OR, and global i32 store semantics;
- both PTX outputs assemble and both runtime routes agree for positive, negative, and zero masks;
- the final focused prefix and preservation matrix pass after pinned formatting;
- the provider still exports only V1/V2 getters and has no LLVM DLL dependency;
- every new helper/fallback/special case survives the required principled input-shape audit.

## Failure and Recovery

The probe, incremental builds, focused tests, formatter, and binary inspection are safe to repeat.
If the final expression becomes `kIROp_ConstexprBitOr`, folds, or acquires an unexpected cast,
audit the producer and optimization trace; do not accept multiple shapes downstream merely to make
the fixture pass. Retain a parameterized source so PTX cannot remove the operation as constant.

If LLVM verification fails, fix provider validation or ownership/dominance before serialization.
If a CUDA toolchain spells the optimized operation differently, first prove its entry-scoped
truth-table semantics and record the discovery; do not weaken the PTX classifier to an arbitrary
substring. `ptxas` and runtime equality remain required executable evidence.

If ABI layout differs from 288 bytes, stop and inspect field ordering/alignment rather than
changing the frozen 280-byte predecessor. If a partial or null table initializes, fix negotiation
before enabling emission. If a failed provider call leaves a handle or instruction, fix
sanitization/no-mutation at the provider or wrapper boundary before continuing.

Do not delete/reset the user's worktree or stage `external/slang-binaries/`, any ExecPlan, or probe
artifact. Remove `probe.slice14.i32-bit-or.slang` and generated PTX/dumps with `apply_patch` before
committing. The direct route remains experimental and removable without changing default NVRTC
dispatch.

## Artifacts and Hand-Off

Retain in this plan:

- exact linked IR and measured baseline diagnostic/discovery counters;
- compile-time/runtime prefix sizes and old/partial/null/full/future negotiation matrix;
- verified LLVM assembly and invalid/no-mutation matrix;
- fake producer-to-consumer topology and capability-gate counters;
- direct/NVRTC PTX summaries, both `ptxas` results, and runtime values;
- final focused/preservation counts, exports/dependencies, formatter evidence, and helper/input-
  shape audit;
- final `slice 14` commit hash.

Distill stable architecture into `docs/design/nvvm-backend.md`, durable coverage into
`docs/design/nvvm-backend-capability-ledger.md`, and the five-part implementation narrative into
the eventual PR description. Keep this and all probe artifacts untracked.
