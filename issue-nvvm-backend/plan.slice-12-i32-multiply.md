# Multiply signed i32 values through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 12 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts the smallest remaining signed-integer expression
that already has a stable canonical producer: multiplication of two signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x * y;
}
```

The final linked Slang IR must contain one exact `IRMul`/`kIROp_Mul` with the two kernel parameters
as operands and signed `i32` as both operand and result type. Its result must feed the established
device-pointer store. The provider emits LLVM `mul i32` only after validating the current
unterminated insertion block and both available same-function integer operands. Direct NVVM and
NVRTC must agree on raw parameter widths `[64, 32, 32]`, expose 32-bit integer multiplication plus
the global store, assemble through `ptxas`, and produce matching positive, negative, and zero
products at runtime.

## Progress

- [x] (2026-08-27) Completed Slice 11 as `slice 11` with its final 76/76 NVVM
  prefix, preservation matrix, and binary inspection green.
- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the Slice 11 hand-off, current scalar ABI/provider
  boundary, and the retained E52017 multiplication fixture.
- [x] (2026-08-27) Froze the exact post-`simplifyIR` producer/consumer graph, reproduced direct
  E52017 `'mul'`, and proved the same source compiles through explicit NVRTC.
- [x] (2026-08-27) Appended and implemented the coherent dedicated integer-multiply provider
  suffix, host negotiation/identity/wrapper, provider validation, and strict-C probes; the Release
  provider build is green.
- [x] (2026-08-27) Extended direct-NVVM preflight, terminal capability gating, and canonical
  value-map body emission for exact two-operand signed-i32 `kIROp_Mul`; the Debug `slangc` build
  and first end-to-end direct compile are green.
- [x] (2026-08-27) Added ABI, provider, fake-topology, capability-gate, PTX, `ptxas`, runtime,
  and negative tests; every new multiplication test passed on its first execution.
- [x] (2026-08-27) Corrected the former Slice 11 terminal-size assertion to freeze its prefix
  boundary, applied pinned clang-format 17, rebuilt both binaries, passed the final 84/84 focused
  prefix and the complete preservation matrix, audited the diff, updated durable docs, and
  inspected the provider binary surface.
- [x] (2026-08-27) Committed the tracked work as `slice 12`.

## Surprises and Discoveries

- Observation: multiplication is already a deterministic unsupported boundary.
  Evidence: `kDirectNVVMUnsupportedMultiplySource` in
  `tools/slang-unit-test/unit-test-nvvm-compiler.cpp` reaches E52017 `'mul'` before builder
  discovery, while add/subtract are part of the frozen Slice 7 integer-binary contract.
  Consequence: promote this exact producer rather than inventing a general arithmetic framework.

- Observation: the motivating source remains unchanged across the final repeated `simplifyIR`
  dumps.
  Evidence: its entry signature is exactly `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout),
  Int, Int)`. One `let %result : Int = mul(%x, %y)` consumes the two signed-i32 parameters, and
  `store(%destination, %result)` is its only value consumer before the void return. Direct NVVM
  reports E52017 `'mul'`; explicit NVRTC succeeds for the same PTX target and `cuda_sm_7_0`.
  Consequence: validate and lower the canonical `kIROp_Mul` directly; no producer fix, syntax
  reconstruction, or alternate opcode spelling is needed.

- Observation: the first integrated direct compile preserves the same semantic PTX instruction as
  NVRTC.
  Evidence: after the provider/host build, direct NVVM emitted `[64,32,32]`, `mul.lo.s32`, and
  `st.global.u32` for `computeMain`; the earlier explicit NVRTC probe emitted the same ABI and
  instruction families.
  Consequence: keep PTX assertions semantic and entry-scoped, then use `ptxas` and runtime values
  for executable proof rather than requiring textual equality.

- Observation: extending the table exposed one intentionally obsolete assertion in the Slice 11
  negotiation test.
  Evidence: the first complete focused run passed every Slice 12 case but stopped at 83/84 because
  `nvvmIRBuilderNegotiatesScalarArrayAddressingAPI` still equated `sizeof(V2)` with the frozen
  Slice 11 minimum. The table is now larger by design. Replacing that check with
  `offsetof(emitIntegerMultiply) == SCALAR_ARRAY_MIN_SIZE` preserves the intended ABI invariant;
  the Slice 12 test separately owns the new full-size assertion. The repeated pre- and post-format
  runs then passed 84/84.
  Consequence: when an append-only suffix is added, the preceding slice's terminal-size assertion
  becomes an adjacency assertion rather than being deleted or weakened.

## Decision Log

- Decision: Slice 12 is exact signed-i32 multiplication, not local arrays, pointer helper ABI, or
  shared memory.
  Rationale: it is the smallest stable executable boundary already preserved by a negative test;
  it reuses the scalar ABI, value map, store, and runtime infrastructure. The adjacent memory and
  helper cases introduce independently meaningful type/allocation ABI decisions.
  Date/Author: 2026-08-27, Codex.
  Revisit when: this exact source fails to retain canonical `kIROp_Mul` after final linking.

- Decision: append a dedicated `emitIntegerMultiply` operation rather than add a new value to the
  frozen `SlangNVVMIntegerBinaryOp_2` contract.
  Rationale: exact Slice 7 providers implement only ADD/SUB. A new enum value would silently widen
  the accepted input domain of an old function pointer and would still need an appended capability
  marker. A dedicated terminal operation makes negotiation and dispatch atomic without relying on
  an old provider's behavior for an unknown enum value.
  Date/Author: 2026-08-27, Codex.
  Revisit when: an ABI audit finds a pre-existing versioned extended-arithmetic mechanism.

## Outcomes and Retrospective

Slice 12, committed as `slice 12`, now accepts canonical two-operand signed-i32
multiplication through the direct NVVM route.
The host and provider append one dedicated operation rather than widening the frozen ADD/SUB enum;
the complete x64 V2 prefix is 272 bytes, while an exact 264-byte Slice 11 provider retains every
program it previously supported. Partial and null suffixes are rejected, future tables are safely
clamped, the identity carries `scalar-integer-multiply=0|1`, and wrapper outputs are sanitized.

The LLVM provider validates the module, current unterminated block, scalar integer types, exact type
match, ownership, same-function availability, and dominance before its only mutation,
`CreateMul`. The final linked Slang shape is canonical and intentional, so the emitter consumes it
directly through `_validateI32Value` and the existing value map. The self-review found no new custom
equivalence, syntax reconstruction, operand-graph walk, fallback, representation repair, or
producer-side invariant break. The only new helpers/special cases are the coherent-prefix support
predicate, partial-prefix rejection, sanitized host wrapper, provider operation, and exact emitter
switch arms; each survives because it owns an established ABI or canonical IR boundary.

The verified provider module contains exactly one `mul i32` feeding the store. Fake routing proves
the exact parameter identities and result consumer. Direct NVVM and NVRTC agree on `[64,32,32]`, a
32-bit multiply, and a global i32 store; CUDA 12.9 `ptxas` accepts both. Both routes produce `42`,
`-42`, and `0` on the RTX 5090. The final focused prefix passes 84/84, and preservation remains
green for parser 1/1, routing/hash 2/2, unsupported IR 1/1, sampler lanes 3/3, NVRTC pass-through
2/2, and runtime dispatch 1/1. The provider exports only the V1/V2 getters and depends on
`KERNEL32.dll` plus delay-loaded `SHELL32.dll` and `ole32.dll`, with no process-visible LLVM DLL.

## Context and Current Pipeline

`source/slang/slang-emit.cpp` links and optimizes the selected raw CUDA program, calls
`validateNVVMSupportedIR`, discovers the optional LLVM 14 provider only after semantic preflight,
and checks the maximum `NVVMIRCapability` before creating a module.
`source/slang/slang-emit-nvvm.cpp` validates the finite direct-call closure in dominance order,
declares functions and parameters, maps canonical Slang IR values to provider handles, emits each
body, verifies/serializes once, and hands LLVM bitcode to the registered libNVVM compiler.

Slice 7's `SlangNVVMEmitIntegerBinary_2` and `SLANG_NVVM_INTEGER_BINARY_OP_ADD/SUB` are already a
frozen append-only provider ABI. `_validateI32Value` is the source of truth for exact signed-i32
constants and available SSA values. The ordinary first-pass instruction switch owns exact opcode
and result-type admission; the second pass owns operand availability/dominance; the emission
switch consumes the canonical value map. Slice 12 adds one operation at each of those existing
boundaries and does not alter AST lowering or reconstruct expression syntax.

## Scope and Non-Goals

In scope:

- exact `kIROp_Mul` with exactly two signed-i32 operands and a signed-i32 result;
- parameters, exact representable constants, phis, calls, and other existing signed-i32 producers
  as multiplication operands when already available and dominant;
- one append-only provider operation, terminal capability, identity bit, and sanitized wrapper;
- fake topology, verified LLVM, differential PTX, both `ptxas` lanes, and runtime evidence.

Explicitly out of scope:

- unsigned, 8/16/64-bit, arbitrary-precision, floating-point, vector, or matrix multiplication;
- multiply-high, overflow flags, saturation, checked arithmetic, fused multiply-add, division,
  remainder, shifts, bitwise operations, casts, or comparisons beyond the existing signed less;
- changing ADD/SUB enum semantics or accepting any other arithmetic opcode;
- pointer scaling, byte addressing, array stride, local/global/shared storage, resources, libdevice,
  thread builtins, barriers, atomics, or waves;
- performance claims beyond semantic PTX classification and successful assembly/runtime.

## Architecture and Invariants

Capability selection remains monotonic. Add terminal `NVVMIRCapability::ScalarIntegerMultiply`
after Slice 11's `ScalarArrayAddressing`. An exact Slice 11 provider remains valid and compiles all
published programs; a multiply program reaches E52016 after provider discovery but before module
creation. The V2 table appends exactly one function pointer, so the expected 64-bit full prefix is
272 bytes while the 264-byte Slice 11 minimum and every older minimum remain frozen. A size inside
the new pointer or a complete prefix with a null operation is malformed; future-larger tables are
accepted and clamped.

Slang preflight accepts only the canonical two-operand `kIROp_Mul` whose result is signed `i32`.
Both operands pass the existing `_validateI32Value`, which owns type, constant representability,
availability, and dominance. The result is added to the existing value set/map and may feed any
already-supported signed-i32 consumer. There is no custom equivalence or syntax fallback.

The provider wrapper clears its output before dispatch and after any failed or success-without-
handle call. The provider requires a live module, non-null output, current unterminated insertion
block, two scalar integer operands of exactly equal type, and values owned by/available in the
current function. All validation precedes the sole `IRBuilder::CreateMul` mutation. LLVM integers
are signless; the Slang boundary, not the provider, owns the signed-i32 policy.

## Interfaces and Dependencies

Append after Slice 11 in `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerMultiply_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue);
```

Add `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE`, host
`supportsScalarIntegerMultiply()`/`emitIntegerMultiply(...)`, identity
`scalar-integer-multiply=0|1`, and terminal Slang capability. No public Slang API changes. The
implementation retains the optional statically linked LLVM 14.0.6 provider, libNVVM/NVRTC, CUDA
12.9 `ptxas`, and CUDA-driver gates.

## Milestones

1. Probe the motivating source after final linking and confirm exact `kIROp_Mul` topology, current
   E52017 stop, and established NVRTC acceptance. Discard any probe files before commit.

2. Freeze the one-member provider suffix in
   `source/compiler-core/slang-nvvm-ir-builder-api.h`, its strict-C ordering probe, coherent host
   negotiation/identity/wrapper, and provider getter. Prove exact Slice 11 and future-table
   compatibility plus partial/null/success-null/failure-after-write behavior.

3. Implement provider multiplication in
   `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` with complete validation before `CreateMul`; add a
   verified positive module and invalid/no-mutation matrix.

4. Extend `source/slang/slang-emit-nvvm.{h,cpp}` and `source/slang/slang-emit.cpp` for exact
   preflight, capability gating, and value-map emission. Extend the fake provider to prove exact
   lhs/rhs/result/store topology and absence of unrelated operations.

5. Promote the retained negative fixture to positive coverage, keep unsigned/wider/floating/vector
   and adjacent arithmetic shapes deterministic unsupported boundaries, and add exact Slice 11
   capability gating.

6. Compile through direct NVVM and NVRTC, compare `[64,32,32]` plus integer-multiply/global-store
   semantics, assemble both outputs, and run positive/negative/zero products through both routes.

7. Apply pinned formatting, rebuild, run the complete focused and preservation matrices, inspect
   exports/dependencies, perform the helper/input-shape audit, update durable docs, and commit only
   tracked Slice 12 files as `slice 12`.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. Every CMake build and test runs outside the
sandbox as required by `AGENTS.md`.

```text
cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm
cmake.exe --build build --config Debug --target slang-test

$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

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

- exact final linked signed-i32 two-operand multiply topology and result-store consumer;
- a dedicated provider field with a frozen 272-byte x64 prefix and exact Slice 11 compatibility;
- provider invalid calls clear outputs and add no LLVM instruction;
- verified LLVM contains exactly one `mul i32` feeding the global store;
- an exact Slice 11 provider gates only multiply after discovery and before module creation;
- direct NVVM/NVRTC expose `[64,32,32]`, integer multiply, and global i32 store semantics;
- both PTX outputs assemble and both runtime routes agree for positive, negative, and zero cases;
- the final focused prefix and preservation matrix pass after formatting;
- the provider still exports only V1/V2 getters and has no LLVM DLL dependency;
- all new helpers/special cases survive the required principled input-shape audit.

## Failure and Recovery

Probes, incremental builds, focused tests, formatting checks, and binary inspection are safe to
repeat. If the expression canonicalizes to another opcode, reassess the producer contract instead
of adding a spelling fallback. If LLVM verification fails, fix provider validation or type
ownership before serialization. If PTX removes the multiply for a constant case, retain the
parameterized differential/runtime fixture and prove the pre-libNVVM operation through fake and
LLVM assembly rather than asserting exact optimized spelling.

Do not delete/reset the user's worktree or stage `external/slang-binaries/`, any ExecPlan, or probe
sources. Remove Slice 12 probes with `apply_patch` before committing. The experimental direct route
remains removable without affecting default NVRTC dispatch.

## Artifacts and Hand-Off

The exact linked IR is `mul(%x, %y)` followed by `store(%destination, %result)` in a
`Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)` entry point. The frozen old/new x64
prefix sizes are 264/272 bytes. Verified LLVM, invalid/no-mutation, PTX, both-assembler, runtime,
test-count, binary-surface, and helper/input-shape evidence are summarized above and distilled into
`docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md`. The next bounded
slice is exact signed-i32 bitwise AND, whose canonical `kIROp_BitAnd` producer can reuse this
operand/value/store path without inheriting division edge semantics. Keep this and prior ExecPlans
untracked.
