# XOR signed i32 values through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the prepared,
uncommitted successor working log for Slice 15 of the direct NVVM backend experiment. Do not begin
tracked implementation until Slice 14 has completed and been committed.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts the next exact signed-integer bitwise expression:
bitwise XOR of two signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x ^ y;
}
```

The final linked Slang IR must contain one exact two-operand `kIROp_BitXor` whose operands and
result are signed `i32`; that result feeds the established device-pointer store. The provider emits
LLVM `xor i32` only after validating the current unterminated insertion block and two available,
same-function scalar integer operands of identical LLVM type. Direct NVVM and NVRTC must agree on
raw parameter widths `[64, 32, 32]`, expose `xor.b32` and a global 32-bit store, assemble through
`ptxas`, and produce identical positive, negative-bit-pattern, sign-clearing, and zero-identity
results at runtime.

## Progress

- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the current Slice 14 ExecPlan/probe, and the
  established provider ABI, fake-provider, PTX-summary, `ptxas`, and runtime-test structure.
- [x] (2026-08-27) Prepared this planning-only hand-off and parameterized XOR probe without
  editing tracked files, building, testing, or running the probe.
- [x] (2026-08-27) After committing Slice 14 as `slice 14`, measured the final
  post-`simplifyIR` XOR topology and direct E52017 boundary, then generated explicit NVRTC PTX.
- [x] (2026-08-27) Appended and implemented the coherent dedicated integer-bit-XOR provider
  suffix, host negotiation/identity/wrapper, provider validation, and strict-C probes; the Release
  provider builds successfully.
- [x] (2026-08-27) Extended direct-NVVM preflight, terminal capability gating, and canonical
  value-map emission for exact two-operand signed-i32 `kIROp_BitXor`; the Debug compiler builds and
  the integrated direct route emits the expected PTX.
- [x] (2026-08-27) Added the eight ABI, provider, fake-topology, capability-gate,
  negative-boundary, PTX, `ptxas`, and runtime tests; the first integrated focused run passes
  108/108 and the preservation matrix passes in full.
- [x] (2026-08-27) Applied pinned formatting, rebuilt the Release provider and Debug
  `slang-test`, reran the complete focused and preservation matrices outside the sandbox, audited
  the production and test diffs, inspected the provider binary, and finalized the durable docs.
  The post-format focused run also passes 108/108. The tracked work is ready for the parent to
  commit as `slice 15`; the final commit hash remains pending.

## Surprises and Discoveries

- Observation: ordinary source XOR has its own canonical opcode and signless LLVM operation.
  Evidence: `source/slang/slang-lower-to-ir.cpp` maps `BuiltinOperationKind::BitXor` to
  `kIROp_BitXor`; `IRBuilder::emitBitXor` constructs that exact ordinary instruction; and
  `source/slang/slang-ir-insts.lua` gives the opcode the textual name `xor`.
  Consequence: admit exact ordinary `kIROp_BitXor` and emit `CreateXor`. Do not accept
  `kIROp_ConstexprBitXor`, logical inequality, or another opcode as a fallback.

- Observation: XOR is independently executable even though its provider and test shapes parallel
  AND and OR.
  Evidence: the frozen ADD/SUB enum publishes only those two operations, while Slices 13 and 14
  negotiate AND and OR through distinct terminal function pointers. Changing any older callable's
  input domain would break append-only capability negotiation.
  Consequence: append a dedicated `emitIntegerBitXor` operation. Do not widen the old arithmetic,
  AND, or OR contracts or introduce a second generic bitwise opcode mapping.

- Observation: the remaining adjacent operations carry genuinely different contracts.
  Evidence: bitwise NOT is unary; shifts must reconcile Slang's shift-count policy with LLVM poison;
  signed division and remainder have zero and `INT_MIN / -1` exceptional boundaries; pointer
  helper ABI and local arrays cross type/address-space boundaries.
  Consequence: Slice 15 contains XOR alone. Retain NOT, shifts, division, remainder, pointer-helper
  ABI, and local arrays as deterministic negative coverage.

- Observation: the post-Slice-14 baseline matches the canonical promotion hypothesis exactly.
  Evidence: the explicit-output direct probe repeatedly retains
  `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)`, one
  `let %result : Int = xor(%x, %y)`, and `store(%destination, %result)` through the final
  `simplifyIR`; it then stops at E52017 `'xor'`. Explicit NVRTC succeeds with `.param .u64`, two
  `.param .u32` values, `xor.b32`, `cvta.to.global.u64`, and `st.global.u32`.
  Consequence: promote only exact ordinary signed-i32 `kIROp_BitXor`; no downstream spelling or
  representation fallback is needed.

- Observation: the promoted production path preserves the same PTX contract as NVRTC.
  Evidence: after the Release provider and Debug compiler builds, explicit direct NVVM and NVRTC
  both expose `[64,32,32]`, exact `xor.b32`, and `st.global.u32`; the direct path stores through the
  raw global pointer, while NVRTC first emits `cvta.to.global.u64`.
  Consequence: keep semantic entry-scoped PTX classification rather than requiring textual PTX
  identity.

- Observation: the complete integrated test prefix passed without an ABI-test correction cycle.
  Evidence: the first focused run passed 108/108, including the verified one-`xor i32` module,
  invalid/no-mutation matrix, fake producer/store topology, exact Slice 14 compatibility gate,
  both CUDA 12.9 `ptxas` lanes, and RTX 5090 runtime results `0x66`, `-305419897`, `15`, and `-1`.
  The preservation matrix passed 1/1, 2/2, 1/1, 3/3, 2/2, and 1/1 in command order.
  Consequence: the completed slice has executable evidence at the ABI, LLVM, PTX assembler, and
  CUDA runtime boundaries rather than relying on source inspection alone.

- Observation: formatting and rebuilding did not change the focused or preservation outcomes.
  Evidence: the pinned formatter is clean; the Release provider and Debug `slang-test` targets
  both rebuild successfully; and the post-format focused run remains 108/108. The preservation
  results remain 1/1, 2/2, 1/1, 3/3, 2/2, and 1/1 in command order.
  Consequence: the final recorded evidence corresponds to the exact formatted tree prepared for
  commit.

- Observation: the provider retains its intended optional, statically linked boundary.
  Evidence: binary inspection exposes only the V1 and V2 builder API getters. `KERNEL32.dll` is
  the ordinary dependency; `SHELL32.dll` and `ole32.dll` are delayed dependencies; and there is no
  LLVM DLL dependency.
  Consequence: appending XOR did not leak LLVM linkage or another export into the host process.

## Decision Log

- Decision: Slice 15 is exact signed-i32 bitwise XOR, not NOT, shifts, division/remainder, pointer
  helper ABI, or local arrays.
  Rationale: XOR is one stable ordinary opcode with exact LLVM bit-pattern semantics and reuses the
  established binary-integer validation, scalar ABI, value map, store, PTX, and runtime machinery.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the post-Slice-14 promotion probe does not retain exact `kIROp_BitXor`.

- Decision: append `emitIntegerBitXor` as a dedicated V2 operation.
  Rationale: widening the frozen ADD/SUB enum or reusing the dedicated AND/OR callables would
  silently change an older provider's published input domain. One terminal function pointer
  negotiates support and dispatch atomically.
  Date/Author: 2026-08-27, Codex.
  Revisit when: an ABI audit finds an existing versioned operation whose published domain already
  includes XOR.

- Decision: preserve Slice 14's 288-byte x64 prefix and publish a 296-byte Slice 15 terminal
  prefix.
  Rationale: the only appended member is one 64-bit function pointer. Sizes 289 through 295 are
  partial and malformed; 288 remains a complete Slice 14 provider; future-larger tables are
  accepted and clamped to the locally known 296 bytes.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the completed Slice 14 build or strict-C offset probes contradict those sizes.

- Decision: use ordinary LLVM `IRBuilder::CreateXor` after complete validation.
  Rationale: exact signed-i32 Slang XOR and LLVM's signless scalar integer XOR have identical
  bit-pattern semantics and no exceptional operand values.
  Date/Author: 2026-08-27, Codex.
  Revisit when: verified LLVM or direct/NVRTC runtime evidence exposes a representation mismatch.

## Outcomes and Retrospective

Slice 15 is complete and ready for its parent commit. Both the first integrated and post-format
focused NVVM runs pass 108/108. The preservation matrix passes, in command order, 1/1
`parseCUDAEmissionMethods`, 2/2 `cudaEmissionMethod`, 1/1
`tests/cuda/nvvm-unsupported-ir`, 3/3 `tests/cuda/sampler-comparison-state-unused`, 2/2
`tests/cuda/cuda-compile`, and 1/1 `coverageCudaRuntimeDispatch`. The Release
`slang-llvm-nvvm` provider and Debug `slang-test` targets both build successfully, and the pinned
format check is clean.

The negotiated ABI preserves the exact 288-byte Slice 14 prefix and completes the 296-byte Slice
15 prefix. Exact-old, every partial size 289--295, full-null, full, future-larger/clamped,
uninitialized, unsupported, success-null, and failure-after-write cases behave as specified. The
verified LLVM module has exactly one `xor i32` feeding the store and no `xor i64`. Every invalid
provider call clears its output and leaves the instruction count unchanged. The fake route records
kernel parameters 1 and 2 as the dedicated XOR operands, records the XOR result as the stored
value, and dispatches none of the adjacent callbacks. The exact Slice 14 provider compiles OR,
then gates XOR after one successful discovery and before builder-module creation or libNVVM use.

Direct NVVM and NVRTC both summarize as `[64,32,32]`, exact entry-scoped `xor.b32`, and a global
unsigned 32-bit store. Both artifacts assemble with CUDA 12.9 `ptxas`. On the RTX 5090, both
runtime routes produce `0x66`, `-305419897`, `15`, and `-1` for the four planned vectors.
Binary inspection finds only the V1/V2 getters, `KERNEL32.dll` as the ordinary dependency, delayed
`SHELL32.dll` and `ole32.dll`, and no LLVM DLL dependency.

The helper/special-case inventory contains one coherent ABI support predicate, the direct host and
provider XOR wrappers, distinct fake callback/value-index/module-construction helpers, and one
entry-scoped PTX summary bit. No compiler fallback, custom semantic equivalence, operand-graph
walk, syntax reconstruction, alternate opcode, or producer repair was introduced. The
partial-prefix rejection is owned by append-only ABI negotiation, and the terminal gate is owned
before builder-module/libNVVM creation. The input-shape audit confirms that ordinary lowering
produces the canonical two-operand signed-i32 `kIROp_BitXor`; the first pass owns exact opcode,
arity, and result type, the second pass owns signed-i32 availability/dominance, and canonical
value-map emission owns the provider dispatch. The provider's signless scalar-integer contract is
intentionally broader than the frontend's signed-i32 subset, as for AND and OR, and does not widen
accepted Slang IR.

Final commit: `slice 15`.

## Context and Current Pipeline

The motivating `x ^ y` expression is semantically resolved as `BuiltinOperationKind::BitXor`.
`source/slang/slang-lower-to-ir.cpp` selects `kIROp_BitXor` and emits an ordinary instruction with
the result type and two operands. `IRBuilder::emitBitXor` is the canonical constructor used by
other IR producers. The parameterized source prevents constant folding; the promotion probe must
confirm that linking, optimization, and repeated `simplifyIR` retain one signed-i32
`kIROp_BitXor` feeding `store(destination, result)`.

`source/slang/slang-emit.cpp` links and optimizes the selected CUDA entry point, calls
`validateNVVMSupportedIR`, discovers the optional LLVM 14 provider only after semantic preflight,
and checks the maximum `NVVMIRCapability` before creating a builder module.
`source/slang/slang-emit-nvvm.cpp` validates the finite direct-call closure in dominance order,
declares functions and parameters, maps canonical Slang IR values to provider handles, emits each
body, verifies and serializes once, and hands LLVM bitcode to libNVVM.

Slice 14's `SlangNVVMEmitIntegerBitOr_2` is the append-only predecessor. `_validateI32Value` is the
source of truth for exact signed-i32 constants and available SSA values. The first instruction pass
owns exact opcode, operand count, and result-type admission; the second pass owns operand type,
availability, and dominance; the body-emission switch consumes the canonical value map. Slice 15
adds one operation at each existing boundary. It does not change AST checking/lowering,
reconstruct syntax, repair malformed IR, or introduce another value representation.

Before Slice 15, exact `kIROp_BitXor` reaches the validator's default first-pass diagnostic.
Because the opcode's stable textual spelling is `xor`, the expected stop is E52017
`direct NVVM lowering does not support ... 'xor'`, with no builder load request. The post-Slice-14
probe measured this boundary before implementation.

## Scope and Non-Goals

In scope:

- exact ordinary `kIROp_BitXor` with exactly two signed-i32 operands and a signed-i32 result;
- parameters, exact representable constants, loads, add/subtract/multiply/AND/OR results, phis,
  calls, and other already-supported signed-i32 producers when available and dominant;
- one appended provider operation, terminal capability, stable identity bit, and sanitized host
  wrapper;
- fake topology, verified LLVM, direct/NVRTC differential PTX, both `ptxas` lanes, and runtime
  bit-pattern evidence.

Explicitly out of scope:

- `kIROp_BitNot`, shifts, logical operations, new comparisons, or select changes;
- `kIROp_ConstexprBitXor` or comparison inequality as alternate accepted spellings;
- unsigned, bool, 8/16/64-bit, arbitrary-precision, vector, matrix, or aggregate bitwise values;
- widening the raw kernel/helper ABI or changing ADD/SUB/multiply/AND/OR semantics;
- division, remainder, casts, overflow/saturation variants, bitfields, atomics, reductions, waves,
  resources, pointer masking, local/shared/global declarations, thread builtins, barriers, or
  libdevice;
- performance claims beyond semantic PTX classification and successful assembly/runtime.

## Architecture and Invariants

Capability selection remains monotonic. Add terminal `NVVMIRCapability::ScalarIntegerBitXor`
after Slice 14's `ScalarIntegerBitOr`. An exact Slice 14 provider remains valid and compiles every
previously published program; an XOR program reaches E52016 after provider discovery but before
builder-module creation or libNVVM use.

Append exactly one `SlangNVVMEmitIntegerBitXor_2` pointer to `SlangNVVMBuilderAPI_V2`. Preserve
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE` at 288 bytes on x64 and publish
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE` at 296 bytes. Require:

- `offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitXor)` equals the frozen Slice 14 minimum;
- size 288 initializes successfully with `supportsScalarIntegerBitXor() == false`;
- every size greater than 288 and less than 296 (289 through 295) is rejected as a partial suffix;
- size at least 296 requires a non-null `emitIntegerBitXor` member;
- a future-larger table is accepted, copied only through 296 bytes, and reports local size 296.

The provider identity appends `scalar-integer-bit-xor=0|1`, so shader-cache identity differs when
the capability differs. Exact old prefixes report zero; a coherent full/future prefix reports one.

Slang preflight accepts only exact two-operand `kIROp_BitXor` with signed-i32 result. Both operands
pass `_validateI32Value`; the result joins the existing available-value set and emission map and
may feed any existing signed-i32 consumer. No custom equality, opcode fallback, operand-graph walk,
or syntax reconstruction is permitted.

The host wrapper clears its output before dispatch and passes a private cleared slot. It also
clears after a failed provider call and converts success-without-handle to failure. Unsupported or
uninitialized builders return the established error without exposing a stale handle.

Provider `_emitIntegerBitXor` clears a non-null output first, obtains a live current unterminated
insertion block with `_getValidInsertionBlock`, and validates both handles through
`_areMatchingIntegerValues`. Thus operands must be scalar LLVM integers of exactly equal type,
belong to the same module/context and current function, and be available/dominant at the insertion
point. Only after every check passes may it call `state->builder.CreateXor(left, right)` and publish
the result. Invalid calls add no LLVM instruction.

## Interfaces and Dependencies

Append after Slice 14 in `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitXor_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue);
```

Add:

- table member `emitIntegerBitXor`;
- `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE`;
- host `supportsScalarIntegerBitXor()` and `emitIntegerBitXor(...)`;
- identity component `scalar-integer-bit-xor=0|1`;
- terminal `NVVMIRCapability::ScalarIntegerBitXor` and its pre-module gate.

No public Slang API changes. The implementation retains the optional statically linked LLVM
14.0.6 provider, libNVVM/NVRTC, CUDA `ptxas`, and CUDA-driver environment gates already used by
the focused suite.

## Milestones

1. After committing Slice 14, run the retained parameterized probe through final linking with
   `-dump-ir` and direct NVVM, always with an explicit `-o`. Confirm the exact function signature,
   one two-operand signed-i32 `kIROp_BitXor`, its store consumer, expected E52017 `'xor'`, and zero
   builder discovery in a focused fake test. Compile the same source through explicit NVRTC and
   record `[64,32,32]`, `xor.b32`, and global-store PTX. Promotion requires this stable canonical
   shape. If it folds, changes opcode, or introduces a cast, investigate the producer and revise
   the slice instead of adding a downstream spelling fallback.

2. Freeze the provider suffix in `source/compiler-core/slang-nvvm-ir-builder-api.h`, add strict-C
   minimum-size and capability-order probes in
   `source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c`, and update coherent host negotiation,
   support query, identity, sanitized wrapper, and provider getter. Prove exact Slice 14, every
   partial size 289--295, full-null, full, and future-larger behavior plus uninitialized,
   unsupported, invalid-input, success-null, and failure-after-write output clearing.

3. Implement provider `_emitIntegerBitXor` in
   `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` by reusing `_getValidInsertionBlock` and
   `_areMatchingIntegerValues`, then calling `CreateXor`. Add a verified positive module with
   exactly one `xor i32` feeding the store. Add invalid/no-mutation cases for null module/output,
   no or terminated insertion block, pointer/non-integer operand, mismatched integer width,
   foreign module/context/function, and sibling/non-dominating instruction values.

4. Extend `source/slang/slang-emit-nvvm.{h,cpp}` and the capability gate in
   `source/slang/slang-emit.cpp`. Add exact first-pass admission, terminal capability requirement,
   second-pass `_validateI32Value` checks, available-value registration, and body emission through
   the canonical value map and `builder.emitIntegerBitXor`. Keep preflight and emission switches
   structurally parallel.

5. Extend the fake provider and fixtures in
   `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`. Prove the operation receives kernel
   parameters 1 and 2 in the entry block; its result is the store value; and no old integer-binary,
   multiply, AND, OR, load, branch, call, phi, pointer-offset, or array operation is used. Gate an
   XOR source against an exact Slice 14 provider after discovery/before module creation, while
   proving bitwise OR still works on that provider.

6. Preserve deterministic adjacent boundaries. Keep bitwise NOT at E52017 `'bitnot'`, shifts at
   `'shl'`/`'shr'`, division/remainder at `'div'`/`'irem'`, and pointer helper/local-array fixtures
   at their established semantic boundaries before builder discovery. Keep raw unsigned/wide
   integer XOR at the existing `'entry-point parameter'` boundary. Do not use semantically invalid
   floating-point bitwise fixtures merely to manufacture diagnostics.

7. Compile the parameterized source through direct NVVM and NVRTC. Extend `PTXEntrySummary` with
   exact entry-scoped, token-boundary `xor.b32` classification; compare `[64,32,32]`, XOR, and
   global-store semantics; assemble both outputs; and launch both routes for the reconnaissance
   vectors `0x5a ^ 0x3c == 0x66`, `-1 ^ 0x12345678 == -305419897`,
   `-16 ^ -1 == 15`, and `0 ^ -1 == -1`.

8. Apply pinned formatting, rebuild, run the complete focused and preservation matrices outside
   the sandbox, inspect exports/dependencies, perform the required helper/input-shape audit, update
   durable docs, and commit only tracked Slice 15 files as `slice 15`.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. Every CMake build and test must run outside the
sandbox as required by `AGENTS.md`.

Prototype commands after Slice 14 is committed:

```text
$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvvm -dump-ir `
  -o issue-nvvm-backend\probe.slice15.direct.ptx `
  issue-nvvm-backend\probe.slice15.i32-bit-xor.slang
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvrtc `
  -o issue-nvvm-backend\probe.slice15.nvrtc.ptx `
  issue-nvvm-backend\probe.slice15.i32-bit-xor.slang
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

- `nvvmIRBuilderNegotiatesScalarIntegerBitXorAPI`;
- `nvvmIRBuilderRejectsInvalidIntegerBitXorOperations`;
- `nvvmIRBuilderBuildsIntegerBitXorKernel`;
- `nvvmSlangIntegerBitXorUsesDirectPipeline`;
- `nvvmSlangNegotiatesScalarIntegerBitXorCapability`;
- `nvvmSlangRealIntegerBitXorDifferentialPTX`;
- `nvvmSlangRealIntegerBitXorPtxasAccepts`;
- `nvvmSlangIntegerBitXorRuntimeMatchesNVRTC`.

Slice 14's actual focused baseline was 100/100. These eight independent tests complete the
measured first and post-format Slice 15 focused prefix at 108/108.

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

- measured final linked topology: exact two-operand signed-i32 `kIROp_BitXor` feeding the store;
- measured pre-change E52017 `'xor'` before builder discovery;
- dedicated ABI field with a complete 296-byte Slice 15 x64 prefix and exact frozen 288-byte
  Slice 14 compatibility;
- rejection of every partial size 289--295, full-null/future negotiation, stable identity bit, and
  complete output sanitization;
- provider invalid calls clear outputs and add no LLVM instruction;
- verified LLVM contains exactly one `xor i32` whose result feeds the global store;
- fake topology proves exact parameter operands, result/store flow, and absence of unrelated calls;
- an exact Slice 14 provider compiles prior bitwise-OR work but gates XOR after discovery and
  before module or libNVVM creation;
- signed NOT/shifts/division/remainder and unsigned/wide XOR boundaries remain deterministic;
- direct NVVM/NVRTC expose `[64,32,32]`, `xor.b32`, and global i32 store semantics;
- both PTX outputs assemble and both runtime routes agree for all four bit-pattern vectors;
- the final focused prefix and preservation matrix pass after pinned formatting;
- the provider still exports only V1/V2 getters and has no LLVM DLL dependency;
- every new helper/fallback/special case survives the required principled input-shape audit.

## Failure and Recovery

The probe, incremental builds, focused tests, formatter, and binary inspection are safe to repeat.
If the final expression becomes `kIROp_ConstexprBitXor`, folds, or acquires an unexpected cast,
audit the producer and optimization trace; do not accept multiple shapes downstream merely to make
the fixture pass. Retain a parameterized source so PTX cannot remove the operation as constant.

If LLVM verification fails, fix provider validation or ownership/dominance before serialization.
If a CUDA toolchain spells the optimized operation differently, first prove its entry-scoped truth
table and record the discovery; do not weaken the PTX classifier to arbitrary substring presence.
`ptxas` and runtime equality remain required executable evidence.

If ABI layout differs from 296 bytes, stop and inspect field ordering/alignment rather than changing
the frozen 288-byte predecessor. If a partial or null table initializes, fix negotiation before
enabling emission. If a failed provider call leaves a handle or instruction, fix sanitization and
no-mutation at the provider or wrapper boundary before continuing.

Do not delete/reset the user's worktree or stage `external/slang-binaries/`, any ExecPlan, or probe
artifact. Remove `probe.slice15.i32-bit-xor.slang` and generated PTX/dumps with `apply_patch` before
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
- final `slice 15` commit hash.

Distill stable architecture into `docs/design/nvvm-backend.md`, durable coverage into
`docs/design/nvvm-backend-capability-ledger.md`, and the five-part implementation narrative into
the eventual PR description. Keep this and all probe artifacts untracked.
