# AND signed i32 values through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the prepared,
uncommitted successor working log for Slice 13 of the direct NVVM backend experiment; do not begin
implementation until Slice 12 has completed and been committed.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts the smallest remaining bitwise expression with a
stable canonical producer: bitwise AND of two signed `i32` values.

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x & y;
}
```

The final linked Slang IR must contain one exact two-operand `kIROp_BitAnd` whose operands and
result are signed `i32`; that result feeds the established device-pointer store. The provider
emits LLVM `and i32` only after validating the current unterminated insertion block and two
available, same-function scalar integer operands of identical LLVM type. Direct NVVM and NVRTC
must agree on raw parameter widths `[64, 32, 32]`, expose a 32-bit bitwise-AND operation and global
store, assemble through `ptxas`, and produce identical positive, negative-bit-pattern, and zero
results at runtime.

## Progress

- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the Slice 12 ExecPlan, and the current scalar
  ABI/provider, linked-IR preflight/emission, fake-provider, PTX-summary, `ptxas`, and runtime-test
  patterns.
- [x] (2026-08-27) Traced the expected pre-slice producer and failure boundary from source
  bitwise `&` to canonical `kIROp_BitAnd`, then to the direct validator's default E52017 path
  before provider discovery.
- [x] (2026-08-27) Prepared this planning-only hand-off and the parameterized probe source without
  editing tracked production, tests, or documentation, and without building or testing.
- [x] (2026-08-27) After committing Slice 12 as `slice 12`, measured the final
  post-`simplifyIR` topology, direct E52017 `'and'` boundary, and explicit NVRTC
  `[64,32,32]`/`and.b32`/`st.global.u32` evidence.
- [x] (2026-08-27) Appended and implemented the coherent dedicated integer-bit-AND provider
  suffix, host negotiation/identity/wrapper, provider validation, and strict-C probes; the first
  Release provider build is green.
- [x] (2026-08-27) Extended direct-NVVM preflight, terminal capability gating, and canonical
  value-map emission for exact two-operand signed-i32 `kIROp_BitAnd`; the Debug `slangc` build and
  first integrated direct compile are green.
- [x] (2026-08-27) Added ABI, provider, fake-topology, capability-gate, PTX, `ptxas`, runtime,
  and negative tests. After correcting two test-only ABI expectations, the focused prefix passes
  92/92.
- [x] (2026-08-27) Applied pinned clang-format 17, rebuilt both binaries, passed the final 92/92
  focused prefix and complete preservation matrix, audited the production/test diffs, updated
  durable docs, removed the probes, and inspected the provider binary surface.
- [x] (2026-08-27) Committed the tracked work as `slice 13`.

## Surprises and Discoveries

- Observation: the source producer has an exact ordinary bitwise-AND opcode rather than sharing
  the older arithmetic enum.
  Evidence: `source/slang/slang-lower-to-ir.cpp` maps
  `BuiltinOperationKind::BitAnd` to `kIROp_BitAnd`, and
  `IRBuilder::emitBitAnd` in `source/slang/slang-ir.cpp` constructs that exact instruction with
  left and right operands. `source/slang/slang-ir-insts.lua` gives the opcode the textual name
  `and`.
  Consequence: admit and lower exact `kIROp_BitAnd`; do not add an alternate spelling, reinterpret
  another opcode, or accept `kIROp_ConstexprBitAnd` as a fallback.

- Observation: the measured parameterized probe stops with E52017 `'and'` before provider
  discovery and retains the expected final shape.
  Evidence: every repeated IR dump ends with
  `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)`, one
  `let %result : Int = and(%x, %y)`, `store(%destination, %result)`, and a void return. With an
  explicit output and a deliberately missing builder path, the direct selector reports
  `error[E52017]: ... 'and'`, proving semantic preflight wins over provider discovery. Explicit
  NVRTC succeeds and emits parameters `[64,32,32]`, `and.b32`, and `st.global.u32`.
  Consequence: implement exact ordinary `kIROp_BitAnd`; no producer fix, spelling fallback, cast,
  or syntax reconstruction is needed.

- Observation: route-sensitive `slangc` probes need an explicit output artifact.
  Evidence: when `-o` was omitted, the stdout compilation followed the NVRTC path despite the
  direct selector and accepted both the AND and known barrier fixtures. Repeating the same target
  options with `-o` selected the direct route and produced the expected E52017 diagnostic.
  Consequence: every direct/NVRTC CLI probe in this and later plans uses a distinct explicit
  output path. The API-level routing tests remain the authoritative selector/hash proof.

- Observation: the first integrated direct compile preserves the same exact PTX operation as
  NVRTC without an address-space conversion in the direct lane.
  Evidence: both outputs have `[64,32,32]`, `and.b32`, and `st.global.u32`. Direct NVVM targets
  `sm_70` and stores through its address-space-1 parameter directly; NVRTC targets the locally
  selected `sm_75` spelling and first emits `cvta.to.global.u64`. These differences are expected
  route details and do not change the launch ABI or bitwise/store semantics.
  Consequence: compare semantic entry summaries and executable results, not textual PTX equality.

- Observation: the first complete focused run exposed two obsolete test expectations rather than
  implementation defects.
  Evidence: every real provider, direct topology, PTX, assembler, and runtime case passed, but the
  run ended 90/92. The Slice 12 negotiation test still equated the now-extended `sizeof(V2)` with
  its frozen prefix; it now asserts that `offsetof(emitIntegerBitAnd)` equals that minimum. The new
  uninitialized-wrapper case expected `SLANG_E_NOT_AVAILABLE`, while every host wrapper correctly
  returns `SLANG_E_UNINITIALIZED` before capability dispatch. After those exact corrections, both
  the pre-format and final post-format runs passed 92/92.
  Consequence: keep distinguishing table-prefix availability from builder initialization, and
  evolve each preceding terminal-size test into an adjacency assertion when appending a field.

- Observation: the frozen Slice 7 integer-binary provider operation cannot be widened safely.
  Evidence: `SlangNVVMIntegerBinaryOp_2` publishes only ADD and SUB, while provider
  `_emitIntegerBinary` explicitly rejects every other enum value before mutation. Slice 12 already
  established the append-only dedicated-operation pattern for multiplication.
  Consequence: append a dedicated bit-AND function pointer. Do not add AND to the old enum or make
  old providers interpret a previously invalid value.

- Observation: LLVM integer values are signless, but the accepted Slang subset is not.
  Evidence: provider `_areMatchingIntegerValues` verifies scalar `IntegerType`, exact type
  equality, ownership, function availability, and dominance; `_validateI32Value` owns the exact
  signed-`i32` frontend policy.
  Consequence: reuse both existing validators at their respective boundaries. Do not teach the
  provider about Slang signedness or create a custom equivalence relation.

## Decision Log

- Decision: Slice 13 is exact signed-i32 bitwise AND, not a general bitwise family.
  Rationale: one ordinary opcode is the smallest independently executable increment. OR, XOR,
  shifts, bitwise NOT, logical operations, atomics, vectors, and additional integer ABIs each have
  separate producer or semantic boundaries and remain useful negative coverage.
  Date/Author: 2026-08-27, Codex.
  Revisit when: the final linked parameterized probe does not retain exact `kIROp_BitAnd`.

- Decision: append `emitIntegerBitAnd` as a dedicated V2 operation instead of extending
  `SlangNVVMIntegerBinaryOp_2`.
  Rationale: exact Slice 7 providers accept only ADD/SUB and reject unknown values. Extending the
  enum would silently change an old callable's input domain and still require a new capability
  marker. One terminal function pointer negotiates support and dispatch atomically.
  Date/Author: 2026-08-27, Codex.
  Revisit when: an ABI audit finds a pre-existing versioned extended-bitwise operation.

- Decision: publish one 280-byte x64 terminal prefix after Slice 12's frozen 272-byte prefix.
  Rationale: the only appended member is one 64-bit function pointer. Sizes 273 through 279 are
  partial and malformed; 272 remains a complete Slice 12 provider; 280 is the complete Slice 13
  capability; future-larger tables are accepted and clamped to the locally known 280 bytes.
  Date/Author: 2026-08-27, Codex.
  Revisit when: compile-time offset probes contradict the expected Windows x64 layout.

- Decision: emit ordinary LLVM `and` with `IRBuilder::CreateAnd` after all validation.
  Rationale: Slang's exact `kIROp_BitAnd` and LLVM's signless integer AND have the same bitwise
  semantics. No signedness flag, arithmetic enum, or target-specific rewrite is needed.
  Date/Author: 2026-08-27, Codex.
  Revisit when: LLVM verification or direct/NVRTC runtime comparison exposes a representation
  mismatch.

## Outcomes and Retrospective

Slice 13, committed as `slice 13`, accepts canonical two-operand signed-i32 bitwise
AND through the direct NVVM route. The
private V2 table appends a dedicated operation rather than widening the frozen ADD/SUB callable;
the complete x64 prefix is 280 bytes and an exact 272-byte Slice 12 provider retains all prior
programs. Sizes 273--279 and a complete null suffix are rejected, future tables are clamped, the
identity carries `scalar-integer-bit-and=0|1`, and failed or success-without-handle calls expose no
stale output.

The LLVM provider validates the live module, current unterminated insertion block, exact scalar
integer type match, module/context/function ownership, availability, and dominance before its only
mutation, `CreateAnd`. The final Slang IR is canonical and intentional; the emitter validates exact
ordinary `kIROp_BitAnd` through `_validateI32Value` and lowers it through the existing value map.
Independent production and test audits found no fallback, custom equivalence, syntax rebuilding,
operand-graph rediscovery, representation repair, or unrelated deletion. The new support
predicate, partial-prefix guard, sanitized wrapper, provider endpoint, and exact emitter switch
arms each own an established ABI or canonical-IR boundary and survive the helper/input-shape audit.

Verified LLVM contains exactly one `and i32` feeding the store, and fake routing proves the exact
parameter/result identities. Direct NVVM and NVRTC agree on `[64,32,32]`, `and.b32`, and a global
i32 store; CUDA 12.9 `ptxas` accepts both. Both runtime lanes produce `0x18` for `0x5a & 0x3c`,
`0x12345678` for `-1 & 0x12345678`, `-4` for `-2 & -4`, and `0` for `0 & -1` on the RTX 5090.
The final focused prefix passes 92/92. Preservation remains green for parser 1/1, routing/hash 2/2,
unsupported IR 1/1, sampler lanes 3/3, NVRTC pass-through 2/2, and runtime dispatch 1/1. The DLL
exports only the V1/V2 getters and depends on `KERNEL32.dll` plus delay-loaded `SHELL32.dll` and
`ole32.dll`, with no process-visible LLVM DLL.

## Context and Current Pipeline

The motivating `x & y` expression is semantically resolved as
`BuiltinOperationKind::BitAnd`. `source/slang/slang-lower-to-ir.cpp` selects
`kIROp_BitAnd`, and `IRBuilder::emitBitAnd` creates an ordinary instruction with the result type
and two operands. The parameterized source deliberately prevents constant folding; the execution
probe must confirm that linking, optimization, and repeated `simplifyIR` retain one signed-i32
`kIROp_BitAnd` feeding `store(destination, result)`.

`source/slang/slang-emit.cpp` links and optimizes the selected CUDA entry point, calls
`validateNVVMSupportedIR`, discovers the optional LLVM 14 provider only after semantic preflight,
and checks the maximum `NVVMIRCapability` before creating a builder module.
`source/slang/slang-emit-nvvm.cpp` validates the finite direct-call closure in dominance order,
declares functions and parameters, maps canonical Slang IR values to provider handles, emits each
body, verifies and serializes once, and hands LLVM bitcode to libNVVM.

Slice 12's `SlangNVVMEmitIntegerMultiply_2` is the immediate append-only ABI predecessor.
`_validateI32Value` is the source of truth for exact signed-i32 constants and available SSA
values. The first instruction pass owns exact opcode, operand count, and result-type admission;
the second pass owns operand type, availability, and dominance; the body-emission switch consumes
the canonical value map. Slice 13 adds one operation at each existing boundary. It does not change
AST checking/lowering, reconstruct syntax, repair malformed IR, or introduce another value
representation.

Before the slice, exact `kIROp_BitAnd` reaches the default first-pass diagnostic. Because the IR
opcode's stable textual spelling is `and`, the expected stop is E52017
`direct NVVM lowering does not support ... 'and'`, with no builder load request. This expectation
must be replaced by observed evidence during Milestone 1.

## Scope and Non-Goals

In scope:

- exact ordinary `kIROp_BitAnd` with exactly two signed-i32 operands and a signed-i32 result;
- parameters, exact representable constants, phis, calls, multiplication results, and other
  already-supported signed-i32 producers as operands when available and dominant;
- one appended provider operation, terminal capability, stable identity bit, and sanitized host
  wrapper;
- fake topology, verified LLVM, direct/NVRTC differential PTX, both `ptxas` lanes, and runtime
  bit-pattern evidence.

Explicitly out of scope:

- `kIROp_BitOr`, `kIROp_BitXor`, `kIROp_BitNot`, shifts, logical AND/OR, comparisons, or select
  changes;
- `kIROp_ConstexprBitAnd` as an alternate accepted body spelling;
- unsigned, bool, 8/16/64-bit, arbitrary-precision, vector, matrix, or aggregate bitwise values;
- expanding the raw kernel/helper ABI or changing ADD/SUB/multiply semantics;
- bitfields, casts/reinterpretation, atomics, reductions, waves, resource masks, pointer masking,
  local/shared/global declarations, thread builtins, barriers, or libdevice;
- performance claims beyond semantic PTX classification and successful assembly/runtime.

## Architecture and Invariants

Capability selection remains monotonic. Add terminal
`NVVMIRCapability::ScalarIntegerBitAnd` after Slice 12's `ScalarIntegerMultiply`. An exact Slice
12 provider remains valid and compiles every previously published program; a bit-AND program
reaches E52016 after provider discovery but before builder-module creation or libNVVM use.

The V2 table appends exactly one `SlangNVVMEmitIntegerBitAnd_2` pointer. Preserve the 272-byte
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE`; publish
`SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE`, expected to be 280 bytes on x64.
Require:

- `offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitAnd)` equals the frozen Slice 12 minimum;
- size 272 initializes successfully with `supportsScalarIntegerBitAnd() == false`;
- every size greater than 272 and less than 280 is rejected as a partial suffix;
- size at least 280 requires a non-null `emitIntegerBitAnd` member;
- a future-larger table is accepted, copied only through 280 bytes, and reports local size 280.

The provider identity appends `scalar-integer-bit-and=0|1`, so shader-cache identity differs when
the capability differs. Exact old prefixes report zero; a coherent full/future prefix reports one.

Slang preflight accepts only exact two-operand `kIROp_BitAnd` with signed-i32 result. Both operands
pass `_validateI32Value`; the result joins the existing available-value set and emission map and
may feed any existing signed-i32 consumer. No custom equality, opcode fallback, operand-graph walk,
or syntax reconstruction is permitted.

The host wrapper clears its output before dispatch and passes a private cleared slot. It also
clears after a failed provider call and converts success-without-handle to failure. Unsupported or
uninitialized builders return the established error without exposing a stale handle.

Provider `_emitIntegerBitAnd` clears a non-null output first, obtains a live current unterminated
insertion block with `_getValidInsertionBlock`, and validates both handles through
`_areMatchingIntegerValues`. Thus the operands must be scalar LLVM integers of exactly equal type,
belong to the same module/context and current function, and be available/dominant at the insertion
point. Only after all checks pass may it call `state->builder.CreateAnd(left, right)` and publish
the result. Invalid calls add no LLVM instruction.

## Interfaces and Dependencies

Append after Slice 12 in `SlangNVVMBuilderAPI_V2`:

```c
typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitAnd_2)(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue);
```

Add:

- table member `emitIntegerBitAnd`;
- `SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE`;
- host `supportsScalarIntegerBitAnd()` and `emitIntegerBitAnd(...)`;
- identity component `scalar-integer-bit-and=0|1`;
- terminal `NVVMIRCapability::ScalarIntegerBitAnd` and its gate.

No public Slang API changes. The implementation retains the optional statically linked LLVM
14.0.6 provider, libNVVM/NVRTC, CUDA 12.9 `ptxas`, and CUDA-driver environment gates already used
by the focused suite.

## Milestones

1. After committing Slice 12, run the retained probe through final linking with `-dump-ir` and
   direct NVVM. Confirm the exact function signature, one two-operand signed-i32
   `kIROp_BitAnd`, its store consumer, expected E52017 `'and'`, and zero builder discovery in a
   focused fake test. Compile the same source through explicit NVRTC and record `[64,32,32]`,
   32-bit AND, and global-store PTX. Promotion requires this stable canonical shape. If it folds,
   changes opcode, or introduces a cast, investigate the producer and revise the slice instead of
   adding a downstream spelling fallback. Remove the probe with `apply_patch` before commit.

2. Freeze the provider suffix in
   `source/compiler-core/slang-nvvm-ir-builder-api.h`, add strict-C minimum-size and capability-
   order probes in `source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c`, and update coherent host
   negotiation, support query, identity, sanitized wrapper, and provider getter. Prove exact Slice
   12, partial 273--279, full-null, full, and future-larger behavior plus uninitialized,
   unsupported, invalid-input, success-null, and failure-after-write output clearing.

3. Implement provider `_emitIntegerBitAnd` in
   `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` by reusing `_getValidInsertionBlock` and
   `_areMatchingIntegerValues`, then calling `CreateAnd`. Add a verified positive module with
   exactly one `and i32` feeding the store. Add invalid/no-mutation cases for null module/output,
   no or terminated insertion block, pointer/non-integer operand, mismatched integer width,
   foreign module/context/function, same-block use after insertion, and sibling/non-dominating
   instruction values.

4. Extend `source/slang/slang-emit-nvvm.{h,cpp}` and the capability gate in
   `source/slang/slang-emit.cpp`. Add exact first-pass admission, terminal capability requirement,
   second-pass `_validateI32Value` checks, available-value registration, and body emission through
   the canonical value map and `builder.emitIntegerBitAnd`. Keep preflight and emission switches
   structurally parallel.

5. Extend the fake provider and fixtures in
   `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`. Prove the operation receives kernel
   parameters 1 and 2 in the entry block; its result is the store value; and no old integer-binary,
   multiply, load, branch, call, phi, pointer-offset, or array operation is used. Gate a bit-AND
   source against an exact Slice 12 provider after discovery/before module creation, while proving
   multiplication still works on that provider.

6. Preserve deterministic adjacent boundaries. Keep signed-i32 bitwise OR and XOR at E52017
   `'or'`/`'xor'` before builder discovery, and keep raw unsigned/wide integer AND sources at the
   existing `'entry-point parameter'` boundary. Do not use a semantically invalid floating-point
   `&` fixture merely to manufacture a direct-backend diagnostic.

7. Compile the parameterized source through direct NVVM and NVRTC. Extend `PTXEntrySummary` with
   entry-scoped, token-boundary 32-bit bitwise-AND classification; compare `[64,32,32]`, AND, and
   global-store semantics; assemble both outputs; and launch both routes for representative bit
   patterns such as `0x5a & 0x3c == 0x18`, `-1 & 0x12345678 == 0x12345678`,
   `-2 & -4 == -4`, and `0 & -1 == 0`.

8. Apply pinned formatting, rebuild, run the complete focused and preservation matrices outside
   the sandbox, inspect exports/dependencies, perform the required helper/input-shape audit, update
   durable docs, and commit only tracked Slice 13 files as `slice 13`.

## Validation and Acceptance

Run from `C:\src\slang` with Windows-native tools. Every CMake build and test must run outside the
sandbox as required by `AGENTS.md`.

Prototype commands after Slice 12 is committed:

```text
$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvvm -dump-ir `
  -o issue-nvvm-backend\probe.slice13.direct.ptx `
  issue-nvvm-backend\probe.slice13.i32-bit-and.slang
build\Debug\bin\slangc.exe -target ptx -entry computeMain -stage compute `
  -capability cuda_sm_7_0 -emit-cuda-via-nvrtc `
  -o issue-nvvm-backend\probe.slice13.nvrtc.ptx `
  issue-nvvm-backend\probe.slice13.i32-bit-and.slang
```

Build and focused test commands:

```text
cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm
cmake.exe --build build --config Debug --target slang-test

$env:SLANG_NVVM_BUILDER_PATH =
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Follow the Slice 12 eight-test pattern with these focused contracts (names may be adjusted only to
match local naming conventions):

- `nvvmIRBuilderNegotiatesScalarIntegerBitAndAPI`;
- `nvvmIRBuilderRejectsInvalidIntegerBitAndOperations`;
- `nvvmIRBuilderBuildsIntegerBitAndKernel`;
- `nvvmSlangIntegerBitAndUsesDirectPipeline`;
- `nvvmSlangNegotiatesScalarIntegerBitAndCapability`;
- `nvvmSlangRealIntegerBitAndDifferentialPTX`;
- `nvvmSlangRealIntegerBitAndPtxasAccepts`;
- `nvvmSlangIntegerBitAndRuntimeMatchesNVRTC`.

If Slice 12's final focused prefix remains 84/84 and these remain eight independent tests, the
expected Slice 13 prefix is 92/92. Record the actual count rather than treating this estimate as
acceptance evidence.

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

- measured final linked topology: exact two-operand signed-i32 `kIROp_BitAnd` feeding the store;
- measured pre-change baseline E52017 `'and'` before builder discovery;
- dedicated ABI field with frozen 280-byte x64 prefix and exact 272-byte Slice 12 compatibility;
- strict partial/null/future negotiation, stable identity bit, and complete output sanitization;
- provider invalid calls clear outputs and add no LLVM instruction;
- verified LLVM contains exactly one `and i32` whose result feeds the global store;
- fake topology proves exact parameter operands, result/store flow, and absence of unrelated calls;
- an exact Slice 12 provider compiles prior multiply work but gates bit AND after discovery and
  before module or libNVVM creation;
- signed OR/XOR and unsigned/wide AND boundaries remain deterministic and stop at their intended
  pre-provider diagnostics;
- direct NVVM/NVRTC expose `[64,32,32]`, 32-bit bitwise-AND, and global i32 store semantics;
- both PTX outputs assemble and both runtime routes agree for positive, negative, and zero masks;
- the final focused prefix and preservation matrix pass after pinned formatting;
- the provider still exports only V1/V2 getters and has no LLVM DLL dependency;
- every new helper/fallback/special case survives the required principled input-shape audit.

## Failure and Recovery

The probe, incremental builds, focused tests, formatter, and binary inspection are safe to repeat.
If the final expression becomes `kIROp_ConstexprBitAnd`, folds, or acquires an unexpected cast,
audit the producer and optimization trace; do not accept multiple shapes downstream merely to make
the fixture pass. Retain a parameterized source so PTX cannot remove the operation as constant.

If LLVM verification fails, fix provider validation or ownership/dominance before serialization.
If a CUDA toolchain spells the optimized operation differently (for example with a logically
equivalent instruction), first prove its entry-scoped truth-table semantics and record the
discovery; do not weaken the PTX classifier to an arbitrary substring. `ptxas` and runtime equality
remain required executable evidence.

If ABI layout differs from 280 bytes, stop and inspect field ordering/alignment rather than
changing the frozen 272-byte predecessor. If a partial or null table initializes, fix negotiation
before enabling emission. If a failed provider call leaves a handle or instruction, fix
sanitization/no-mutation at the provider or wrapper boundary before continuing.

Do not delete/reset the user's worktree or stage `external/slang-binaries/`, any ExecPlan, or probe
artifact. Remove `probe.slice13.i32-bit-and.slang` and generated PTX/dumps with `apply_patch`
before committing. The direct route remains experimental and removable without changing default
NVRTC dispatch.

## Artifacts and Hand-Off

The exact linked IR is `and(%x, %y)` followed by `store(%destination, %result)` in a
`Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)` entry point. Frozen old/new x64
prefixes are 272/280 bytes. The verified LLVM, negotiation, invalid/no-mutation, fake topology,
capability-gate, PTX, assembler, runtime, test-count, binary-surface, formatter, and helper audit
evidence is summarized above and distilled into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md`. Slice 14 is prepared as exact signed-i32 bitwise
OR with its own untracked ExecPlan. Keep this and prior ExecPlans untracked.
