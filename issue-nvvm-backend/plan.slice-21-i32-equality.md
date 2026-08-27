# Slice 21: Compare signed i32 values for equality through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires every completed slice plan to ship with its implementation, so this plan will be committed
with Slice 21 rather than left as an uncommitted working log.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts canonical equality between two signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left == right ? 1 : 0;
}
```

The final linked Slang IR must retain exact `kIROp_Eql` with signed-`i32` operands and a Boolean
result consumed by the already-supported conditional branch. The provider emits LLVM `icmp eq`
only after validating the current unterminated insertion point and both same-type scalar integer
operands. Direct NVVM and NVRTC must agree on raw parameter widths `[64,32,32]`, equality-controlled
selection, global storage, `ptxas` acceptance, and equal/not-equal runtime results.

## Progress

- [x] (2026-08-27) Returned `nvvm-backend` to the committed Slice 19 production baseline while
  preserving the completed LLVM 7 feasibility work on `experiment/nvvm-llvm7-bitcode` as Slice 20.
- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the development roadmap, capability ledger, Slice 19
  hand-off, and the established signed-i32 comparison/provider boundaries.
- [x] (2026-08-27) Measured the motivating final linked IR, pre-change E52017 boundary, and NVRTC
  reference.
- [x] (2026-08-27) Appended and implemented the exact equality provider operation, host negotiation, identity,
  validation, and strict-C ABI probes without widening the frozen less-than callable.
- [x] (2026-08-27) Extended direct preflight, capability gating, and canonical value-map emission.
- [x] (2026-08-27) Added ABI, invalid/no-mutation, fake topology, old-provider gate, differential PTX, `ptxas`,
  runtime, and adjacent-negative coverage.
- [x] (2026-08-27) Formatted with clang-format 17.0.6, rebuilt, ran the complete focused and
  preservation matrices, inspected the provider binary, updated durable design/ledger evidence,
  and completed the self-review.

## Surprises and Discoveries

- Observation: final linked Slang IR retains exact `cmpEQ(left, right) : Bool`; the conditional
  expression lowers through the established conditional branch, two integer constants, one
  integer phi, and one global store. The pre-change direct route stopped at E52017 `cmpEQ` before
  provider discovery, while NVRTC accepted the same source.
  Evidence: `-dump-ir` on the motivating probe, the old fake-provider boundary, and the NVRTC
  reference compile.

- Observation: direct libNVVM and NVRTC both select exact 32-bit equality rather than rewriting the
  comparison into arithmetic. Direct PTX contains `[64,32,32]`, `setp.eq.s32`, `selp.u32`, and
  `st.global.u32`; the semantic PTX classifier accepts the equivalent NVRTC instruction family.
  Evidence: the probe PTX and `nvvmSlangRealIntegerEqualDifferentialPTX`.

- Observation: the new equality callable is one pointer after the coherent Slice 19 suffix. The
  complete V2 table is 336 bytes on x64 and 188 bytes on x86; the old complete sizes are 328 and
  184 bytes.
  Evidence: strict-C layout probes and `nvvmIRBuilderNegotiatesScalarIntegerEqualAPI`.

## Decision Log

- Decision: Slice 21 implements exact signed-i32 equality before the broad resource/optimization
  roadmap item.
  Rationale: Bucket 2 remains the lowest incomplete capability bucket. Equality has a canonical,
  side-effect-free producer and defined LLVM/PTX semantics, while the design explicitly records an
  unresolved negative/oversized-count policy for shifts. This slice closes one smaller established
  scalar boundary without inheriting resource ABI or shift-policy decisions.
  Date/author: 2026-08-27, Codex.
  Revisit when: final linked IR does not retain `kIROp_Eql` or the source requires a producer fix.

- Decision: append a dedicated equality callable instead of extending the frozen signed-less-than
  callable or introducing a general comparison enum.
  Rationale: existing providers implement one operation with fixed semantics. Appending one exact
  function pointer preserves old-provider behavior and makes support negotiation atomic.
  Date/author: 2026-08-27, Codex.
  Revisit when: an existing versioned general comparison interface is found before implementation.

## Outcomes and Retrospective

Slice 21 accepts exact canonical signed-i32 equality without widening the established Boolean or
entry-point ABI. Slang preflight owns exact `kIROp_Eql`, two signed-i32 operands, and a canonical
Boolean result. The provider reuses the shared binary integer ownership/type/availability validator
and performs all checks before exact `ICMP_EQ`; its `i1` result feeds only the already-supported
conditional consumer. The complete V2 table is 336 bytes on x64 and 188 bytes on x86. An exact
Slice 19 provider remains usable but gates equality at E52016 before module creation; partial and
null complete suffixes reject, and future tables clamp safely.

The input-shape audit found no producer defect. `cmpEQ` is the canonical final linked opcode from
ordinary equality syntax, its Boolean result is intentional, and the existing signed-i32 value and
dominance checks already own both operands. The implementation adds no custom equivalence,
fallback, arbitrary operand walk, syntax reconstruction, or downstream repair. The only new helper
at the provider boundary factors the already-identical less-than/equality validation and selects an
explicit LLVM predicate; removing the equality cases restores the measured E52017 failure, proving
this is the responsible layer.

The Release provider and host build pass. The focused NVVM prefix passes 148/148, including real
libNVVM/NVRTC differential PTX, both CUDA 12.9 `ptxas` lanes, and RTX 5090 runtime equal/not-equal
cases. Preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2
CUDA compile/pass-through, and 1/1 runtime dispatch. The DLL exports only
`slang_getNVVMBuilderAPI_V1` and `slang_getNVVMBuilderAPI_V2`; its ordinary dependency remains
`KERNEL32.dll`, with delay-loaded `SHELL32.dll` and `ole32.dll`, and no LLVM DLL.

## Context and Current Pipeline

`source/slang/slang-emit.cpp` links and optimizes the selected raw CUDA program, calls
`validateNVVMSupportedIR`, discovers the optional provider only after semantic preflight, and gates
module creation on the maximum `NVVMIRCapability`. `source/slang/slang-emit-nvvm.cpp` validates the
finite direct-call closure, maps canonical Slang IR values to opaque provider handles, emits each
function in dominance order, verifies/serializes once, and hands the artifact to libNVVM.

Slice 7 already owns exact signed-i32 `kIROp_Less`: preflight validates two signed-i32 operands and
a Boolean result, the provider validates same-type scalar LLVM integers before `CreateICmpSLT`, and
the resulting i1 may feed `emitConditionalBranch`. Equality has the same producer/consumer shape
but distinct semantics. The canonical Slang opcode is the source of truth; this slice must not
infer equality from source syntax, branch topology, subtraction, or compare-result use.

## Scope and Non-Goals

In scope are exact two-operand `kIROp_Eql` with signed-i32 operands and Boolean result, a dedicated
append-only provider operation, deterministic provider identity/capability gating, verified LLVM,
fake graph evidence, direct/NVRTC PTX comparison, both assembler lanes, and runtime equal/not-equal
cases.

Out of scope are inequality and ordered comparisons other than established signed less-than;
unsigned, wider, floating-point, pointer, vector, matrix, aggregate, or resource equality; Boolean
value storage or return ABI; shifts, division, remainder, resources, new address spaces, builtins,
barriers, other atomics, waves, and optimization-quality claims.

## Architecture and Invariants

The V2 table remains append-only. Slice 21 appends one `emitIntegerEqual` pointer after the complete
Slice 19 prefix. An exact Slice 19 provider remains valid for every earlier program; an equality
program reaches E52016 after discovery but before module creation. Sizes within the pointer are
malformed, a complete prefix with a null equality callable is rejected, and future-larger tables
are accepted and clamped.

Slang preflight accepts only `kIROp_Eql` with exactly two values that pass the existing signed-i32
validation and whose result is the canonical Boolean type. The value map records the provider's i1
handle, which may feed only already-supported Boolean consumers. Provider validation must finish
before its sole `IRBuilder::CreateICmpEQ` mutation and must clear output handles on every failure.
LLVM integers are signless; Slang preflight owns the signed-i32 policy.

## Interfaces and Dependencies

Append to `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerEqual_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue);
```

Add `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE`, host
`supportsScalarIntegerEqual()`/`emitIntegerEqual(...)`, identity
`scalar-integer-equal=0|1`, and terminal `NVVMIRCapability::ScalarIntegerEqual`. No public Slang API
changes. The production baseline remains the optional LLVM 14.0.6 provider plus the negotiated
NVVM IR 2.0 text serializer and CUDA 12.9 libNVVM/NVRTC/`ptxas`/driver validation.

## Milestones

1. Probe the motivating source after final linking and confirm exact equality topology, current
   E52017 stop, and explicit NVRTC acceptance.
2. Append the strict-C ABI field/minimum and host support predicate, partial-prefix validation,
   identity bit, and sanitized wrapper. Preserve the exact Slice 19 prefix.
3. Implement provider equality with complete ownership/type/availability validation before
   `CreateICmpEQ`; add verified positive and invalid/no-mutation tests.
4. Extend preflight, capability gating, and body emission for canonical signed-i32 equality; extend
   the fake provider to prove exact operands and branch/result topology.
5. Prove exact Slice 19 provider compatibility, retain adjacent unsupported equality types, and run
   direct/NVRTC PTX, matching-root `ptxas`, and equal/not-equal GPU runtime cases.
6. Apply pinned formatting, rebuild outside the sandbox, run focused/preservation tests, inspect
   exports/dependencies, update durable documentation, and perform the required helper and
   input-shape audit before committing only intended files.

## Validation and Acceptance

Run Windows-native CMake builds and every test outside the sandbox as required by `AGENTS.md`:

```text
cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm -- /m
cmake.exe --build build --config Release --target slang-unit-test slang-test -- /m
$env:SLANG_NVVM_BUILDER_PATH='C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm
```

Then run the established parser 1/1, routing/hash 2/2, unsupported 1/1, sampler 3/3, CUDA
compile/pass-through 2/2, and runtime-dispatch 1/1 preservation prefixes.

Acceptance requires exact canonical IR evidence; one append-only callable and frozen old prefix;
provider invalid calls that clear outputs and insert no instruction; verified `icmp eq i32`;
old-provider gating before module creation; direct/NVRTC ABI and semantic PTX agreement; both
`ptxas` lanes; equal/not-equal GPU results; green final matrices; unchanged V1/V2 export allowlist
and no LLVM DLL; and no custom equivalence, syntax reconstruction, fallback, or producer repair.

## Failure and Recovery

Probes, builds, tests, formatting, and binary inspection are safe to repeat. If equality folds away
or canonicalizes to another opcode, fix the fixture or reassess the producer instead of adding a
spelling fallback. If LLVM verification or PTX differs, trace the exact provider instruction and
consumer before changing preflight. Do not delete or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep final canonical IR, ABI sizes, provider assembly, PTX classification, assembler/runtime
results, test counts, binary surface, decisions, and the self-review in this plan. Distill durable
architecture into `docs/design/nvvm-backend.md` and test status into
`docs/design/nvvm-backend-capability-ledger.md`. Commit this completed plan with Slice 21 using a
message whose first line is `slice 21` and whose body describes the capability and validation.
