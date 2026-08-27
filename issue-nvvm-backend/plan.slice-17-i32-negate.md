# Negate signed i32 values through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the prepared,
uncommitted successor working log for Slice 17 of the direct NVVM backend experiment. Do not begin
tracked implementation until Slice 16 has completed and been committed.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts exact arithmetic negation of one signed `i32`
value.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    *destination = -x;
}
```

The final linked Slang IR must contain one exact one-operand `kIROp_Neg` whose operand and result
are signed `i32`; that result feeds the established device-pointer store. The provider emits LLVM
`IRBuilder::CreateNeg` without `nsw` or `nuw`, represented as `sub i32 0, %x`, only after
validating the current unterminated insertion block and one available same-function scalar integer
operand. Direct NVVM and NVRTC must agree on raw parameter widths `[64, 32]`, expose exact
`neg.s32` and a global 32-bit store, assemble through `ptxas`, and produce identical zero,
positive, negative, and `INT_MIN` wrapping results at runtime.

## Progress

- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the Slice 16 ExecPlan/probe, the canonical Neg
  producer, integer wrapping contract, current LLVM arithmetic mapping, and adjacent
  shift/division/remainder semantics.
- [x] (2026-08-27) Prepared this planning-only hand-off and parameterized Neg probe without
  editing tracked files, building, testing, or running the probe.
- [x] (2026-08-27) After committing Slice 16 as `slice 16`, measure the final
  post-`simplifyIR` Neg topology, direct E52017 boundary, and explicit NVRTC PTX evidence. The
  focused fake gate will record the zero-discovery counter as part of implementation validation.
- [x] (2026-08-27) Append and implement the dedicated integer-negate provider suffix, host
  negotiation/identity/wrapper, provider validation, and strict-C probes.
- [x] (2026-08-27) Extend direct-NVVM preflight, terminal capability gating, and canonical value-map emission
  for exact one-operand signed-i32 `kIROp_Neg`.
- [x] (2026-08-27) Add ABI, provider, fake-topology, capability-gate, negative-boundary, PTX, `ptxas`, and
  runtime tests.
- [x] (2026-08-27) Format, build, run the complete validation matrix outside the sandbox, audit
  the diff, update durable docs, remove the probe, and commit the tracked work as `slice 17`.

## Surprises and Discoveries

- Observation: parameterized source negation has a canonical ordinary unary opcode, while
  compile-time evaluation has a distinct representation.
  Evidence: `source/slang/slang-lower-to-ir.cpp` maps runtime `BuiltinOperationKind::Neg` to
  `kIROp_Neg`; `IRBuilder::emitNeg` constructs that exact ordinary instruction;
  `source/slang/slang-ir-insts.lua` defines one `value` operand and the textual name `neg`; and
  constexpr lowering uses `kIROp_ConstexprNeg` separately.
  Consequence: admit exact ordinary `kIROp_Neg` only. Do not accept the constexpr opcode,
  synthesize subtraction in Slang IR, or reconstruct source syntax as a fallback.

- Observation: signed integer negation has an explicit wrapping source-language contract.
  Evidence: `docs/language-reference/types-fundamental.md` states that all signed and unsigned
  integer arithmetic wraps on overflow. `BuiltinOperationIntVal::tryFoldImpl` performs negation
  through the unsigned representation to avoid host-language overflow, so negating `INT_MIN`
  produces `INT_MIN`.
  Consequence: LLVM must use plain `CreateNeg` without `nsw`/`nuw`. Runtime coverage must include
  `INT_MIN`; an overflow flag or special rejection would contradict the canonical source policy.

- Observation: LLVM's negate builder is ordinary subtraction rather than a distinct LLVM opcode.
  Evidence: LLVM 14 `IRBuilder::CreateNeg(value)` constructs `sub 0, value`; without no-wrap flags,
  LLVM integer subtraction has the required modular bit-pattern behavior.
  Consequence: provider tests must verify one `sub i32 0, %x` feeding the store, not search for a
  nonexistent LLVM `neg` instruction. PTX promotion separately expects `neg.s32`.

- Observation: Slice 16's unary integer validation boundary is the exact provider contract Neg
  needs.
  Evidence: BitNot requires a live unterminated insertion block and one scalar LLVM integer value
  with valid module/context/function ownership and availability/dominance. Neg changes only the
  final LLVM builder call.
  Consequence: reuse the named unary integer predicate. Do not copy validation, pass a dummy second
  operand to a binary helper, or add a new value-equivalence rule.

- Observation: shifts, division, and remainder are not equally mechanical follow-ups.
  Evidence: LLVM makes oversized shifts poison, while Slang's AST fold masks nonnegative counts by
  `IRIntegerValue` width and SCCP uses the result integer width with zero/sign-fill behavior.
  LLVM signed division and remainder are undefined for divisor zero and `INT_MIN / -1`, while
  Slang diagnoses constant zero and its symbolic folder explicitly preserves wrapping
  `INT_MIN / -1` and zero `INT_MIN % -1`.
  Consequence: defer those operations until their source-level exceptional-input policy and safe
  LLVM representation are settled. Neg is independently bounded and has no exceptional operand.

- Observation: the post-Slice-16 promotion baseline matches the canonical Neg hypothesis exactly.
  Evidence: the explicit-output direct probe repeatedly retains
  `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int)`, one
  `let %result : Int = neg(%x)`, and `store(%destination, %result)` through the final
  `simplifyIR`, then stops at E52017 `'neg'`. Explicit NVRTC succeeds with `.param .u64`, one
  `.param .u32`, `neg.s32`, `cvta.to.global.u64`, and `st.global.u32`.
  Consequence: promote exact token-safe `neg.s32` as the differential PTX classifier and accept
  only ordinary one-operand signed-i32 `kIROp_Neg`; no synthesized subtraction or alternate
  downstream shape is needed.

- Observation: the integrated direct route preserves the same ABI and promoted negate form as
  NVRTC.
  Evidence: after the provider and host implementation, both PTX outputs contain raw parameter
  widths `[64,32]`, one exact `neg.s32`, and `st.global.u32`. The direct output stores through the
  raw device address; NVRTC first emits `cvta.to.global.u64`.
  Consequence: plain unflagged LLVM `CreateNeg` reaches the desired target operation without a
  synthesized Slang subtraction or a special overflow path.

## Decision Log

- Decision: Slice 17 is exact signed-i32 arithmetic negation, not left/right shift, signed
  division/remainder, logical NOT, another comparison, or select.
  Rationale: Neg is one stable unary opcode, reuses Slice 16's principled unary validation, and
  maps directly to modular LLVM subtraction. Every shift/division candidate requires a new
  semantic policy before raw LLVM emission is sound.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the post-Slice-16 promotion probe does not retain exact `kIROp_Neg` or explicit
  NVRTC does not provide a stable semantic PTX classification.

- Decision: append `emitIntegerNegate` as a dedicated V2 unary operation.
  Rationale: the ADD/SUB enum and multiply/AND/OR/XOR callables have frozen binary contracts, and
  BitNot has a frozen unary-bitwise contract. One new terminal function pointer negotiates Neg
  support and dispatch atomically without widening any older callable.
  Date/Author: 2026-08-27, Codex.
  Revisit when: an ABI audit finds an existing versioned unary-integer operation whose published
  domain already includes arithmetic negation.

- Decision: preserve Slice 16's 304-byte x64 prefix and publish a 312-byte Slice 17 terminal
  prefix.
  Rationale: the only appended member is one 64-bit function pointer. Sizes 305 through 311 are
  partial and malformed; 304 remains a complete Slice 16 provider; future-larger tables are
  accepted and clamped to the locally known 312 bytes.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the completed Slice 16 build or strict-C offset probes contradict those sizes.

- Decision: use plain LLVM `IRBuilder::CreateNeg` after complete validation.
  Rationale: without `nsw` or `nuw`, `sub i32 0, value` implements the documented wrapping
  two's-complement result for every input, including `INT_MIN`.
  Date/Author: 2026-08-27, Codex.
  Revisit when: verified LLVM or direct/NVRTC runtime evidence exposes a representation mismatch.

- Decision: require an explicit NVRTC promotion result before treating `neg.s32` as the PTX
  classifier contract.
  Rationale: `neg.s32` is the natural PTX instruction, but PTX spelling is optimizer-selected and
  semantic equivalence, not an assumed text form, owns promotion.
  Date/Author: 2026-08-27, Codex.
  Revisit when: NVRTC emits an equivalent stable subtraction form; if so, define the narrowest
  semantic classifier supported by both routes before implementation.

## Outcomes and Retrospective

Slice 17 implements exact ordinary one-operand signed-i32 `kIROp_Neg` through one append-only
unary provider operation. It introduces no new production helper: `_emitIntegerNegate` consumes
Slice 16's `_isIntegerValueUsableAtInsertionPoint`, so scalar integer classification, module,
context, function, insertion-point availability, and dominance remain one shared per-value rule.
That exact canonical LLVM value is the correct input shape; no graph walk, custom equivalence,
syntax reconstruction, dummy operand, constexpr fallback, or synthesized Slang subtraction is
needed. Plain one-argument `CreateNeg` owns construction and carries no `nsw` or `nuw` flag, so
the documented wrapping policy includes `INT_MIN` without a special case.

The ABI retains the exact 304-byte Slice 16 table, rejects every partial size from 305 through 311
bytes and a null complete operation, accepts a complete 312-byte table, and clamps future tables
to that known prefix. Identity reports `scalar-integer-negate=0|1`; uninitialized and unsupported
wrappers return their established errors, and success-without-value or failure-after-write never
exposes a stale handle. Strict-C size/order probes compile. An exact Slice 16 fake provider still
compiles BitNot, then gates Negate after one discovery and before module creation or libNVVM use.

The verified provider module contains exactly one unflagged `sub i32 0, %x` whose result feeds the
address-space-1 store, with no `sub i64`, `nsw`, or `nuw`. Invalid missing/terminated insertion
points, null module/output/value, pointer input, foreign module/context/function values, and
sibling-block non-dominance clear outputs and add no instruction. The fake direct graph is exactly
parameter 1 to the dedicated Negate operation to the store through parameter 0; BitNot and every
unrelated callback remain unused. All predecessor unary and binary validation tests stay green,
and independent review confirmed the multiply, AND, OR, and XOR invalid-test blocks are unchanged
from `HEAD`.

Direct NVVM and NVRTC both expose parameter widths `[64,32]`, exact token-safe `neg.s32`, and a
global u32 store, with no fallback `sub.s32` or `not.b32`. NVRTC performs its normal
`cvta.to.global.u64`; direct NVVM uses the raw device pointer. CUDA 12.9 `ptxas` accepts both
outputs. On the RTX 5090, both runtime routes produce `0`, `-1`, `7`, and `-2147483648` for inputs
`0`, `1`, `-7`, and `INT_MIN`, proving modular wrapping at the exceptional bit pattern. Adjacent
shift, division, remainder, logical-NOT, and raw unsigned/i64/float Neg boundaries remain
deterministic.

The first and post-format focused runs both pass 124/124. The post-format preservation matrix
passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA
compile/pass-through, and 1/1 runtime dispatch. The Release provider and Debug `slang-test`
targets rebuild successfully after pinned clang-format 17.0.6, whose verification reports no
changes. Binary inspection shows only `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`; dependencies are `KERNEL32.dll` plus delayed `SHELL32.dll` and
`ole32.dll`, with no LLVM DLL dependency. Production, test, and whole-diff audits are clean. The
final commit is `slice 17`.

## Context and Current Pipeline

The motivating `-x` expression is semantically resolved as `BuiltinOperationKind::Neg`.
`source/slang/slang-lower-to-ir.cpp` selects `kIROp_Neg` for ordinary runtime lowering and emits
one instruction with the result type and operand. `IRBuilder::emitNeg` is the canonical constructor
used by other IR producers. The parameterized source prevents constant folding; the promotion
probe must confirm that linking, optimization, and repeated `simplifyIR` retain one signed-i32
`kIROp_Neg` feeding `store(destination, result)`.

`source/slang/slang-emit.cpp` links and optimizes the selected CUDA entry point, calls
`validateNVVMSupportedIR`, discovers the optional LLVM 14 provider only after semantic preflight,
and checks the maximum `NVVMIRCapability` before creating a builder module.
`source/slang/slang-emit-nvvm.cpp` validates the finite direct-call closure in dominance order,
declares functions and parameters, maps canonical Slang IR values to provider handles, emits each
body, verifies and serializes once, and hands LLVM bitcode to libNVVM.

Slice 16's `SlangNVVMEmitIntegerBitNot_2` is the append-only predecessor. `_validateI32Value` is
the Slang-side source of truth for exact signed-i32 constants and available SSA values. The first
instruction pass owns exact opcode, operand count, and result-type admission; the second pass owns
operand type, availability, and dominance; the body-emission switch consumes the canonical value
map. Slice 17 adds one unary operation at each existing boundary. It does not change AST
checking/lowering, reconstruct syntax, repair malformed IR, or introduce another value
representation.

At the provider boundary, Slice 16's unary integer predicate should own scalar-integer
classification, module/context/function ownership, same-block ordering, and cross-block
dominance. Neg consumes that exact canonical LLVM value and changes only instruction construction.
The implementation must not add another graph walk, type spelling, or alternate equality rule.

Before Slice 17, exact `kIROp_Neg` should reach the validator's default first-pass diagnostic.
Because the opcode's stable textual spelling is `neg`, the expected stop is E52017
`direct NVVM lowering does not support ... 'neg'`, with no builder load request. This remains a
promotion hypothesis until measured after Slice 16 commits.

## Scope and Non-Goals

In scope:

- exact ordinary `kIROp_Neg` with exactly one signed-i32 operand and a signed-i32 result;
- parameters, exact representable constants, loads, add/subtract/multiply/AND/OR/XOR/BitNot
  results, phis, calls, and other already-supported signed-i32 producers when available and
  dominant;
- documented wrapping behavior for every signed-i32 value, including `INT_MIN`;
- one appended unary provider operation, terminal capability, stable identity bit, and sanitized
  host wrapper;
- fake topology, verified LLVM `sub i32 0`, promotion-gated direct/NVRTC differential PTX, both
  `ptxas` lanes, and runtime wrapping evidence.

Explicitly out of scope:

- `kIROp_ConstexprNeg` or a synthesized Slang-IR subtraction from zero as another accepted shape;
- unsigned, 8/16/64-bit, arbitrary-precision, floating-point, vector, matrix, or aggregate negate;
- `nsw`, `nuw`, saturation, overflow reporting, absolute value, or fused arithmetic;
- left/right shifts, logical NOT/operations, new comparisons, or select changes;
- division, remainder, casts, bitfields, atomics, reductions, waves, resources, pointer masking,
  local/shared/global declarations, thread builtins, barriers, or libdevice;
- widening the raw kernel/helper ABI or changing prior integer-operation semantics;
- performance claims beyond semantic PTX classification and successful assembly/runtime.

## Architecture and Invariants

Capability selection remains monotonic. Add terminal `NVVMIRCapability::ScalarIntegerNegate`
after Slice 16's `ScalarIntegerBitNot`. An exact Slice 16 provider remains valid and compiles every
previously published program; a Neg program reaches E52016 after provider discovery but before
builder-module creation or libNVVM use.

Append exactly one `SlangNVVMEmitIntegerNegate_2` pointer to `SlangNVVMBuilderAPI_V2`. Preserve
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE` at 304 bytes on x64 and publish
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE` at 312 bytes. Require:

- `offsetof(SlangNVVMBuilderAPI_V2, emitIntegerNegate)` equals the frozen Slice 16 minimum;
- size 304 initializes successfully with `supportsScalarIntegerNegate() == false`;
- every size greater than 304 and less than 312 (305 through 311) is rejected as a partial suffix;
- size at least 312 requires a non-null `emitIntegerNegate` member;
- a future-larger table is accepted, copied only through 312 bytes, and reports local size 312.

The provider identity appends `scalar-integer-negate=0|1`, so shader-cache identity differs when
the capability differs. Exact old prefixes report zero; a coherent full/future prefix reports one.

Slang preflight accepts only exact one-operand `kIROp_Neg` with signed-i32 result. The operand
passes `_validateI32Value`; the result joins the existing available-value set and emission map and
may feed any existing signed-i32 consumer. No custom equality, opcode fallback, operand-graph walk,
or syntax reconstruction is permitted.

The host wrapper clears its output before dispatch and passes a private cleared slot. It also
clears after a failed provider call and converts success-without-handle to failure. Unsupported or
uninitialized builders return the established error without exposing a stale handle.

Provider `_emitIntegerNegate` clears a non-null output first, obtains a live current unterminated
insertion block with `_getValidInsertionBlock`, and validates its handle through Slice 16's unary
integer predicate. The operand must be a scalar LLVM integer belonging to the same module/context
and current function and be available/dominant at the insertion point. Only after every check
passes may it call `state->builder.CreateNeg(value)` with no no-wrap flags and publish the result.
Invalid calls add no LLVM instruction.

The valid input shape is an already-canonical LLVM integer value corresponding to a preflighted
signed-i32 Slang value. This provider operation does not repair an alternate IR shape. Its LLVM
result is a `sub i32 0, value`; the distinction between arithmetic signedness policy and LLVM's
signless modular integer construction remains owned by Slang preflight.

## Interfaces and Dependencies

Append after Slice 16 in `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerNegate_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue);
```

Add:

- table member `emitIntegerNegate`;
- `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE`;
- host `supportsScalarIntegerNegate()` and `emitIntegerNegate(...)`;
- identity component `scalar-integer-negate=0|1`;
- terminal `NVVMIRCapability::ScalarIntegerNegate` and its pre-module gate.

No public Slang API changes. The implementation retains the optional statically linked LLVM
14.0.6 provider, libNVVM/NVRTC, CUDA `ptxas`, and CUDA-driver environment gates already used by
the focused suite.

## Milestones

1. After committing Slice 16, add the parameterized Neg source to the unsupported fixture and run
   the retained probe through final linking with `-dump-ir` and direct NVVM, always with an
   explicit `-o`. Confirm the exact function signature, one one-operand signed-i32 `kIROp_Neg`,
   its store consumer, expected E52017 `'neg'`, and zero builder discovery in a focused fake test.
   Compile the same source through explicit NVRTC and record `[64,32]`, the observed exact negate
   instruction, and global-store PTX. Promote `neg.s32` as the classifier only if this evidence is
   stable. If the operation folds, changes opcode, introduces a cast, or uses a materially different
   PTX family, investigate the producer/semantics and revise the slice instead of adding a broad
   fallback.

2. Freeze the provider suffix in `source/compiler-core/slang-nvvm-ir-builder-api.h`, add strict-C
   minimum-size and capability-order probes in
   `source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c`, and update coherent host negotiation,
   support query, identity, sanitized wrapper, and provider getter. Prove exact Slice 16, every
   partial size 305--311, full-null, full, and future-larger behavior plus uninitialized,
   unsupported, invalid-input, success-null, and failure-after-write output clearing.

3. Implement provider `_emitIntegerNegate` in
   `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` by reusing `_getValidInsertionBlock` and the Slice
   16 unary integer predicate, then calling `CreateNeg` without flags. Add a verified positive
   module containing exactly one `sub i32 0, %x` whose result feeds the store. Add
   invalid/no-mutation cases for null module/output/value, no or terminated insertion block,
   pointer/non-integer value, foreign module/context/function, and sibling/non-dominating
   instruction values. Re-run Slice 16's invalid unary tests to prove helper preservation.

4. Extend `source/slang/slang-emit-nvvm.{h,cpp}` and the capability gate in
   `source/slang/slang-emit.cpp`. Add exact first-pass admission, terminal capability requirement,
   second-pass `_validateI32Value` checking, available-value registration, and body emission
   through the canonical value map and `builder.emitIntegerNegate`. Keep preflight and emission
   switches structurally parallel.

5. Extend the fake provider and fixtures in
   `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`. Prove the operation receives kernel
   parameter 1 in the entry block; its result is the store value; and no old integer-binary,
   multiply, AND, OR, XOR, BitNot, load, branch, call, phi, pointer-offset, or array operation is
   used. Gate a Neg source against an exact Slice 16 provider after discovery/before module
   creation, while proving BitNot still works on that provider.

6. Preserve deterministic adjacent boundaries. Keep left/right shifts at E52017 `'shl'`/`'shr'`,
   division/remainder at `'div'`/`'irem'`, and pointer helper/local-array fixtures at their
   established semantic boundaries before builder discovery. Keep raw unsigned/wide and
   floating-point negation at the existing entry-point parameter/type boundary. Keep logical NOT
   separate.

7. Compile the parameterized source through direct NVVM and NVRTC. Once promotion confirms it,
   extend `PTXEntrySummary` with exact entry-scoped, token-boundary `neg.s32` classification;
   compare `[64,32]`, negate, and global-store semantics; assemble both outputs; and launch both
   routes for `-(0) == 0`, `-(1) == -1`, `-(-7) == 7`, and
   `-(-2147483648) == -2147483648` under wrapping signed-i32 arithmetic.

8. Apply pinned formatting, rebuild, run the complete focused and preservation matrices outside
   the sandbox, inspect exports/dependencies, perform the required helper/input-shape audit, update
   durable docs, and commit only tracked Slice 17 files as `slice 17`.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. Every CMake build and test must run outside the
sandbox as required by `AGENTS.md`.

Prototype commands after Slice 16 is committed:

```text
$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvvm -dump-ir `
  -o issue-nvvm-backend\probe.slice17.direct.ptx `
  issue-nvvm-backend\probe.slice17.i32-negate.slang
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvrtc `
  -o issue-nvvm-backend\probe.slice17.nvrtc.ptx `
  issue-nvvm-backend\probe.slice17.i32-negate.slang
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

- `nvvmIRBuilderNegotiatesScalarIntegerNegateAPI`;
- `nvvmIRBuilderRejectsInvalidIntegerNegateOperations`;
- `nvvmIRBuilderBuildsIntegerNegateKernel`;
- `nvvmSlangIntegerNegateUsesDirectPipeline`;
- `nvvmSlangNegotiatesScalarIntegerNegateCapability`;
- `nvvmSlangRealIntegerNegateDifferentialPTX`;
- `nvvmSlangRealIntegerNegatePtxasAccepts`;
- `nvvmSlangIntegerNegateRuntimeMatchesNVRTC`.

If Slice 16 finishes at 116/116 and these remain eight independent tests, the expected Slice 17
prefix is 124/124. Record the actual Slice 16 baseline and final count rather than treating this
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

- measured final linked topology: exact one-operand signed-i32 `kIROp_Neg` feeding the store;
- measured pre-change E52017 `'neg'` before builder discovery;
- explicit NVRTC promotion of `[64,32]`, exact token-safe `neg.s32`, and global u32 store;
- dedicated ABI field with a complete 312-byte Slice 17 x64 prefix and exact frozen 304-byte
  Slice 16 compatibility;
- rejection of every partial size 305--311, full-null/future negotiation, stable identity bit, and
  complete output sanitization;
- provider invalid calls clear outputs and add no LLVM instruction;
- verified LLVM contains exactly one unflagged `sub i32 0, %x` whose result feeds the global store;
- fake topology proves the exact parameter operand, result/store flow, and absence of unrelated
  calls;
- an exact Slice 16 provider compiles prior BitNot work but gates Neg after discovery and before
  module or libNVVM creation;
- shifts/division/remainder and unsigned/wide/floating Neg boundaries remain deterministic;
- direct NVVM/NVRTC expose `[64,32]`, promoted `neg.s32`, and global u32 store semantics;
- both PTX outputs assemble and both runtime routes agree for all four values, including `INT_MIN`;
- the final focused prefix and preservation matrix pass after pinned formatting;
- the provider still exports only V1/V2 getters and has no LLVM DLL dependency;
- every new helper/fallback/special case survives the required principled input-shape audit.

The following policy questions explicitly remain deferred rather than being answered by Slice 17:

- Do negative or oversized shift counts use modulo, zero/sign-fill clamping, a warning, an error,
  or an invalid-program rule?
- Which one shift rule must be made consistent across AST folding, SCCP, NVRTC, and direct LLVM?
- Does a runtime integer divisor of zero have defined behavior, trap/diagnose, or remain invalid?
- How must `INT_MIN / -1` and `INT_MIN % -1` be represented safely in LLVM while preserving
  Slang's documented wrapping and constant-folding behavior?
- Does the shift RHS remain exact signed `i32` independently of the LHS type, as the language's
  `int amount` signature suggests?
- Are signed division and remainder separate negotiated capabilities even if a target can lower
  them through one hardware operation?

## Failure and Recovery

The probe, incremental builds, focused tests, formatter, and binary inspection are safe to repeat.
If the final expression becomes `kIROp_ConstexprNeg`, folds, or acquires an unexpected cast, audit
the producer and optimization trace; do not accept multiple shapes downstream merely to make the
fixture pass. Retain a parameterized source so PTX cannot remove the operation as constant.

If LLVM verification fails, fix provider validation or ownership/dominance before serialization.
Remember that `CreateNeg(i32)` is expected to print as unflagged `sub i32 0, value`; do not diagnose
that representation as a missing operation. If NVRTC does not emit `neg.s32`, stop at the promotion
gate and compare the exact semantic PTX form before designing the classifier. Do not weaken it to
an arbitrary substring. `ptxas`, runtime equality, and the `INT_MIN` wrapping case remain required.

If any generated LLVM negate gains `nsw` or `nuw`, remove the flag before proceeding: it makes the
documented `INT_MIN` input poison rather than wrapping. If Slice 16's unary helper accepts or
rejects an unexpected value shape, audit the canonical producer and helper ownership instead of
adding a Neg-only fallback.

If ABI layout differs from 312 bytes, stop and inspect field ordering/alignment rather than
changing the frozen 304-byte predecessor. If a partial or null table initializes, fix negotiation
before enabling emission. If a failed provider call leaves a handle or instruction, fix
sanitization and no-mutation at the provider or wrapper boundary before continuing.

Do not delete/reset the user's worktree or stage `external/slang-binaries/`, any ExecPlan, or probe
artifact. Remove `probe.slice17.i32-negate.slang` and generated PTX/dumps with `apply_patch` before
committing. The direct route remains experimental and removable without changing default NVRTC
dispatch.

## Artifacts and Hand-Off

Retain in this plan:

- exact linked IR and measured baseline diagnostic/discovery counters;
- explicit NVRTC promotion evidence for the PTX classifier;
- compile-time/runtime prefix sizes and old/partial/null/full/future negotiation matrix;
- verified LLVM unflagged-sub assembly and invalid/no-mutation matrix;
- unary-helper audit and Slice 16 preservation evidence;
- fake producer-to-consumer topology and capability-gate counters;
- direct/NVRTC PTX summaries, both `ptxas` results, and runtime values including `INT_MIN`;
- final focused/preservation counts, exports/dependencies, formatter evidence, and helper/input-
  shape audit;
- final `slice 17` commit hash.

Distill stable architecture into `docs/design/nvvm-backend.md`, durable coverage into
`docs/design/nvvm-backend-capability-ledger.md`, and the five-part implementation narrative into
the eventual PR description. Keep this and all probe artifacts untracked.
