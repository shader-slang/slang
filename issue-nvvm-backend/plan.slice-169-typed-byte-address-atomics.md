# Generalize typed byte-address memory and common atomic roots

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM supports the canonical 16-bit typed byte-address load/store family
and the remaining common scalar atomic forms whose storage is rooted in a byte-address buffer or a
selected Half reference. One producer-based representation carries byte offsets to exact typed
global pointers; existing generic load/store and atomic builder operations consume those pointers.

The bounded probe is six healthy frozen-v1 workloads: `byte-address-16bit`,
`byte-address-16bit-vector`, `byte-address-half-atomics`, `atomic-reduce-half-cuda`,
`atomic-float-byte-address-buffer`, and `cas-int64-byte-address-buffer`. Promote only exact O0/O3
differential successes. A later unrelated operation or a libNVVM limitation remains measured
rather than broadening the slice.

## Progress

- [x] (2026-09-01) Completed and committed Slice 168 as `5098921e5`; frozen v1 remains
  402/402/402 and discovery advances to 68/68/68 O0/O3/both.
- [x] (2026-09-01) Re-ranked the remaining 25 healthy frozen and four healthy discovery failures.
  Selected the six-row byte-address/atomic vertical because one typed storage root may unlock two
  tied three-row clusters and exercises ordinary loads, stores, CAS, and floating add.
- [x] (2026-09-01) Traced the first final-IR producers: aggregate byte-address legalization emits
  scalar Int16 loads/stores; Half atomics emit an equivalent `RWStructuredBuffer<Atomic<Half>>`;
  the Float32-add and UInt64-CAS methods remain exact complete GenericAsm helper bodies.
- [x] (2026-09-01) Proved the typed-pointer and atomic contracts. Omitted byte-address alignment is
  `min(4, natural alignment)`; scalar Half global add uses exact typed PTX inline assembly in the
  provider; Float32 add and UInt64 CAS reuse the existing byte-offset and atomic operations.
- [x] (2026-09-01) Promoted five stable O0/O3 successes, regenerated both exact corpora and three
  bounded measurements, documented the invariants, and completed the special-case audit.

## Surprises and Discoveries

- Aggregate `RWByteAddressBuffer.Load<Data>` is correctly decomposed by
  `slang-ir-byte-address-legalize.cpp` before direct emission. For `Data { int16_t a; int16_t b; }`,
  the final canonical operations are scalar Int16 loads/stores at byte offsets zero and two. The
  direct classifier's older 32/64-bit leaf subset is the only first stop.
- The alignment operand on those final stores is zero and loads omit it. The current resolver maps
  zero/omitted to four because all previously selected leaves were naturally at least four-byte
  aligned. Widening to 16-bit values must derive the default from the selected physical value type;
  claiming alignment four at offset two would encode invalid LLVM alignment.
- `byte-address-half-atomics` does not use an ad-hoc raw pointer. Byte-address legalization creates
  `getEquivalentStructuredBuffer` with element `Atomic<Half>`, then the established structured
  element-pointer and relaxed atomic-add flow. The existing equivalent-view classifier stops at
  signed/unsigned 32/64-bit and Float32 leaves.
- The shared atomic semantic catalog accepts global Float32/Float64 add but intentionally excludes
  Half. LLVM 14 can construct scalar Half `atomicrmw fadd`; libNVVM acceptance and SM70 PTX
  assembly must be proved before that descriptor is retained.
- `InterlockedAddF32` and `InterlockedCompareExchangeU64` remain exact one-instruction GenericAsm
  helpers from `hlsl.meta.slang`, including a byte-address buffer, byte offset, scalar operands, and
  one typed out parameter. The existing builder already supports byte-offset pointer construction
  and the corresponding F32-add/U64-CAS descriptors; compiler-side exact helper classification is
  the apparent missing bridge.
- The first selected-prefix run exposed three fake-provider alignment regressions. Existing wide,
  vector, and array accesses prove that an omitted alignment is capped at four bytes, while the
  new Int16 accesses require reduction to their two-byte physical alignment. The shared rule is
  therefore `min(4, natural alignment)`, not unrestricted natural alignment.
- Scalar Half atomic add verifies, compiles through CUDA 12.9 libNVVM, assembles for SM70/80/90,
  and matches the native runtime. The same bounded reduction fixture next stops at the independent
  vector-Half GenericAsm signature, which remains outside this slice.

## Decision Log

- Decision: treat direct 16-bit memory and selected raw-buffer atomics as one typed byte-address
  vertical, but keep core IR and GenericAsm producer classification distinct.
  Rationale: both consume the same raw `{data, count}` view and typed byte-offset pointer invariant,
  while their canonical semantic producers remain different and must retain exact validation.
  Date/author: 2026-09-01, Codex.
- Decision: derive zero/omitted byte-address alignment as `min(4, natural alignment)`.
  Rationale: byte-address legalization guarantees the established four-byte boundary for wide and
  aggregate payloads, but a narrow physical leaf cannot promise more than its own alignment. This
  preserves all old contracts and correctly represents canonical Int16/Half offsets.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32 unless a concrete canonical operation cannot be encoded.
  Rationale: generic typed load/store, byte-offset pointer, and atomic descriptor operations already
  carry the needed semantics. Half-add support, if valid, is a new descriptor overload rather than
  a new callback shape.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Five of the six bounded healthy workloads are correct through native reference, direct O0, and
direct O3 and now have permanent lanes. `atomic-reduce-half-cuda` deliberately remains rejected at
its next exact `RefParam<vector<half,2>>` GenericAsm reduction shape.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and advances from
402/402/402 to 407/407/407 O0/O3/both, with exactly five gains and no old-correct regression.
Discovery remains exactly 82 workloads/72 healthy references at 68/68/68. The permanent NVVM
category passes 60/60. The final selected prefix passes 433/433 after the alignment invariant is
preserved.

Three representative gates produced 15 accepted PTX/cubin configurations. Direct O3 PTX assembled
with CUDA 12.9 for SM70, SM80, and SM90. Provider ABI revision 32 remains unchanged; every compiler
path uses existing generic type, aggregate, pointer, memory, and atomic operations.

## Context and Current Pipeline

Consider:

```slang
struct Data { int16_t a; int16_t b; }
Data value = input.Load<Data>(byteOffset);
output.Store<Data>(byteOffset, value);
```

Byte-address legalization recursively decomposes the aggregate into canonical
`byteAddressBufferLoad`/`byteAddressBufferStore` operations over Int16 at offsets separated by two
bytes. `_getNVVMByteAddressAccess` already validates the buffer, unsigned byte offset, optional
alignment, access mode, and value relation. Emission extracts the raw data pointer, applies
`emitByteOffsetPointer`, and calls typed load/store. The selected leaf classifier and default
alignment are narrower than this canonical producer.

For `InterlockedAddF16`, legalization uses `getEquivalentStructuredBuffer` and the established
structured element-pointer/atomic flow. For Float32 add and UInt64 CAS, the CUDA prelude retains an
exact GenericAsm helper. The helper's complete assembly and signature are semantic sources of
truth, as with existing atomic reductions, texture operations, and barriers.

## Scope and Non-Goals

In scope are selected scalar Int16/UInt16/Half and finite selected vectors for core byte-address
loads/stores; natural alignment; exact equivalent structured views for selected physical atomic
leaves; scalar global Half add subject to libNVVM proof; exact Float32-add and UInt64-CAS raw-buffer
GenericAsm helper contracts; existing typed builder operations; the six bounded workloads; both
fixed corpora; and representative assembly/measurement evidence.

Out of scope are ByteAddressBuffer aggregate ABI reconstruction, arbitrary `_getPtrAt<T>` parsing,
Float64 raw-buffer add unless a bounded workload requires it, vector atomics, Half exchange/CAS,
FP8/BFloat16, stronger memory orders, unsupported address spaces, unaligned values without a
producer contract, source/fixture-name checks, compatibility fallbacks, and provider callbacks
without a concrete operation gap.

## Architecture and Invariants

- Core byte-address access accepts a value type only when direct value lowering and natural
  size/alignment agree with the canonical legalized operation. Zero/omitted alignment means that
  natural alignment; an explicit nonzero power of two remains an exact producer promise.
- Equivalent structured views preserve the source access mode and use an exact selected physical
  scalar or `Atomic<T>` leaf. The raw byte count remains runtime data; only the data pointer is
  retargeted at byte offset zero.
- GenericAsm atomic helpers are accepted only by complete assembly, result, parameter count,
  parameter type, out-parameter, and single-body-instruction contracts. No substring parsing or
  source method recognition is permitted.
- One resolved raw-buffer atomic descriptor records operation, semantic type, byte offset,
  operands, output, and global address space. Requirement collection, value validation, and
  emission consume that same result.
- Every descriptor must already be accepted by the shared semantic catalog and provider discovery.
  Unsupported Half or dialect forms stay deterministic preflight/provider failures.

## Interfaces and Dependencies

Value/type classification lives in `source/slang/slang-emit-nvvm-type-lowering.cpp`. Core
byte-address, equivalent-view, GenericAsm helper, requirement, validation, and emission logic lives
in `source/slang/slang-emit-nvvm.cpp`. The shared atomic legality source is
`source/compiler-core/slang-nvvm-semantic-catalog.h`; LLVM construction and legacy textual
serialization live in `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`. Focused contract coverage
belongs in the existing split builder/emitter tests only when it proves a non-redundant provider or
classifier invariant. Real fixtures own differential combinations.

## Milestones

1. Preserve final IR and exact first diagnostics for all six workloads. Audit natural layouts,
   equivalent-view leaf types, GenericAsm signatures, and atomic descriptors.
2. Add the smallest shared classifications/resolvers and focused negatives. Probe Half atomic add
   through LLVM verification, libNVVM compilation, PTX assembly, and runtime before retaining it.
3. Carry each bounded workload to correct O0/O3 execution or record its next independent blocker.
   Promote only stable, deterministic both-mode successes.
4. Rebuild outside the sandbox; run focused, selected-prefix, permanent NVVM, both exact corpora,
   and bounded representative measurements; update design, ledger, report, and this plan.
5. Format, run `git diff --check`, complete the new-helper/special-case input-shape audit, stage
   exactly the slice files excluding `external/slang-binaries/`, and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools and the isolated
Release provider path. Acceptance requires provider/compiler builds; focused contract coverage;
correct O0/O3 differential execution for retained workloads; exact rejection of adjacent
unsupported type/order/pointer/helper shapes; no old-correct regression; frozen identity 452/427;
discovery identity 82/72; separate census/Pareto artifacts; selected-prefix and permanent category
success; bounded compile/PTX/runtime measurements; SM70/SM80/SM90 `ptxas`; formatting; artifact
integrity; and an exact staged-file audit.

## Failure and Recovery

Changes are additive and independently testable. If Half atomic add fails LLVM verification,
libNVVM compilation, PTX assembly, or differential execution, keep its prior deterministic stop
and retain no speculative semantic overload. If a GenericAsm helper exposes an unrelated later
shape, record it and narrow promotion. Never replace an atomic with non-atomic memory operations,
infer a pointer from arbitrary text, overstate alignment, or patch serialized IR without a typed
LLVM instruction that owns the semantics.

## Artifacts and Hand-Off

Keep IR, PTX, and logs under ignored `build/nvvm-census` paths. Retain the completed plan only if
the slice yields a committed result under the user's workflow exception. Distill durable typed raw
memory and atomic invariants into `docs/design/nvvm-backend.md`, exact status into the capability
ledger and separate corpus artifacts, and every producer/input-shape decision into a five-part
Slice 169 report.
