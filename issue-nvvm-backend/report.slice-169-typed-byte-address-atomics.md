# Slice 169: Typed byte-address memory and common atomics

## Motivation

Six healthy frozen-v1 workloads reached two related typed raw-memory boundaries. Consider:

```slang
struct Data { int16_t a; int16_t b; }
Data value = input.Load<Data>(byteOffset);
output.Store<Data>(byteOffset, value);
```

Byte-address legalization decomposes this aggregate into scalar Int16 loads and stores at offsets
zero and two. Direct NVVM rejected the canonical narrow leaves. Related atomic workloads either
used the established equivalent structured-buffer pointer for Half or retained exact CUDA-prelude
GenericAsm helpers for Float32 add and UInt64 compare-exchange.

## Proposed solution

Widen the selected byte-address physical leaf family, derive its implicit alignment from the
producer's bounded alignment contract, and reuse existing typed pointer, load/store, and atomic
builder operations. Add no provider callback: provider ABI revision 32 already expresses every
operation. Implement only the concrete scalar Half global add gap inside the LLVM provider through
exact typed PTX inline assembly. Match retained raw-buffer atomic helpers by their complete body and
typed signature, never by source or fixture name.

## Change summary

- Byte-address values now include selected 16-bit integer/Half leaves and finite selected vectors;
  omitted alignment is `min(4, natural alignment)`.
- Equivalent structured-buffer views admit the selected narrow physical leaves used by canonical
  byte-address legalization.
- The shared atomic catalog and LLVM provider support relaxed scalar global Half add through
  `atom.global.add.noftz.f16` with i16 bit transport.
- Exact complete Float32-add and UInt64-CAS CUDA-prelude helpers lower through the existing raw-view,
  byte-offset-pointer, atomic, and local-result-store operations.
- Five workloads gain permanent direct O0/O3 lanes. Separate frozen and discovery artifacts, three
  measurement gates, the design, capability ledger, plan, and this report retain the evidence.

## Concepts and vocabulary

**Implicit byte-address alignment** is the alignment represented by an omitted or zero alignment
operand after byte-address legalization. It is capped at four bytes and reduced for narrower
physical leaves.

**Equivalent structured-buffer view** is the canonical raw-buffer legalization result that gives
an atomic operation a typed global element pointer without changing the underlying byte-address
storage.

**Complete GenericAsm helper** is a one-block helper whose entire assembly string, result,
parameter count, parameter types, and out-parameter transport form one exact semantic contract.

## Process report

The first input-shape audit traced `byte-address-16bit.slang` and its vector companion through
byte-address legalization. The producer emits ordinary scalar Int16 accesses; it does not retain a
source aggregate requiring downstream reconstruction. The existing classifier already drove type
lowering, pointer formation, loads, stores, and finite numeric arrays, so widening that one physical
leaf predicate is the correct ownership boundary.

The initial implementation interpreted omitted alignment as unrestricted natural alignment. The
selected-prefix revert drill immediately failed three existing fake-provider tests: wide integer,
vector, and numeric-array access had intentionally retained four-byte alignment. That evidence
refined the producer contract to `min(4, natural alignment)`. Int16/Half therefore use two while
all established wider forms retain four. No test expectation was relaxed to hide the regression.

`byte-address-half-atomics.slang` next proved that the canonical producer is
`getEquivalentStructuredBuffer` followed by the existing structured element pointer and scalar
atomic add. The result view's physical element is Half; no `Atomic<T>` wrapper reconstruction is
needed downstream. The compiler's shared descriptor catalog now admits only global scalar Half add.
The LLVM provider validates the already-typed Half pointer/value, bitcasts them to i16 transport,
emits exact side-effecting `atom.global.add.noftz.f16`, and restores the returned Half bits. The
real workload verifies, compiles through libNVVM, assembles, and matches the native CUDA result.
The attempted neighboring reduction advances to an independent
`RefParam<vector<half,2>>` GenericAsm shape and remains deterministically rejected.

The Float32 and UInt64 audit started from the exact linked helper bodies produced from
`hlsl.meta.slang`:

```text
(*$3 = atomicAdd($0._getPtrAt<float>($1), $2))
(*$4 = atomicCAS($0._getPtrAt<uint64_t>($1), $2, $3))
```

`_resolveNVVMByteAddressAtomic` accepts only those complete strings plus a one-instruction void
body, a read-write byte-address buffer, UInt32 byte offset, exact scalar operands, and an exact
numeric `OutParam<T>`. An early focused run exposed a classifier mistake: the result used the
existing local numeric out-parameter representation, not the separate `RefParam` helper-reference
family. Reusing the correct classifier made both workloads pass without widening either family.
Emission extracts raw field zero, creates one typed global pointer at the byte offset, invokes the
existing atomic descriptor, stores the old value to the out parameter, and returns void.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy references. O0, O3, and both-mode
correctness advance from 402 to 407, with exactly the five bounded gains and zero old-correct
regressions. Direct all-row classification is 421 correct, eight runtime mismatches, and 23
preflight failures in each mode. The remaining atomic cluster is one exact vector-Half reduction.

Discovery remains exactly 82 workloads and 72 healthy references at 68/68/68 O0/O3/both. Its
direct classification remains 68 correct, seven infrastructure failures, six preflight failures,
and one runtime mismatch in each mode. No discovery workload was newly unlocked. Its leading
healthy clusters remain aggregate struct-field pointer, aggregate storage layout, aggregate helper
ABI, and double-indirect pointer helper ABI; none overlaps the retained scalar typed-memory change
enough to justify broadening this slice.

The selected prefix passes 433/433 after the alignment correction, and the permanent NVVM category
passes 60/60. The three bounded gates produce 15 accepted configurations. At SM70, direct O3 PTX is
1,186 bytes for the narrow vector workload, 1,043 bytes for Half atomic add, and 1,153 bytes for
UInt64 CAS, versus native NVRTC PTX of 9,672, 13,050, and 9,354 bytes respectively. All direct O3
PTX assembles with CUDA 12.9 for SM70, SM80, and SM90. Three-repetition timings remain exploratory.

The final special-case inventory retains four bounded decisions. The shared physical leaf
predicate survives because canonical legalization and five runtime tests prove it. The implicit
alignment helper survives because old and new tests jointly prove its exact contract. The complete
raw-buffer helper resolver survives because GenericAsm is the canonical final producer and the
full signature proves this layer owns semantic lowering. The provider Half branch survives because
the existing descriptor cannot otherwise produce accepted libNVVM input, while real verification,
assembly, and runtime tests prove the exact operation. The unused `Atomic<T>` equivalent-view
unwrap was removed during self-review. No fixture-name check, compatibility fallback, syntax
reconstruction, arbitrary operand-graph search, downstream malformed-IR patch, serialized-text
rewrite, or new provider ABI remains.
