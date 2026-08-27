# Add relaxed global signed-i32 atomic add

This ExecPlan follows `.agent/PLANS.md`. At the user's request, this slice's completed plan is kept
with its implementation commit so the compatibility decision and its evidence remain reviewable.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts one canonical atomic boundary: a relaxed signed-i32
atomic add through the established read-write device pointer ABI. Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, 1);
}
```

Launching multiple CUDA threads against one initialized integer must produce the same final count
through NVRTC and direct NVVM. The LLVM 14 provider must emit a verifier-valid global-address-space
atomic operation, libNVVM must compile it, the matching CUDA toolkit's `ptxas` must accept the PTX,
and the local GPU must execute it. The slice must preserve all earlier provider prefixes and reject
other atomic operations, types, address spaces, pointer forms, and memory orders before provider
discovery.

This is one bounded half of the existing “atomics and wave operations” roadmap entry. Wave
operations remain a later slice because their lane convergence, intrinsic selection, and executable
proof are independent semantics.

## Progress

- [x] (2026-08-27) Started Slice 19 immediately after committing Slice 18 as `slice 18`.
- [x] (2026-08-27) Chose exact relaxed global signed-i32 atomic add as the first bounded atomic
  candidate because it has a canonical `kIROp_AtomicAdd` producer and a multi-thread runtime proof
  that requires no new thread-index builtin.
- [x] (2026-08-27) Measured the final linked Slang IR, NVRTC PTX, direct pre-change rejection,
  memory-order operand, pointer type/access, and runtime launch contract.
- [x] (2026-08-27) Confirmed from LLVM 14 and NVIDIA's primary specifications that Slang Relaxed
  maps to LLVM `monotonic`, `atomicrmw add` returns the old value, NVVM supports 32/64-bit
  `atomicrmw` on global/shared/generic pointers, and the unsuffixed CUDA atomic reference is
  relaxed at device scope.
- [x] (2026-08-27) Froze the append-only builder ABI as one coherent two-field capability:
  `emitRelaxedGlobalI32AtomicAdd` plus `serializeNVVMIR20AssemblyWithDiagnostics`. The terminal
  prefix grows from 312 to 328 bytes on x64 and 176 to 184 bytes on x86; the provider owns AS1
  typed i32, alignment four, LLVM `monotonic`, and default/System spelling.
- [x] (2026-08-27) Implemented the provider/wrapper and direct preflight/emission paths. The Release
  LLVM 14 provider rebuilt successfully before host integration.
- [x] (2026-08-27) Rechecked the rebuilt provider binary boundary: exports remain exactly the V1
  and V2 getters; dependencies remain `KERNEL32.dll` plus delay-loaded `SHELL32.dll`/`ole32.dll`,
  with no process-visible LLVM DLL.
- [x] (2026-08-27) Built the Debug host and ran the focused suite with the real provider. All
  provider, negotiation, fake-direct, and prior-slice lanes passed, but the three new real direct
  lanes stopped while libNVVM loaded the LLVM 14 bitcode. The exact diagnostic is
  `parse Invalid value (Producer: 'LLVM14.0.6' Reader: 'LLVM 7.0.1')`.
- [x] (2026-08-27) Isolated the incompatibility to LLVM 14's atomic bitcode record. The LLVM 14
  writer emits current `FUNC_CODE_INST_ATOMICRMW` record 59 with an explicit value type and
  alignment; the LLVM 7 reader used by CUDA 12.9 understands the legacy record 38 instead.
- [x] (2026-08-27) Confirmed with a direct libNVVM probe that LLVM-7-dialect textual
  `atomicrmw ... monotonic` without an alignment suffix verifies and compiles, while LLVM 14's
  printed `, align 4` suffix is rejected. Ordinary named SSA values are accepted, while explicit
  numeric parameter declarations are not.
- [x] (2026-08-27) Rejected a provider-side inline-PTX workaround. LLVM 14 encodes InlineAsm with a
  newer constant record too; the real compile appeared successful but silently emitted an empty
  kernel, including for the fixture that stores the returned old value. The probe was reverted.
- [x] (2026-08-27) Chose the Numba-supported text bridge as the bounded experimental architecture:
  the LLVM 14 provider owns the exact LLVM-7/NVVM-2.0 dialect adapter, while production-compatible
  bitcode remains a later writer experiment rather than a prerequisite for this semantic slice.
- [x] (2026-08-27) Preserved old-provider compatibility with explicit negotiation. Slice 17 and
  older V2 providers still receive bitcode; only a complete Slice 19 provider receives audited
  NVVM IR 2.0 assembly. Existing generic assembly serialization still returns raw LLVM 14 text.
- [x] (2026-08-27) Named function parameters at construction and removed only the LLVM-14-only
  natural `atomicrmw` alignment suffix after semantic validation and a one-to-one rewrite count.
  The normalized module verifies and compiles through CUDA 12.9 libNVVM.
- [x] (2026-08-27) Implemented provider, host wrapper, direct preflight/emission, fake/real/
  differential/negative tests, and durable design evidence.
- [x] (2026-08-27) Rebuilt the Release provider and host outside the sandbox. The complete NVVM
  prefix passed after repairing Release-only fake-test assertions whose side effects had been
  compiled out. Differential PTX, same-root `ptxas`, and GPU runtime lanes all passed.

## Surprises and Discoveries

- Observation: `InterlockedAdd` and `Atomic<T>.add` share the canonical `kIROp_AtomicAdd` producer,
  whose result is the original stored value. The HLSL-style no-result overload discards that value
  but retains the atomic side effect.
  Evidence: `hlsl.meta.slang` routes `InterlockedAdd` through `__atomic_add`; `core.meta.slang`
  declares `Atomic<T>.add` with the same intrinsic op and documents device scope.
  Consequence: preflight must validate the producer's exact pointer, value, result, and order shape;
  emission must map the returned old value even when the source does not consume it.

- Observation: a multi-thread increment of one device integer gives a strong executable atomicity
  proof without first adding lane/thread identifiers.
  Evidence: every launched lane can add the constant one to the same raw device pointer; the final
  value must equal the launched lane count, while a non-atomic store/add sequence can lose updates.
  Consequence: keep thread builtins and wave operations outside this slice.

- Observation: after linking and force inlining, the chosen source has exactly
  `atomicAdd(destination, 1 : Int, 0 : Int) : Int`; `destination` is the established read-write
  device-pointer launch parameter, and order zero is Relaxed. The old-value result is dead in the
  no-result overload but remains the instruction result.
  Evidence: the final `-dump-ir` stage immediately before direct emission retained that exact
  three-operand instruction. The pre-change direct route then reported E52017 for `atomicAdd`.
  Consequence: the order operand is semantic policy and is validated as a literal; it is not
  lowered as ordinary SSA. The provider must still return and map the old value.

- Observation: NVRTC emits `atom.global.add.u32` for the same source, with a 64-bit launch
  parameter, `ld.param.u64`, and `cvta.to.global.u64`. PTX's omitted semantic and scope qualifiers
  mean relaxed and GPU scope, matching CUDA's unsuffixed `atomicAdd` device-scope contract.
  Evidence: CUDA 12.9 NVRTC produced the instruction directly; NVIDIA's CUDA Programming Guide
  states that unsuffixed legacy atomics are relaxed and device scoped, and PTX documents omitted
  qualifiers as relaxed/GPU.
  Consequence: direct PTX must preserve that token-safe class; a system-scope or non-atomic
  load/add/store sequence is not equivalent.

- Observation: the first canonical `atomicrmw` disproves the Slice 2 minimal-module assumption that
  LLVM 14 bitcode is generally consumable by libNVVM's LLVM 7 reader.
  Evidence: LLVM 14 verification and both assembly/bitcode materialization pass, but CUDA 12.9
  rejects the bitcode during `nvvmAddModuleToProgram` with producer LLVM 14.0.6 / reader LLVM 7.0.1.
  LLVM 14 writes atomic record 59; LLVM 7 expects legacy record 38. Earlier operations happened to
  use backward-readable records and therefore did not establish whole-dialect compatibility.
  Consequence: fix the writer/provider boundary. The emitter negotiates a declared wire dialect;
  it does not inspect content, retry after libNVVM failure, or replace this supported NVVM
  instruction with inline PTX.

- Observation: the LLVM 7 textual reader is a viable bounded compatibility bridge when its dialect
  is owned and negotiated by the provider.
  Evidence: libNVVM verifies and compiles `atomicrmw add ... monotonic` with its natural default
  alignment; adding LLVM 14's `, align 4` suffix reaches `parse expected metadata after comma`.
  Explicit LLVM 14 numeric parameter declarations fail too, while stable named parameters and
  implicit local numbering succeed. Numba independently uses textual LLVM IR with libNVVM and
  performs version-specific normalization at that boundary.
  Consequence: expose a distinct NVVM-2.0-assembly format and provider callable, preserve raw LLVM
  14 assembly and old-provider bitcode behavior, and keep a production bitcode writer as a later
  replaceable implementation behind the same builder boundary.

- Observation: text serialization has no demonstrated material cost at the current boundary.
  Evidence: warmed synthetic modules with 1, 100, and 500 empty kernels spent approximately
  3.1/39/200 ms end-to-end through text and 3.1/39/268 ms through LLVM 14 bitcode on this machine;
  serialization itself remained below 1.5 ms for text and 2.8 ms for bitcode. Compilation dominated
  both paths, and the 500-kernel difference is treated as reader-path variability rather than a
  performance claim.
  Consequence: correctness and dialect ownership decide this experiment. Keep the wire format
  negotiated so a future compatible bitcode writer can replace text without changing IR lowering.

## Decision Log

- Decision: Slice 19 implements only relaxed signed-i32 atomic add on the existing read-write
  device `Ptr<int>` entry-point ABI.
  Rationale: this is one canonical producer, one provider mutation, and one directly executable
  semantic contract. Bundling other operations, memory orders, address spaces, types, or waves
  would hide independent LLVM/NVVM policy decisions.
  Date/Author: 2026-08-27, Codex.

- Decision: append a dedicated terminal V2 builder operation rather than widening an existing
  arithmetic callable.
  Rationale: prior callable domains are frozen, and atomicity/order/scope are distinct from ordinary
  integer addition. The dedicated field provides one atomic negotiated capability and preserves
  short/future provider compatibility.
  Date/Author: 2026-08-27, Codex.

- Decision: the appended operation is the exact
  `emitRelaxedGlobalI32AtomicAdd(module, pointer, value, outOldValue)` callable. It accepts only an
  LLVM AS1 typed i32 pointer and matching i32 value, and owns alignment four plus Relaxed ordering.
  Rationale: this slice has only one valid order, alignment, address space, and width. Exposing
  configurable ABI parameters would create unneeded provider states and tests without accepting
  another canonical Slang shape. A later semantic expansion can append its own coherent callable.
  Date/Author: 2026-08-27, Codex.

- Decision: map the accepted ABI Relaxed value to LLVM 14 `AtomicOrdering::Monotonic` and emit an
  ordinary `atomicrmw add` without a target-specific LLVM sync-scope spelling.
  Rationale: LLVM defines `monotonic` as the IR representation of relaxed read-modify-write
  ordering. NVVM's supported `atomicrmw` form and libNVVM lower it to the unsuffixed PTX atomic,
  whose omitted qualifiers are relaxed/GPU scope and therefore match CUDA `atomicAdd`. Inventing a
  sync-scope string not specified by the pinned NVVM dialect would weaken compatibility.
  Date/Author: 2026-08-27, Codex.

- Decision: append `serializeNVVMIR20AssemblyWithDiagnostics` beside atomic add and treat both
  pointers as one Slice 19 capability block.
  Rationale: the atomic operation cannot cross this libNVVM boundary through LLVM 14 bitcode.
  Explicit negotiation lets current providers supply an audited compatible dialect without
  changing generic assembly semantics. Older providers remain valid and receive bitcode; partial
  or null two-field suffixes are rejected before use.
  Date/Author: 2026-08-27, Codex.

- Decision: the compatibility writer owns exactly two proven conversions: stable parameter naming
  at LLVM construction and removal of natural `atomicrmw i32` alignment when printing NVVM IR 2.0.
  Rationale: semantic LLVM inspection proves every atomic is the one supported shape, and the
  rewritten-line count must equal the semantic atomic count. This keeps normalization at the
  dialect-producing boundary and prevents a general text search from silently accepting a future
  unsupported atomic form.
  Date/Author: 2026-08-27, Codex.

- Decision: do not land inline PTX, content-sniffing fallback, or retry-after-libNVVM-failure.
  Rationale: NVVM supports canonical integer `atomicrmw`; inline assembly was silently lost by the
  older reader. The host chooses bitcode or audited text from the provider capability before
  serialization, so failure remains deterministic and no malformed module is retried.
  Date/Author: 2026-08-27, Codex.

## Outcomes and Retrospective

Slice 19 now accepts exact canonical Relaxed signed-i32 atomic add and preserves its returned old
value. The complete x64 provider prefix is 328 bytes: a 312-byte Slice 17 provider stays usable and
receives bitcode, while every size 313 through 327 is malformed because atomic emission and its
NVVM-2.0 text writer are one capability. Full tables require both pointers; future tables are
clamped. The x86 equivalents are 176 and 184 bytes.

The input-shape audit keeps each fix at its producer boundary. `kIROp_AtomicAdd` is canonical linked
Slang IR produced by `__atomic_add`, so preflight owns the signed-i32/read-write-device-pointer/
Relaxed policy. LLVM 14's explicit numeric parameter spelling and atomic alignment suffix are
accidental alternative textual spellings for libNVVM's LLVM 7 reader. The provider therefore gives
parameters stable names as they are declared and, only in the negotiated NVVM-2.0 writer,
semantically validates every atomic before removing its exact natural alignment suffix. Raw
assembly and LLVM objects remain unchanged; the rewrite count must match the semantic atomic count.
No syntax is rebuilt from Slang values, no arbitrary operand graph is walked, and no downstream
retry masks malformed input.

CUDA 12.9 libNVVM verifies and compiles the normalized text to token-safe
`atom.global.add.u32`. Differential PTX, same-toolkit `ptxas`, and RTX 5090 multi-thread runtime
lanes pass for both discarded-result and returned-old-value cases. The runtime launches 2,048
threads adding one to a shared initialized integer and requires the final value to equal the launch
width on both direct NVVM and NVRTC routes. The Release suite initially exposed four fake callbacks
that populated records only inside `SLANG_ASSERT`; replacing those side effects with ordinary
validation made the tests meaningful under `NDEBUG` without changing production behavior.

The final Release focused suite passed 140/140. Preservation passed 1/1 parser, 2/2 routing/hash,
1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.
The rebuilt provider still exports exactly `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`; its only ordinary dependency is `KERNEL32.dll`, with delay-loaded
`SHELL32.dll` and `ole32.dll`, so no LLVM DLL enters the process boundary. Record the final commit
subject as `slice 19`; report the resulting hash at handoff because a commit cannot contain its own
stable hash.

## Context and Current Pipeline

Slices 7–17 established a raw CUDA entry ABI of signed `i32` and read/read-write device pointers,
complete scalar CFG/SSA/direct calls, pointer and fixed-array addressing, and a small signed-i32
operation set. Slice 18 added downstream libdevice and floating-policy handling without changing the
direct builder ABI. Before this slice, `validateNVVMSupportedIR` rejected every atomic instruction
and the private V2 table ended at `emitIntegerNegate`.

The CUDA source emitter already maps `kIROp_AtomicAdd` to CUDA `atomicAdd`, but that target-specific
source representation is not reusable by direct NVVM. The direct route must preserve the canonical
Slang IR producer and choose an LLVM/NVVM atomic representation through its provider API.

## Scope and Non-Goals

In scope:

- exact canonical `kIROp_AtomicAdd` with signed-i32 result/value;
- the established read-write device `Ptr<int>` entry parameter as the atomic destination;
- exactly the measured relaxed memory-order operand and device/system scope required by the
  canonical producer;
- one append-only two-field provider capability with complete short/partial/null/future table
  negotiation;
- provider validation before the sole atomic mutation, including type, pointer address space,
  module/function/insertion point, dominance, order, alignment, and output sanitization;
- fake graph/order/count evidence, real LLVM verification, differential PTX, matching-root
  `ptxas`, and multi-thread GPU runtime comparison with NVRTC;
- deterministic negative boundaries for adjacent atomic operations/types/orders/pointer shapes;
- preservation of all Slice 18 and earlier tests.

Out of scope:

- atomic load/store, subtract, min/max, exchange, compare-exchange, AND/OR/XOR, inc/dec, or reduce;
- unsigned, narrow, wide, floating-point, vector, matrix, aggregate, or resource atomics;
- read-only pointers, local/shared/constant/generic storage, and address-space casts; this slice
  adds no new pointer producers, but an already-supported canonical read-write device-i32 pointer
  remains valid regardless of whether it is a raw parameter or a derived pointer;
- Acquire, Release, AcquireRelease, or SeqCst policy;
- fences, barriers, volatile, weak compare-exchange, failure ordering, or configurable alignment;
- lane/thread builtins, wave/subgroup operations, convergence tokens, masks, or reconvergence;
- general textual-LLVM construction or parsing, Slang producer rewriting, syntax reconstruction,
  source-name matching, content-sniffing fallback, or retry after libNVVM failure.

## Architecture and Invariants

The final linked Slang IR is the source of truth. Preflight accepts the exact atomic op only when
its result and value are signed `i32`, its destination is the established canonical read-write
device-i32 pointer, and its memory-order operand is the measured relaxed constant. No emitter-side
fallback may reinterpret an ordinary add/store sequence as atomic or infer semantics from an
intrinsic name.

The provider operation receives already-created LLVM handles, validates the module, insertion
block, pointer/value/result types, ownership, availability/dominance, and the frozen order/scope
before mutation, clears its output on entry and failure, and performs one LLVM atomic construction.
The provider does not expose LLVM objects or enums across the ABI.

Provider table negotiation remains append-only. Every prior exact prefix stays usable for its
established programs and continues to serialize bitcode. Slice 19 appends the atomic operation and
its NVVM-2.0 text serializer as one coherent block; sizes inside either pointer and either null field
are malformed. Future-larger tables are accepted and clamped. A program that needs atomic add but
receives the exact Slice 17 provider prefix reaches E52016 after discovery and before module
creation. Programs accepted by that older provider still compile through the bitcode path.

## Plan of Work

First, probe the ordinary Slang source through final linked IR and explicit NVRTC. Record exact
`kIROp_AtomicAdd` operands, result uses, memory-order constant, destination type/access/address
space, PTX instruction/scope/semantics, and pre-change E52017 label. Confirm the multi-thread launch
can use the existing one-pointer ABI.

Second, define one terminal V2 provider operation and host wrapper/capability identity. Add fake
negotiation/forwarding/output-sanitization tests and a real provider negative matrix before changing
the direct emitter.

Third, add the exact preflight classification and emission mapping in `slang-emit-nvvm.cpp`.
Require the new capability only after the full input shape is valid. Add fake direct graph evidence,
old-provider E52016 negotiation, and adjacent negative boundaries before builder discovery.

Fourth, add real direct/NVRTC PTX classification, matching-root `ptxas`, and runtime tests. Launch a
bounded number of lanes that all add one to one initialized device integer and require the final
value on both routes to equal the lane count.

Finally, run pinned clang-format 17, rebuild the Release LLVM 14 provider and Release host outside the
sandbox, run the complete focused NVVM prefix and established preservation matrix, inspect the diff
and provider binary, update durable design/ledger evidence, remove probes, stage only intended
tracked files, and commit exactly `slice 19`.

## Concrete Steps and Validation

Run from `C:\src\slang` with Windows-native tools. All CMake builds and tests run outside the
sandbox as required by `AGENTS.md`.

    cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm -- /m
    cmake.exe --build build --config Release --target slang-unit-test slang-test -- /m
    build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm

Run the established preservation matrix:

    build\Release\bin\slang-test.exe slang-unit-test-tool/parseCUDAEmissionMethods
    build\Release\bin\slang-test.exe slang-unit-test-tool/cudaEmissionMethod
    build\Release\bin\slang-test.exe tests/cuda/nvvm-unsupported-ir
    build\Release\bin\slang-test.exe tests/cuda/sampler-comparison-state-unused
    build\Release\bin\slang-test.exe tests/cuda/cuda-compile
    build\Release\bin\slang-test.exe slang-unit-test-tool/coverageCudaRuntimeDispatch

Acceptance requires:

- measured canonical IR and pre-change negative boundary;
- exact old/new V2 layout, two-field capability negotiation, and old-provider bitcode fallback;
- provider validation before mutation and LLVM verifier-valid atomic IR;
- fake direct graph proving one atomic emission and no fallback;
- direct/NVRTC PTX with matching global signed-i32 atomic-add semantics;
- same-root `ptxas` acceptance and multi-thread GPU result equal to launch width on both routes;
- adjacent unsupported atomics/types/orders reject before provider discovery;
- focused and preservation matrices green after formatting;
- unchanged V1/V2 exports, no process-visible LLVM dependency, and no unprincipled producer repair,
  content sniffing, syntax reconstruction, source-name matching, or failure-driven fallback.

## Idempotence and Recovery

Probes, builds, fake compiles, real compiles, `ptxas`, and bounded runtime launches are safe to
repeat. Generated probe files remain untracked and are removed before commit. Provider validation
must happen before its sole mutation so failed cases leave no partial atomic instruction. Existing
RAII owns modules, programs, CUDA allocations, and contexts.

Do not delete/reset user work or stage `external/slang-binaries/`. Remove only Slice 19 probes and
the temporary text-audit files before committing. Keep this completed Slice 19 plan with the slice
implementation as explicitly requested by the user.

## Artifacts and Hand-Off

Retain final evidence and the input-shape audit here. Distill stable architecture into
`docs/design/nvvm-backend.md`, durable results into
`docs/design/nvvm-backend-capability-ledger.md`, and the eventual five-part PR narrative.
