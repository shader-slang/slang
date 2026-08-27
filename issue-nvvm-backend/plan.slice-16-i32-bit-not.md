# Complement signed i32 values through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the prepared,
uncommitted successor working log for Slice 16 of the direct NVVM backend experiment. Do not begin
tracked implementation until Slice 15 has completed and been committed.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts the next exact signed-integer bitwise expression:
unary bitwise complement of a signed `i32` value.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    *destination = ~x;
}
```

The final linked Slang IR must contain one exact one-operand `kIROp_BitNot` whose operand and result
are signed `i32`; that result feeds the established device-pointer store. The provider emits LLVM
`CreateNot`, represented as `xor i32` with an all-ones operand, only after validating the current
unterminated insertion block and one available same-function scalar integer operand. Direct NVVM
and NVRTC must agree on raw parameter widths `[64, 32]`, expose exact `not.b32` and a global 32-bit
store, assemble through `ptxas`, and produce identical all-zero, all-one, alternating-bit, and
negative-value results at runtime.

## Progress

- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the Slice 15 ExecPlan/probe, and the current provider
  integer-validation, fake-provider, PTX-summary, `ptxas`, and runtime-test structure.
- [x] (2026-08-27) Prepared this planning-only hand-off and parameterized BitNot probe without
  editing tracked files, building, testing, or running the probe.
- [x] (2026-08-27) After committing Slice 15 as `slice 15`, measured the final
  post-`simplifyIR` BitNot topology and direct E52017 boundary, then generated explicit NVRTC PTX.
- [x] (2026-08-27) Append and implement the dedicated integer-bit-NOT provider suffix, host
  negotiation/identity/wrapper, provider validation, and strict-C probes.
- [x] (2026-08-27) Refactor the provider's integer validation into a principled shared unary predicate while
  preserving every established binary-operation contract.
- [x] (2026-08-27) Extend direct-NVVM preflight, terminal capability gating, and canonical value-map emission
  for exact one-operand signed-i32 `kIROp_BitNot`.
- [x] (2026-08-27) Add ABI, provider, fake-topology, capability-gate, negative-boundary, PTX, `ptxas`, and
  runtime tests.
- [x] (2026-08-27) Format, build, run the complete validation matrix outside the sandbox, audit
  the diff, update durable docs, remove the probe, and commit the tracked work as `slice 16`.

## Surprises and Discoveries

- Observation: parameterized source BitNot has a canonical ordinary unary opcode, while
  compile-time evaluation has a distinct representation.
  Evidence: `source/slang/slang-lower-to-ir.cpp` maps runtime
  `BuiltinOperationKind::BitNot` to `kIROp_BitNot`; `IRBuilder::emitBitNot` constructs that exact
  ordinary instruction; `source/slang/slang-ir-insts.lua` defines one `value` operand and the
  textual name `bitnot`; and constexpr lowering uses `kIROp_ConstexprBitNot` separately.
  Consequence: admit exact ordinary `kIROp_BitNot` only. Do not accept the constexpr opcode,
  synthesize XOR in Slang IR, or reconstruct source syntax as a fallback.

- Observation: LLVM's unary NOT builder is an all-ones XOR rather than a distinct LLVM opcode.
  Evidence: LLVM 14 `IRBuilder::CreateNot(value)` creates an XOR with an all-ones constant of the
  operand type. For `i32`, verified textual LLVM should therefore contain one `xor i32` with `-1`
  whose result feeds the store, while libNVVM/NVRTC should expose PTX `not.b32`.
  Consequence: provider tests must verify the semantic LLVM shape, not search for a nonexistent
  LLVM `not` instruction. PTX tests still require exact token-boundary `not.b32`.

- Observation: the existing `_areMatchingIntegerValues` combines a unary value-validity rule with
  the binary equal-type rule.
  Evidence: it calls `_isValueUsableAtInsertionPoint` for both values, compares their types, then
  requires `llvm::IntegerType`. BitNot needs exactly the per-value ownership, availability, and
  scalar-integer part, but no second operand or type comparison.
  Consequence: extract a named unary integer predicate and implement the binary helper in terms of
  it plus exact type equality. This keeps one source of truth and avoids dummy operands or copied
  validation. Re-run all prior integer-operation tests to prove the refactor is behavior-neutral.

- Observation: remaining adjacent operations have stronger semantic traps than BitNot.
  Evidence: shifts must define out-of-range/negative-count behavior relative to LLVM poison;
  signed division/remainder have zero and `INT_MIN / -1` exceptional cases; pointer helper ABI and
  local arrays cross type/address-space boundaries.
  Consequence: Slice 16 contains BitNot alone and retains shifts, division, remainder, pointer
  helper ABI, and local arrays as deterministic negative coverage.

- Observation: the post-Slice-15 baseline matches the canonical promotion hypothesis exactly.
  Evidence: the explicit-output direct probe repeatedly retains
  `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int)`, one
  `let %result : Int = bitnot(%x)`, and `store(%destination, %result)` through the final
  `simplifyIR`; it then stops at E52017 `'bitnot'`. Explicit NVRTC succeeds with `.param .u64`,
  one `.param .u32`, `not.b32`, `cvta.to.global.u64`, and `st.global.u32`.
  Consequence: promote only exact ordinary signed-i32 `kIROp_BitNot`; no synthesized XOR or other
  downstream representation fallback is needed.

- Observation: the implemented direct and NVRTC routes preserve the same public scalar ABI and
  exact target instruction.
  Evidence: both generated PTX artifacts have raw parameter widths `[64,32]`, one exact
  `not.b32`, and `st.global.u32`; the direct route stores through the raw device address while
  NVRTC first emits `cvta.to.global.u64`.
  Consequence: the dedicated unary provider operation reaches the expected CUDA representation
  without an ABI cast or a synthesized binary operation.

- Observation: the shared unary integer predicate is behavior-neutral for established binary
  operations.
  Evidence: the first focused run passed 116/116, including the unchanged multiply, AND, OR, and
  XOR invalid-operation matrices as well as the new unary ownership, context, function,
  availability, and no-mutation matrix. Independent review confirmed those four predecessor test
  blocks remain byte-for-byte identical to `HEAD`.
  Consequence: keep `_isIntegerValueUsableAtInsertionPoint` as the single per-value rule and keep
  exact type equality only in `_areMatchingIntegerValues`.

## Decision Log

- Decision: Slice 16 is exact signed-i32 bitwise NOT, not shifts, division/remainder, pointer
  helper ABI, or local arrays.
  Rationale: BitNot is one stable unary opcode with exact bit-pattern semantics and reuses the
  established scalar ABI, available-value map, store, PTX, and runtime machinery. Its only new
  structural issue is a bounded, principled unary validation boundary.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the post-Slice-15 promotion probe does not retain exact `kIROp_BitNot`.

- Decision: append `emitIntegerBitNot` as a dedicated V2 unary operation.
  Rationale: the ADD/SUB enum and the multiply/AND/OR/XOR callables have frozen binary contracts.
  A unary operation cannot truthfully reuse them, and one terminal function pointer negotiates
  support and dispatch atomically.
  Date/Author: 2026-08-27, Codex.
  Revisit when: an ABI audit finds an existing versioned unary-integer operation whose published
  domain already includes BitNot.

- Decision: preserve Slice 15's 296-byte x64 prefix and publish a 304-byte Slice 16 terminal
  prefix.
  Rationale: the only appended member is one 64-bit function pointer. Sizes 297 through 303 are
  partial and malformed; 296 remains a complete Slice 15 provider; future-larger tables are
  accepted and clamped to the locally known 304 bytes.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the completed Slice 15 build or strict-C offset probes contradict those sizes.

- Decision: extract one unary integer-validity helper and keep `_areMatchingIntegerValues` as the
  binary composition of that predicate and exact type equality.
  Rationale: provider ownership, availability, and scalar-integer classification are properties of
  each operand. Centralizing those properties avoids duplicate policy and preserves a simple,
  reviewable binary equality check.
  Date/Author: 2026-08-27, Codex.
  Revisit when: LLVM value kinds reveal a valid integer input whose availability contract differs
  between unary and binary operations.

- Decision: use LLVM `IRBuilder::CreateNot` after complete validation.
  Rationale: exact signed-i32 Slang complement and LLVM's signless all-ones XOR have identical
  bit-pattern semantics and no exceptional operand values.
  Date/Author: 2026-08-27, Codex.
  Revisit when: verified LLVM or direct/NVRTC runtime evidence exposes a representation mismatch.

## Outcomes and Retrospective

Slice 16 implements exact ordinary one-operand signed-i32 `kIROp_BitNot` through a dedicated
append-only unary provider operation. The provider's one new helper,
`_isIntegerValueUsableAtInsertionPoint`, combines the existing canonical LLVM-value ownership,
context, function, insertion-point availability, and dominance checks with scalar
`llvm::IntegerType` classification. It survives the helper audit because these are intrinsic
properties of one already-lowered provider value. `_areMatchingIntegerValues` now composes two
calls to that rule plus exact LLVM type identity, preserving one source of truth rather than
creating a new equivalence or walking an operand graph. No fallback, syntax reconstruction,
synthesized Slang XOR, or target-side repair was added.

The provider ABI retains the exact 296-byte Slice 15 table, rejects every partial size from 297
through 303 bytes and a null complete operation, accepts a full 304-byte table, and clamps future
tables to that known prefix. Identity reports `scalar-integer-bit-not=0|1`; the wrapper returns
`SLANG_E_UNINITIALIZED` before initialization and clears success-without-value and
failure-after-write outputs. Strict-C order/size probes compile. The exact-Slice-15 fake provider
still compiles XOR, then rejects BitNot after one discovery and before module creation or libNVVM
use.

The verified provider module contains exactly one `xor i32` with the all-ones operand `-1`; its
result feeds the address-space-1 store, and there is no `xor i64`. The invalid-operation matrix
rejects missing or terminated insertion points, null module/output/value, pointer input, foreign
module/context/function inputs, and sibling-block non-dominance before mutation. The fake direct
graph is exactly parameter 1 to the dedicated BitNot operation to the store through parameter 0,
with XOR and all unrelated callbacks unused. All established binary invalid-operation tests stay
green; independent review also confirmed the multiply, AND, OR, and XOR blocks are byte-for-byte
unchanged from the predecessor.

Direct NVVM and NVRTC both expose parameter widths `[64,32]`, exact token-safe `not.b32`, and a
global u32 store. NVRTC performs its normal `cvta.to.global.u64`; direct NVVM uses the raw device
pointer. CUDA 12.9 `ptxas` accepts both outputs. On the RTX 5090, both runtime routes produce
`-1`, `0`, `-1431655766`, and `15` for `~0`, `~-1`, `~0x55555555`, and `~-16` respectively.

The first and post-format focused runs both pass 116/116. The post-format preservation matrix
passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA
compile/pass-through, and 1/1 runtime dispatch. The Release provider and Debug `slang-test`
targets rebuild successfully after pinned clang-format 17.0.6, whose verification reports no
changes. Binary inspection shows only `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`; dependencies are `KERNEL32.dll` plus delayed `SHELL32.dll` and
`ole32.dll`, with no LLVM DLL dependency. Production, test, and whole-diff audits are clean. The
final commit is `slice 16`.

## Context and Current Pipeline

The motivating `~x` expression is semantically resolved as `BuiltinOperationKind::BitNot`.
`source/slang/slang-lower-to-ir.cpp` selects `kIROp_BitNot` for ordinary runtime lowering and emits
one instruction with the result type and operand. `IRBuilder::emitBitNot` is the canonical
constructor used by other IR producers. The parameterized source prevents constant folding; the
promotion probe must confirm that linking, optimization, and repeated `simplifyIR` retain one
signed-i32 `kIROp_BitNot` feeding `store(destination, result)`.

`source/slang/slang-emit.cpp` links and optimizes the selected CUDA entry point, calls
`validateNVVMSupportedIR`, discovers the optional LLVM 14 provider only after semantic preflight,
and checks the maximum `NVVMIRCapability` before creating a builder module.
`source/slang/slang-emit-nvvm.cpp` validates the finite direct-call closure in dominance order,
declares functions and parameters, maps canonical Slang IR values to provider handles, emits each
body, verifies and serializes once, and hands LLVM bitcode to libNVVM.

Slice 15's `SlangNVVMEmitIntegerBitXor_2` is the append-only predecessor. `_validateI32Value` is the
Slang-side source of truth for exact signed-i32 constants and available SSA values. The first
instruction pass owns exact opcode, operand count, and result-type admission; the second pass owns
operand type, availability, and dominance; the body-emission switch consumes the canonical value
map. Slice 16 adds one unary operation at each existing boundary. It does not change AST
checking/lowering, reconstruct syntax, repair malformed IR, or introduce another value
representation.

At the provider boundary, `_isValueUsableAtInsertionPoint` already owns module/context/function
ownership plus same-block ordering and cross-block dominance. `_areMatchingIntegerValues` adds
integer classification and binary type equality. The implementation should extract only the
per-value integer predicate needed by both unary and binary consumers; it must not add a new graph
walk or alternate equivalence relation.

Before Slice 16, exact `kIROp_BitNot` should reach the validator's default first-pass diagnostic.
Because the opcode's stable textual spelling is `bitnot`, the expected stop is E52017
`direct NVVM lowering does not support ... 'bitnot'`, with no builder load request. This remains a
promotion hypothesis until measured after Slice 15 commits.

## Scope and Non-Goals

In scope:

- exact ordinary `kIROp_BitNot` with exactly one signed-i32 operand and a signed-i32 result;
- parameters, exact representable constants, loads, add/subtract/multiply/AND/OR/XOR results,
  phis, calls, and other already-supported signed-i32 producers when available and dominant;
- one appended unary provider operation, terminal capability, stable identity bit, and sanitized
  host wrapper;
- a shared provider unary-integer validation predicate used by the established binary helper;
- fake topology, verified LLVM all-ones XOR representation, direct/NVRTC differential PTX, both
  `ptxas` lanes, and runtime bit-pattern evidence.

Explicitly out of scope:

- shifts, logical NOT, logical operations, new comparisons, or select changes;
- `kIROp_ConstexprBitNot`, a synthesized Slang-IR XOR-with-minus-one, or another accepted spelling;
- unsigned, bool, 8/16/64-bit, arbitrary-precision, vector, matrix, or aggregate bitwise values;
- widening the raw kernel/helper ABI or changing ADD/SUB/multiply/AND/OR/XOR semantics;
- division, remainder, casts, overflow/saturation variants, bitfields, atomics, reductions, waves,
  resources, pointer masking, local/shared/global declarations, thread builtins, barriers, or
  libdevice;
- performance claims beyond semantic PTX classification and successful assembly/runtime.

## Architecture and Invariants

Capability selection remains monotonic. Add terminal `NVVMIRCapability::ScalarIntegerBitNot`
after Slice 15's `ScalarIntegerBitXor`. An exact Slice 15 provider remains valid and compiles every
previously published program; a BitNot program reaches E52016 after provider discovery but before
builder-module creation or libNVVM use.

Append exactly one `SlangNVVMEmitIntegerBitNot_2` pointer to `SlangNVVMBuilderAPI_V2`. Preserve
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE` at 296 bytes on x64 and publish
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE` at 304 bytes. Require:

- `offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitNot)` equals the frozen Slice 15 minimum;
- size 296 initializes successfully with `supportsScalarIntegerBitNot() == false`;
- every size greater than 296 and less than 304 (297 through 303) is rejected as a partial suffix;
- size at least 304 requires a non-null `emitIntegerBitNot` member;
- a future-larger table is accepted, copied only through 304 bytes, and reports local size 304.

The provider identity appends `scalar-integer-bit-not=0|1`, so shader-cache identity differs when
the capability differs. Exact old prefixes report zero; a coherent full/future prefix reports one.

Slang preflight accepts only exact one-operand `kIROp_BitNot` with signed-i32 result. The operand
passes `_validateI32Value`; the result joins the existing available-value set and emission map and
may feed any existing signed-i32 consumer. No custom equality, opcode fallback, operand-graph walk,
or syntax reconstruction is permitted.

The host wrapper clears its output before dispatch and passes a private cleared slot. It also
clears after a failed provider call and converts success-without-handle to failure. Unsupported or
uninitialized builders return the established error without exposing a stale handle.

Provider `_emitIntegerBitNot` clears a non-null output first, obtains a live current unterminated
insertion block with `_getValidInsertionBlock`, and validates its handle through the extracted
unary integer predicate. The operand must be a scalar LLVM integer belonging to the same
module/context and current function and be available/dominant at the insertion point. Only after
every check passes may it call `state->builder.CreateNot(value)` and publish the result. Invalid
calls add no LLVM instruction.

The helper refactor must preserve `_areMatchingIntegerValues` behavior exactly: both binary
operands independently satisfy the unary predicate and their LLVM type pointers are identical.
The new helper classifies an already-canonical LLVM value; it must not compensate for malformed
Slang IR or rediscover ownership by walking arbitrary operands.

## Interfaces and Dependencies

Append after Slice 15 in `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitNot_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue);
```

Add:

- table member `emitIntegerBitNot`;
- `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE`;
- host `supportsScalarIntegerBitNot()` and `emitIntegerBitNot(...)`;
- identity component `scalar-integer-bit-not=0|1`;
- terminal `NVVMIRCapability::ScalarIntegerBitNot` and its pre-module gate.

No public Slang API changes. The implementation retains the optional statically linked LLVM
14.0.6 provider, libNVVM/NVRTC, CUDA `ptxas`, and CUDA-driver environment gates already used by
the focused suite.

## Milestones

1. After committing Slice 15, run the retained parameterized probe through final linking with
   `-dump-ir` and direct NVVM, always with an explicit `-o`. Confirm the exact function signature,
   one one-operand signed-i32 `kIROp_BitNot`, its store consumer, expected E52017 `'bitnot'`, and
   zero builder discovery in a focused fake test. Compile the same source through explicit NVRTC
   and record `[64,32]`, exact `not.b32`, and global-store PTX. Promotion requires this stable
   canonical shape. If it folds, changes opcode, or introduces a cast, investigate the producer
   and revise the slice instead of adding a downstream fallback.

2. Freeze the provider suffix in `source/compiler-core/slang-nvvm-ir-builder-api.h`, add strict-C
   minimum-size and capability-order probes in
   `source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c`, and update coherent host negotiation,
   support query, identity, sanitized wrapper, and provider getter. Prove exact Slice 15, every
   partial size 297--303, full-null, full, and future-larger behavior plus uninitialized,
   unsupported, invalid-input, success-null, and failure-after-write output clearing.

3. Refactor `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` so one named helper validates a single
   integer value at the insertion point and `_areMatchingIntegerValues` composes two such checks
   with exact type equality. Implement provider `_emitIntegerBitNot` through the unary helper and
   `CreateNot`. Add a verified positive module containing exactly one `xor i32` with an all-ones
   operand whose result feeds the store. Add invalid/no-mutation cases for null module/output/value,
   no or terminated insertion block, pointer/non-integer value, foreign module/context/function,
   and sibling/non-dominating instruction values. Re-run prior binary invalid tests as the helper
   refactor's preservation evidence.

4. Extend `source/slang/slang-emit-nvvm.{h,cpp}` and the capability gate in
   `source/slang/slang-emit.cpp`. Add exact first-pass admission, terminal capability requirement,
   second-pass `_validateI32Value` checking, available-value registration, and body emission
   through the canonical value map and `builder.emitIntegerBitNot`. Keep preflight and emission
   switches structurally parallel.

5. Extend the fake provider and fixtures in
   `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`. Prove the operation receives kernel
   parameter 1 in the entry block; its result is the store value; and no old integer-binary,
   multiply, AND, OR, XOR, load, branch, call, phi, pointer-offset, or array operation is used.
   Gate a BitNot source against an exact Slice 15 provider after discovery/before module creation,
   while proving bitwise XOR still works on that provider.

6. Preserve deterministic adjacent boundaries. Keep shifts at E52017 `'shl'`/`'shr'`,
   division/remainder at `'div'`/`'irem'`, and pointer helper/local-array fixtures at their
   established semantic boundaries before builder discovery. Keep raw unsigned/wide integer
   BitNot at the existing `'entry-point parameter'` boundary. Keep logical NOT separate and do not
   use semantically invalid floating-point bitwise fixtures merely to manufacture diagnostics.

7. Compile the parameterized source through direct NVVM and NVRTC. Extend `PTXEntrySummary` with
   exact entry-scoped, token-boundary `not.b32` classification; compare `[64,32]`, complement, and
   global-store semantics; assemble both outputs; and launch both routes for `~0 == -1`,
   `~-1 == 0`, `~0x55555555 == -1431655766`, and `~-16 == 15`.

8. Apply pinned formatting, rebuild, run the complete focused and preservation matrices outside
   the sandbox, inspect exports/dependencies, perform the required helper/input-shape audit, update
   durable docs, and commit only tracked Slice 16 files as `slice 16`.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. Every CMake build and test must run outside the
sandbox as required by `AGENTS.md`.

Prototype commands after Slice 15 is committed:

```text
$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvvm -dump-ir `
  -o issue-nvvm-backend\probe.slice16.direct.ptx `
  issue-nvvm-backend\probe.slice16.i32-bit-not.slang
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvrtc `
  -o issue-nvvm-backend\probe.slice16.nvrtc.ptx `
  issue-nvvm-backend\probe.slice16.i32-bit-not.slang
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

- `nvvmIRBuilderNegotiatesScalarIntegerBitNotAPI`;
- `nvvmIRBuilderRejectsInvalidIntegerBitNotOperations`;
- `nvvmIRBuilderBuildsIntegerBitNotKernel`;
- `nvvmSlangIntegerBitNotUsesDirectPipeline`;
- `nvvmSlangNegotiatesScalarIntegerBitNotCapability`;
- `nvvmSlangRealIntegerBitNotDifferentialPTX`;
- `nvvmSlangRealIntegerBitNotPtxasAccepts`;
- `nvvmSlangIntegerBitNotRuntimeMatchesNVRTC`.

If Slice 15 finishes at 108/108 and these remain eight independent tests, the expected Slice 16
prefix is 116/116. Record the actual Slice 15 baseline and final count rather than treating this
estimate as acceptance evidence.

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

- measured final linked topology: exact one-operand signed-i32 `kIROp_BitNot` feeding the store;
- measured pre-change E52017 `'bitnot'` before builder discovery;
- dedicated ABI field with a complete 304-byte Slice 16 x64 prefix and exact frozen 296-byte
  Slice 15 compatibility;
- rejection of every partial size 297--303, full-null/future negotiation, stable identity bit, and
  complete output sanitization;
- the unary integer helper owns only canonical LLVM type/ownership/availability validation, and
  established binary integer validation remains behaviorally unchanged;
- provider invalid calls clear outputs and add no LLVM instruction;
- verified LLVM contains exactly one `xor i32` with an all-ones operand whose result feeds the
  global store;
- fake topology proves the exact parameter operand, result/store flow, and absence of unrelated
  calls;
- an exact Slice 15 provider compiles prior bitwise-XOR work but gates BitNot after discovery and
  before module or libNVVM creation;
- shifts/division/remainder and unsigned/wide BitNot boundaries remain deterministic;
- direct NVVM/NVRTC expose `[64,32]`, exact `not.b32`, and global i32 store semantics;
- both PTX outputs assemble and both runtime routes agree for all four bit-pattern vectors;
- the final focused prefix and preservation matrix pass after pinned formatting;
- the provider still exports only V1/V2 getters and has no LLVM DLL dependency;
- every new helper/fallback/special case survives the required principled input-shape audit.

## Failure and Recovery

The probe, incremental builds, focused tests, formatter, and binary inspection are safe to repeat.
If the final expression becomes `kIROp_ConstexprBitNot`, folds, or acquires an unexpected cast,
audit the producer and optimization trace; do not accept multiple shapes downstream merely to make
the fixture pass. Retain a parameterized source so PTX cannot remove the operation as constant.

If LLVM verification fails, fix provider validation or ownership/dominance before serialization.
Remember that `CreateNot(i32)` is expected to print as all-ones `xor i32`; do not diagnose that
representation as a missing operation. If a CUDA toolchain spells optimized PTX differently, first
prove its entry-scoped truth table and record the discovery; do not weaken the PTX classifier to an
arbitrary substring. `ptxas` and runtime equality remain required executable evidence.

If the unary helper changes any established binary acceptance/rejection result, revert the
refactor and audit the exact input shape before proceeding. Do not keep duplicated validation or
pass the same value twice to the binary helper as a shortcut: both would obscure the unary
contract. The valid shape is an already-canonical provider value at the current insertion point;
malformed earlier IR belongs at Slang preflight.

If ABI layout differs from 304 bytes, stop and inspect field ordering/alignment rather than
changing the frozen 296-byte predecessor. If a partial or null table initializes, fix negotiation
before enabling emission. If a failed provider call leaves a handle or instruction, fix
sanitization and no-mutation at the provider or wrapper boundary before continuing.

Do not delete/reset the user's worktree or stage `external/slang-binaries/`, any ExecPlan, or probe
artifact. Remove `probe.slice16.i32-bit-not.slang` and generated PTX/dumps with `apply_patch` before
committing. The direct route remains experimental and removable without changing default NVRTC
dispatch.

## Artifacts and Hand-Off

Retain in this plan:

- exact linked IR and measured baseline diagnostic/discovery counters;
- compile-time/runtime prefix sizes and old/partial/null/full/future negotiation matrix;
- verified LLVM all-ones-XOR assembly and invalid/no-mutation matrix;
- unary-helper audit and prior binary-operation preservation evidence;
- fake producer-to-consumer topology and capability-gate counters;
- direct/NVRTC PTX summaries, both `ptxas` results, and runtime values;
- final focused/preservation counts, exports/dependencies, formatter evidence, and helper/input-
  shape audit;
- final `slice 16` commit hash.

Distill stable architecture into `docs/design/nvvm-backend.md`, durable coverage into
`docs/design/nvvm-backend-capability-ledger.md`, and the five-part implementation narrative into
the eventual PR description. Keep this and all probe artifacts untracked.
