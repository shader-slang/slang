# Slice 144: Establish one common relaxed scalar atomic algebra

## Motivation

The Slice 143 census left a coherent set of ordinary compute workloads at canonical atomic
instructions. Consider the operations in `language-feature/atomic-t/atomic-0.slang`:

```slang
RWStructuredBuffer<Atomic<int>> outputBuffer;

int old = outputBuffer[0].compareExchange(4, 5);
int now = outputBuffer[0].load();
outputBuffer[0].store(now + old);
```

Resource legalization already produced a typed writable structured-buffer element pointer.
Intrinsic lowering already represented compare-exchange, load, and store as distinct atomic IR
operations with explicit memory-order operands. The direct backend nevertheless accepted only a
narrow add/max subset, while the provider callback could express only a two-operand
read-modify-write. Similar first failures blocked shared HLSL atomics, signed and 64-bit byte-
address atomics, Float32 bitwise exchange/CAS, and the `Atomic<T>` method family.

These were not eleven fixture-specific gaps. They were one missing scalar atomic algebra over
established global/shared pointer producers. The slice therefore targets the representation once,
then measures every corpus workload it unlocks.

## Proposed solution

One compiler-owned resolver now classifies load, store, exchange, compare-exchange, add, subtract,
min, max, bitwise and/or/xor, increment, and decrement. It validates the exact canonical pointer,
physical scalar type, operand/result relation, and relaxed order literals. The same resolved
descriptor drives capability collection, SSA/dominance validation, and emission.

Provider ABI revision 30 replaces the fixed pointer/value atomic call with one descriptor plus an
SSA operand array. Load has one pointer operand, store and RMW have pointer/value, and compare-
exchange has pointer/compare/replacement. Subtract composes typed negation with add; increment and
decrement compose typed constants with add. No per-intrinsic provider callbacks are added.

The LLVM provider emits monotonic atomic RMW and compare-exchange operations. libNVVM's NVVM IR
2.0 reader rejects LLVM textual atomic load/store instructions, so relaxed load uses the standard
value-preserving compare-exchange-of-zero-with-zero idiom and store uses exchange while discarding
the old SSA value. Float32 exchange/CAS/load bit-transport through same-width integer pointers.
The isolated serializer validates those exact operations and removes LLVM 14's cmpxchg/atomicrmw
alignment suffix for the LLVM-7-era reader.

## Change summary

- The builder API, wrapper, semantic catalog, fake provider, and LLVM provider move forward
  together to ABI revision 30 and one generic scalar atomic call.
- `slang-emit-nvvm.cpp` shares one atomic resolver across preflight and emission, composes
  subtract/inc/dec through existing typed operations, and admits only exact global/shared
  producers and relaxed order literals.
- `Atomic<T>` becomes a physical `T` storage leaf. Byte-address legalization's equivalent
  structured view admits selected signed/unsigned 32/64-bit and Float32 scalar leaves.
- Shared scalar/array lowering handles selected wide/Float32/atomic storage, canonical whole-array
  initialization stores, and deterministic physical names for anonymous synthesized shared
  globals.
- Provider and fake-emitter tests cover the complete operation algebra and negative contracts.
  Eleven existing workloads gain O0/O3 runtime lanes.
- The fixed 452-row census, Pareto clusters, representative performance/assembly evidence, plan,
  this report, and durable design documents record the measured outcome.

## Concepts and vocabulary

- **Atomic descriptor**: operation, semantic scalar type, physical address space, success order,
  and failure order; runtime SSA values are separate operands.
- **Physical atomic leaf**: the provider representation of `Atomic<T>` storage, which is exactly
  `T`; the wrapper remains the canonical Slang semantic marker.
- **Equivalent structured view**: the typed structured-buffer producer created by byte-address
  legalization after it has already converted a byte offset to an element index.
- **Legacy serialization boundary**: the isolated LLVM 14 provider's validated conversion to the
  LLVM-7-era textual dialect accepted by libNVVM.
- **Healthy MVP reference**: one of the 427 MVP workloads whose native CUDA/NVRTC O3 lane is
  correct.

## Process report

### The canonical IR was already correct

For `RWStructuredBuffer<Atomic<int>>`, structured-buffer lowering produces
`rwstructuredBufferGetElementPtr` with an `Atomic<int>` pointee. Core intrinsic lowering then
produces `atomicCompareExchange`, `atomicLoad`, or `atomicStore` with the physical `int` values and
literal order operands. For HLSL byte-address methods, `slang-ir-byte-address-legalize.cpp`
produces `getEquivalentStructuredBuffer` followed by the same typed element-pointer instruction.
For groupshared arrays, the pointer is `getElementPtr` rooted at the canonical module-scope shared
global.

Those shapes are intentional upstream representations:

1. The resource/shared producer establishes address space and writable storage.
2. Byte-address legalization owns byte-to-element conversion before direct emission.
3. The atomic instruction remains the semantic source of truth for operation, values, result, and
   order.
4. `Atomic<T>` marks legal atomic access but adds no physical field around `T`.
5. The eleven promoted workloads cover structured, raw, shared, signed, unsigned, wide, Float32,
   load/store, CAS, exchange, RMW, and composed operations.

The fix therefore belongs in direct-NVVM classification/type lowering and the provider's concrete
atomic construction. It does not reconstruct source syntax or patch malformed IR downstream.

### One resolver owns the complete operation contract

`_resolveNVVMAtomicOperation` maps each admitted IR opcode to one descriptor and zero, one, or two
value operands. It peels an `Atomic<T>` pointee only at the physical storage boundary, requires
exact operand and result types, and accepts only executable literal `MemoryOrder::Relaxed` values.
Store must return Void; every other admitted operation returns the original `T`. Compare-exchange
requires both success and failure orders to be relaxed.

`_validateNVVMFunction`, `_validatePointerValue`, requirement collection, dominance validation,
and final emission all consume this resolver. There is no parallel operation list or fallback.
An acquire-order negative still fails with the exact canonical opcode before provider discovery.

Subtract becomes add of typed negation. Increment/decrement become add of typed `1`/`-1`. These
are exact modular integer operations and use the generic value callback already required by the
backend. The provider operation surface therefore grows only for memory semantics that cannot be
expressed by ordinary generic value construction.

### ABI revision 30 is one economical memory interface

`SlangNVVMAtomicOperationDesc` carries the complete typed semantic key. The operand array contains
only provider SSA handles. The wrapper and provider validate operand count, pointer address space,
pointee/value type, ownership, dominance, and result presence before mutation. Store is the only
Void form and must return a null handle.

The fake provider records per-operation operand offsets/counts. The focused emitter graph compiles
one `groupshared Atomic<int>` program containing all 13 source operations; subtract, increment,
and decrement intentionally appear as four provider adds in total. The test proves every operation
uses one shared typed interface and the canonical shared-global pointer producer.

### The provider boundary handles libNVVM's actual dialect

LLVM 14 can construct `load atomic`, `store atomic`, `atomicrmw`, and `cmpxchg`, but the installed
libNVVM NVVM IR 2.0 reader rejects atomic load/store text. Removing only explicit alignment does
not fix it. The retained lowering is:

- relaxed load: monotonic cmpxchg with zero as both compare and replacement, returning the old
  value;
- relaxed store: monotonic atomic exchange, discarding the old value;
- exchange and integer reductions: typed atomicrmw;
- compare-exchange: typed cmpxchg, returning its old-value projection;
- Float32 bitwise load/exchange/CAS: identical same-width integer pointer/value transport;
- Float32/Float64 global add: the established legacy NVVM atomic-add intrinsic translation.

The serializer rejects any raw LLVM atomic load/store instruction. It validates only natural
alignment, system sync scope, monotonic order, selected global/shared address spaces, exact
integer cmpxchg, and the selected atomicrmw family before removing LLVM 14 alignment suffixes.
Focused builder serialization and all eleven real differential workloads pass libNVVM verification,
compilation, launch, and output comparison.

### The storage widenings are tied to exact producers

The selected corpus exposed three adjacent storage shapes:

- `Atomic<T>` inside structured buffers and shared globals is physically `T`.
- HLSL 64-bit/Float32 byte-address atomics use signed/unsigned/Float32 equivalent structured views
  produced by byte-address legalization.
- HLSL groupshared array initialization remains one whole-array store before element atomics.

The direct type classifier admits the first two finite leaf families. Whole-array pointer
validation is narrower: only the canonical initialization store may consume the global pointer;
element access continues through the established element pointer. An intermediate broader
admission was removed during self-review.

A function-local `static groupshared Atomic<int>` is synthesized as an anonymous module global.
Because it has no source mangling, `_getNVVMSharedGlobalName` derives a stable target-private symbol
from its order among admitted shared producers. Existing collision validation remains authoritative.
This is emission identity for a valid canonical global, not source-name dispatch.

### Fixed-denominator coverage and Pareto result

The corpus remains 452 eligible workloads from 448 sources: 430 MVP and 22 extension workloads.
Native CUDA/NVRTC O3 is correct for 449; three rows remain infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 357 | 8 | 80 | 7 | 365 |
| Direct O3 | 362 | 8 | 80 | 2 | 370 |

Both direct modes gain eleven exact workload identities and lose none from Slice 143. Against 427
healthy MVP references, O0 correctness is 355/427 (83.1%), O3 correctness is 359/427 (84.1%), and
both-mode correctness is 355/427 (83.1%). The selected 422/422 result remains a regression score,
not the coverage denominator.

The leading healthy-MVP failure clusters are:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Residual target marker / undefined value | 10 | 10 |
| Aggregate / pointer / layout transport | 8 | 8 |
| Helper ABI / type contract | 8 | 8 |
| Preflight other | 8 | 8 |
| Wave / reconvergence GenericAsm | 8 | 8 |
| Function identity | 6 | 6 |
| Remaining atomic opcode | 3 | 3 |
| Atomic GenericAsm | 3 | 3 |
| Raw-buffer view access | 3 | 3 |

O0 additionally has four healthy unoptimized-Half provider failures. The common atomic family is
no longer a leading cluster; next-slice selection should first decompose the ten residual-marker
rows and the tied eight-row semantic populations by exact producer.

### Representative workload and productionization evidence

The three release-gate workloads remain correct. Median standalone compile time across three runs
is 380/365/367 ms for NVRTC O3 and 265/247/249 ms for direct O3 SM70. Direct O3 PTX is
919/793/1404 bytes versus native PTX at 8889/8839/9190 bytes. These are backend artifacts, not a
claim of equal optimization pipelines.

The census end-to-end compile/load/launch/compare samples are respectively 4284/4236/4516 ms for
NVRTC and 4309/4356/4280 ms for direct O3. They are deliberately not labeled kernel-only runtime.
CUDA 12.9 `ptxas` accepts every direct O3 gate for SM70, SM80, and SM90. Runtime differential
execution occurred on an SM120 RTX 5090 with driver 610.62. CUDA 13 validation and physical
SM70/SM80/SM90 runtime workers remain explicit productionization gaps rather than inferred
coverage.

### Final self-review inventory

Retained changes all have a measured owner: the unified resolver, `Atomic<T>` physical leaf,
selected equivalent raw views, store-only shared-array initialization, deterministic anonymous
shared naming, provider CAS/exchange lowering, floating bit transport, and strict cmpxchg dialect
rewrite. Removing any of these reproduces a focused contract failure or one or more of the eleven
promoted workload failures.

The review removed broader shared-array consumers and a serializer acceptance for floating
cmpxchg that the provider never produces. No new syntax reconstruction, fixture-name check,
compatibility fallback, arbitrary operand walk, or downstream malformed-representation repair
remains.
