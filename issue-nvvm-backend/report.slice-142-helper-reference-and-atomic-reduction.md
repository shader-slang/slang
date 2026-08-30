# Slice 142: Generalize helper references and lower common atomic reductions

## Motivation

The Slice 141 healthy-MVP Pareto contained sixteen helper-ABI failures. Seven representative rows
shared pointer-bearing helper parameters, and five of those immediately entered CUDA atomic
reduction helpers. Consider:

```slang
RWStructuredBuffer<float> target;

[numthreads(64, 1, 1)]
void computeMain()
{
    __atomic_reduce_add(target[0], 0.5f);
}
```

`RWStructuredBufferGetElementPtr` produces a writable scalar-layout resource pointer. The linked
reduction helper accepts a generic default-layout `RefParam<float>` and consists of one exact
`IRGenericAsm` statement. The direct backend rejected the reference parameter before it could
classify the reduction. A groupshared fixture exposed the same missing representation at a
different address space: a scalar-layout array-element pointer had to cross an explicit
default-layout `Ptr<uint4, GroupShared>` helper parameter without losing address space three.

These are canonical linked pointer roles, not malformed alternatives. Treating every pointer as
either local or globally addressed would make one fixture compile while corrupting another. The
slice therefore needed a reusable helper-reference contract and a producer-owned physical
address-space boundary before atomic reduction could be lowered correctly.

## Proposed solution

The compiler now recognizes exact generic read-write `RefParam<T>` and read-only
`BorrowInParam<T>` helper parameters over finite selected pointees. Call compatibility compares
the canonical pointee and access contract. A resource/global producer is converted from provider
address space one to the helper's generic address space only at the call boundary. An admitted
atomic-reduction helper converts that generic parameter back to address space one after every call
has proven a global producer. Local reference arguments remain legal for ordinary helpers but are
rejected deterministically for atomic reductions.

Explicit read-write groupshared numeric helper pointers remain in provider address space three.
The existing shared-array and element-pointer representation is generalized from Int32/UInt32 to
the selected numeric value algebra, and ordinary generic pointer-offset, load, and store operations
carry the value through helper calls.

One compiler-owned atomic-reduction descriptor classifies nine exact CUDA assembly spellings plus
the complete linked signature. It maps relaxed global scalar integer add, min, max, and bitwise
reductions and Float32/Float64 add to the existing generic atomic callback. Subtraction negates its
typed value; increment and decrement construct typed constants. The returned old value is
discarded because the source reduction is `Void`.

LLVM 14 emits floating add as typed `atomicrmw fadd`. CUDA's LLVM-7-era NVVM reader requires the
legacy `llvm.nvvm.atomic.load.add` intrinsic. The isolated provider validates the typed operation,
then translates only the exact provider-produced global scalar Float32/Float64 lines at its
established legacy serialization boundary. Provider ABI revision 29 is unchanged.

## Change summary

- `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` adds exact helper-reference, groupshared
  helper-pointer, and selected atomic-storage classifiers and lowers each by physical role.
- `source/slang/slang-emit-nvvm.cpp` adds reference call relations, producer-proven address-space
  conversions, an exact atomic-reduction resolver, capability collection, validation, and typed
  emission.
- `source/compiler-core/slang-nvvm-semantic-catalog.h` admits the proven relaxed global scalar
  integer reduction family and Float32/Float64 atomic add.
- `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` constructs the corresponding typed LLVM atomic RMW
  operations and owns the two exact floating-add legacy serialization forms.
- Focused builder, fake-provider, real-provider, differential, negative, and `ptxas` tests prove
  the widened family and its adjacent exclusions.
- Three existing fixture files gain direct O0/O3 runtime lanes. The fixed census/Pareto artifacts,
  committed plan, this report, and durable design/capability documents record the measured result.

## Concepts and vocabulary

- **Helper reference**: an exact final `RefParam` or `BorrowInParam` pointer role in an `IRFunc`
  signature, distinct from the pointer role and layout metadata at its call site.
- **Physical producer**: the canonical instruction or parameter that determines whether a pointer
  is local, global, shared, or already generic after type lowering.
- **Atomic reduction**: a `Void` CUDA helper that performs a relaxed atomic read-modify-write and
  deliberately discards the old value.
- **Legacy serialization boundary**: the isolated provider step that converts LLVM 14 textual
  syntax into the LLVM-7-era NVVM IR dialect after semantics have been built and validated as LLVM
  objects.
- **Healthy MVP reference**: one of the 427 MVP workloads whose native CUDA/NVRTC O3 lane is
  correct.
- **Selected prefix**: focused direct-NVVM unit tests; a regression score, not the corpus coverage
  denominator.

## Process report

### Pointer roles and producers define the helper ABI

CUDA specialization intentionally preserves source reference semantics in final function types.
For an atomic reduction, the parameter is exactly:

```text
RefParam<T, ReadWrite, Generic, DefaultLayout>
```

The structured-buffer call argument is exactly:

```text
Ptr<T, ReadWrite, Generic, ScalarLayout>
```

Although both IR types use semantic `Generic`, `RWStructuredBufferGetElementPtr` is a canonical
global-memory producer. `asNVVMSupportedHelperReferencePointerType` recognizes only the complete
parameter contract. `_isSupportedNVVMHelperArgumentType` requires equal canonical pointees and
sufficient access. `_isNVVMGlobalHelperReferenceArgument` identifies only established global
producers. Call emission widens those arguments to the generic provider pointer type; no ordinary
resource load, store, or atomic loses address space one.

Inside the reduction helper, `_emitNVVMAtomicReduction` recovers the global pointer type before
invoking the atomic callback. `_validateNVVMFunction` performs the same producer proof at every
call. Removing that proof makes a local `uint` reduction silently use global-memory semantics, so
the adjacent negative source requires the exact `atomic reduction global reference` diagnostic.
This layer owns the check because final pointer type plus call producer is the complete canonical
information needed to choose the physical address space.

The groupshared shape is also intentional. `__getAddress(shared[0])` produces a scalar-layout
address-space-three element pointer; the helper signature carries a default-layout explicit
groupshared pointer to the same `uint4`. The call relation accepts only that exact role/pointee pair.
Type lowering preserves address space three on both sides, and the generic pointer-offset operation
handles `ptr[1]`. The focused `uint4` and Float32 shared-array sources, the promoted
`ptr-to-groupshared-1.slang`, and real PTX load/store checks prove this representation. Aggregate
groupshared pointees remain outside this slice.

### Complete helper contracts select atomic semantics

`StmtLoweringVisitor::visitIntrinsicAsmStmt` creates each final `IRGenericAsm`. The compiler does
not inspect a fixture path or source helper name. `_resolveNVVMAtomicReduction` requires:

- one block containing only the GenericAsm;
- a `Void` result;
- one of nine exact assembly strings;
- an exact mutable generic reference parameter;
- equal physical value and value-parameter types where applicable;
- an exact signed Int32 memory-order parameter; and
- a descriptor admitted by the shared semantic catalog.

Call validation additionally requires the executable order literal to be `Relaxed`. Half, vector,
wrong-order, wrong-reference, extra-instruction, and mismatched-pointee forms remain deterministic
preflight failures. The half fixture now reaches its correct first unsupported semantic:

```text
GenericAsm assembly=__slang_atomic_reduce_add($0, $1, (int)$2),
signature=Void(RefParam<half, ...>, half, int)
```

No substring parser or compatibility spelling broadens that boundary.

Integer subtraction is represented by the prelude's exact negated-add spelling and uses the
established typed negation recipe. Inc/dec use exact signed or unsigned 32-bit constants. All other
integer operations and floating add invoke the same atomic descriptor that requirement collection
queried before provider discovery. The provider returns the original value because that is the
generic atomic callback contract; the reduction emitter intentionally discards it and returns
Void.

### The provider widening is semantic, while dialect translation stays isolated

The existing atomic callback and descriptor already carry operation, value type, address space,
memory order, pointer, value, and old result. There is no concrete ABI gap, so revision 29 remains
the current forward-only interface.

The provider now maps exact admitted descriptors to LLVM `Add`, `FAdd`, `And`, `Or`, `Xor`,
signed/unsigned `Min`, and signed/unsigned `Max`. It validates pointer address space, pointee type,
value type, ordering, and dominance before constructing the instruction. The shared semantic
catalog is the single legality source used by compiler, wrapper, fake provider, and LLVM provider.

LLVM 7 predates `atomicrmw fadd` support used by LLVM 14. The serializer first validates that a
floating atomic is global, scalar Float32/Float64, relaxed/system/non-volatile, naturally aligned,
and provider-produced. It then recognizes the complete LLVM 14 rendered line and emits one exact
legacy NVVM intrinsic call plus its exact declaration. Any other floating atomic form fails
serialization. This translation belongs at the dialect boundary because the LLVM module's semantic
source of truth remains a typed `AtomicRMWInst`; the compiler never constructs or patches LLVM
text.

### The bounded probe exposes independent next blockers

Three of the seven target workloads become differentially correct at O0 and O3 and are promoted.
The other four advance only as far as their next independent canonical operation:

| Workload | Final Slice 142 result | First remaining blocker |
| --- | --- | --- |
| `atomic-reduce-float` | Correct | — |
| `atomic-reduce-intrinsics` | Correct | — |
| `ptr-to-groupshared-1` | Correct | — |
| `atomic-reduce-half-cuda` | Preflight | scalar-Half atomic-reduction GenericAsm |
| `atomic-reduce-methods-float` | Preflight | ordinary `atomicLoad` |
| `atomic-reduce-methods` | Preflight | ordinary `atomicLoad` |
| `pointer/const-ref` | Preflight | canonical struct field address |

Atomic load/store/compare-exchange needs a concrete provider-interface and memory-order design;
approximating it with an RMW would change semantics. Struct field addressing belongs to the
aggregate/pointer/layout cluster. Half and Half2 reductions require distinct NVVM operations. They
remain measured failures rather than attracting speculative callbacks or downstream patches.

### Fixed-denominator coverage and Pareto result

The fixed corpus remains 452 workloads from 448 sources: 430 MVP and 22 extension. Native
CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 341 | 8 | 96 | 7 | 349 |
| Direct O3 | 346 | 8 | 96 | 2 | 354 |

Both direct modes gain exactly three workload identities and lose none from Slice 141. All three
are MVP. Among 427 healthy MVP references, O0 correctness is 339/427 (79.4%), O3 correctness is
343/427 (80.3%), and both-mode correctness is 339/427 (79.4%).

The leading remaining healthy-MVP clusters are:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Aggregate/pointer/layout transport | 12 | 12 |
| Preflight other | 11 | 11 |
| Atomic/wave operation | 10 | 10 |
| Residual target marker/undefined value | 10 | 10 |
| Helper ABI/type contract | 9 | 9 |
| Wave/reconvergence GenericAsm | 8 | 8 |
| Function identity | 6 | 6 |
| Raw-buffer view access | 4 | 4 |

O0 additionally has four healthy provider failures in the unoptimized Half-operation cluster. The
next slice should decompose the leading aggregate/pointer/layout population by exact canonical
producer before choosing another vertical representation.

### Representative and productionization gates

All three representative release gates remain differentially correct. Median standalone compile
time and generated PTX size from three final samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 388.0 ms / 8,889 B | 271.6 ms / 6,102 B | 270.2 ms / 919 B |
| Parameter-block layout | 385.8 ms / 8,839 B | 247.9 ms / 917 B | 252.3 ms / 793 B |
| Shared control/barriers | 373.3 ms / 9,190 B | 251.7 ms / 1,940 B | 256.4 ms / 1,404 B |

Across all census lanes, startup-inclusive compile/load/execute/compare median/p90/mean times are
4782.5/5095/4854.2 ms for NVRTC O3, 4719.5/5116/4760.3 ms for direct O0, and
4760/5223/4842.9 ms for direct O3. These are not kernel-only runtime measurements.

CUDA 12.9 `ptxas` accepts every representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 workers
remain productionization gaps. The isolated LLVM 14 provider remains compiler-matched at ABI
revision 29.

### Validation and self-review

- Release compiler/unit-test and isolated provider builds pass outside the sandbox.
- Nine focused helper-reference, atomic-reduction, adjacent-negative, real differential, and
  `ptxas` tests pass.
- All twelve active native/direct/Vulkan lanes in the three promoted fixture files pass; three
  unrelated backend lanes are ignored by their existing test conditions.
- The final 452-workload O0/O3 census has three gains and zero old-correct regressions.
- The selected direct-NVVM prefix passes 421/421.
- Representative direct O3 PTX assembles for SM70, SM80, and SM90.
- Pinned formatting and `git diff --check` pass.

The new-helper/special-case inventory is the atomic-storage, helper-reference, shared-helper, shared
array/element classifiers; the two exact helper call relations; global-producer classification;
the atomic-reduction descriptor/resolver/requirement/emitter; and the provider's typed atomic and
legacy Float32/Float64 serialization cases. Each entry above names its canonical producer, proves
why the shape is valid, and identifies its positive and adjacent-negative coverage. Self-review
removed unproved 16-bit atomic alignment and aggregate groupshared pointer widening. No fixture-name
check, syntax reconstruction, custom type equivalence, compatibility fallback, arbitrary operand
walk, silent default, malformed-upstream patch, or provider ABI widening is retained.
