# Slice 139 UserPointer helper transport report

## 1. Motivation

The Slice 138 census left 28 healthy-MVP workloads in the broad helper ABI/type-contract cluster.
The largest coherent subfamily carried the canonical CUDA `AddressSpace::UserPointer` spelling
through direct helpers, finite aggregates, and AnyValue marshalling.

Consider this source shape:

```slang
struct IndirectNode : INode
{
    IValue* source;
    int evaluate() { return source->get(); }
}
```

CUDA specialization preserves `source` as an exact final linked type:

```text
Ptr<IValue, addressSpace=UserPointer, access=ReadWrite, layout=DefaultBufferLayout>
```

AnyValue lowering can reinterpret the pointer as UInt64 or UInt2, carry it through finite
structs/arrays and direct calls, reconstruct it, and dereference it. Before this slice, the direct
path rejected the first helper signature or pointer-bearing aggregate despite already supporting
the surrounding calls, aggregate operations, loads, and stores.

The objective was one reusable representation for this producer family, measured against the fixed
452-workload census. It was not to add dynamic-dispatch fixture checks.

## 2. Proposed solution

The compiler now recognizes one exact UserPointer contract: ordinary four-operand read-write
`Ptr<T>` with the complete 64-bit UserPointer address space, default buffer layout, and an already
selected finite copyable pointee. Helper values form a finite, cycle-safe algebra of copyable
leaves, exact UserPointer leaves, nonempty fixed arrays, and nonempty structs.

Type lowering is role-specific. Kernel parameters and pointer fields in conventional global
storage use LLVM global address space because their producers prove global provenance. Helper
pointer leaves use LLVM generic address space because the same helper contract can also receive a
local `__getAddress`. Emission preserves AS1 for ordinary loads, stores, offsets, and atomics, then
widens a producer-proven global pointer only when it enters helper-value transport.

AnyValue pointer marshalling accepts only exact UInt64 and UInt2 bit payloads. Forward-only builder
ABI revision 29 adds the two concrete operations revision 28 could not express:

- generic pointer-to-bits/bits-to-pointer transport, implemented as exact LLVM
  `ptrtoint`/`inttoptr` plus UInt2/i64 reinterpretation; and
- typed pointer address-space conversion, implemented as LLVM `addrspacecast` with the same
  pointee and different recognized address spaces.

The provider also normalizes an already-classified Int32 shift count to the UInt64 shifted value's
physical LLVM width. Slang's canonical operation preserves the source count width; this is LLVM
operand legalization, not a new semantic operation.

## 3. Change summary

- `slang-emit-nvvm-type-lowering.*` adds exact UserPointer and finite helper-value classification,
  alignment, local-helper-pointer recognition, and separate entry/storage/helper physical type
  maps.
- `slang-emit-nvvm.cpp` admits the representation throughout helper signatures and bodies,
  validates exact pointer bit transport, materializes null pointers, derives reachable structs
  from all admitted types, tracks global pointer provenance, and widens only at helper-value
  boundaries.
- The builder facade and isolated LLVM 14 provider advance to ABI revision 29 with two generic
  construction callbacks. Focused real-provider tests validate success, rejected neighbors,
  insertion-point ownership, cross-module handles, pointee preservation, and exact emitted IR.
- The fake provider records typed cast producers so existing compiler graph tests keep exact
  downstream type checks. Its address-space-insensitive type interning remains intentionally
  limited; the real-provider test owns address-space legality.
- All 20 newly correct census workloads receive explicit direct O0 and O3 runtime lanes. The fixed
  census/Pareto artifacts, capability ledger, and design status are updated.

## 4. Concepts and vocabulary

- **UserPointer**: the complete post-specialization Slang pointer spelling used for a source CUDA
  device address; its 64-bit address-space value is `0x100000001`.
- **Helper value**: a finite first-class value that can cross a direct helper boundary, including a
  struct or array containing exact UserPointer leaves.
- **Producer-proven global pointer**: a kernel parameter or pointer loaded from conventional global
  parameter storage whose origin establishes LLVM AS1.
- **Helper-value boundary**: a call argument, helper return, phi input, or local/aggregate
  construction where global and local pointer origins intentionally share one generic physical
  representation.
- **Pointer bit payload**: exactly UInt64 or UInt2 carrying one 64-bit pointer representation during
  AnyValue marshalling.
- **Healthy MVP reference**: one of the 427 MVP workloads whose native CUDA/NVRTC result is correct.

## 5. Process report

### The complete linked pointer type owns admission

The shape reaching preflight is produced by CUDA address-space specialization and preserved by
linking. It is canonical and intentionally valid. `asNVVMSupportedDeviceCopyableValuePointerType`
requires the ordinary pointer opcode, four operands, read-write access, the complete UserPointer
value, default layout, and a selected copyable pointee. Adjacent access qualifiers, layouts,
address spaces, resources, and recursive storage graphs fail the existing deterministic type
diagnostic.

The initial trace appeared to report address space `1`, but the diagnostic had truncated the
64-bit enum. Printing the complete value exposed `4294967297`; no front-end or syntax repair was
required. The semantic source of truth was already the final `IRPtrTypeBase`.

`_isNVVMSupportedHelperValueType` follows arrays and struct fields with an active-type set but stops
at pointer leaves. This admits the exact finite values emitted by AnyValue and dynamic-dispatch
lowering without turning the pointer's pointee into an arbitrary recursive storage graph. Removing
this algebra restores the original helper parameter/result failures in the promoted family.

### Physical address space follows producer role

The first implementation converted every entry UserPointer from AS1 to AS0. The revert drill showed
why that invariant was too broad: ordinary global stores became generic PTX stores, and global
atomic descriptors no longer matched their pointer operand. Those failures were not valid new
input shapes; they proved the conversion was at the wrong layer.

The retained representation fixes the producer/consumer boundary. Entry-point lowering and
conventional-global storage create AS1 pointer types. `globalUserPointers` records only values from
those canonical producers and propagates the role through exact pointer offsets. Ordinary memory
operations consume the AS1 value unchanged. `_getLoweredNVVMHelperValue` widens and caches it only
when a helper call, helper return, phi, local helper store, or pointer-bearing aggregate requires
the AS0 helper representation. Helper parameters, call results, local loads, and reconstructed
pointer bits are already generic and do not receive a fallback cast.

The 407/407 regression proves global stores, shared/global atomics, CUDA execution, PTX evidence,
ptxas, and runtime comparisons remain intact. The promoted pointer/AnyValue files prove the
boundary cast is necessary and owned by helper transport.

### Pointer bits use exact structural operations

AnyValue lowering already produces canonical one-operand `kIROp_BitCast` instructions between an
exact UserPointer and UInt64 or UInt2. `_getNVVMPointerBitCast` recognizes only those complete type
pairs. It does not parse source syntax, inspect a fixture name, or infer arbitrary integer-pointer
compatibility.

Revision 28 could reinterpret first-class numeric values but could not express LLVM
pointer-to-integer, integer-to-pointer, or address-space casts. Revision 29 therefore appends two
generic construction callbacks. The provider rejects null, foreign-module, wrong-context,
post-terminator, opaque-pointer, same-address-space, different-pointee, pointer-to-pointer, and
non-64-bit bit-pattern shapes before mutation. The focused builder test checks both accepted
directions, the AS1-to-AS0 cast, the round trip, and emitted LLVM/NVVM assembly.

The `impl-ptr-field-anyvaluesize` workload exposed one adjacent canonical operation: a UInt64 shift
with Int32 count. `resolveValueOperationFamily` now preserves independently selected integer count
widths, and the provider performs the LLVM-required zero-extend/truncate after semantic
classification. Both O0 and O3 runtime lanes prove this layer owns the physical normalization.

### Reachable types come from admitted producers

Admitting pointer-bearing helper structs initially exposed a module-scope struct-definition
failure. The struct was valid, but the selected reachable-type inventory only seeded copyable and
resource aggregates. Adding a module-scope exception would have patched the consumer.

`_addNVVMReachableStructTypes` now starts from every admitted function result, parameter,
instruction result, and local helper pointee; it unwraps exact UserPointer pointees and traverses
finite helper arrays/structs. Module-scope validation remains strict and simply consumes the
complete producer-derived inventory. `impl-concrete-ptr-field` is the motivating regression.

### Promotion and fixed-denominator coverage

The fixed corpus remains 452 workloads from 448 sources: 430 MVP and 22 extension. Native
CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 318 | 8 | 119 | 7 | 326 |
| Direct O3 | 323 | 8 | 119 | 2 | 331 |

Both direct modes gain the same 20 exact success identities and lose none from Slice 138. Among
427 healthy MVP references, O0 correctness is 317/427 (74.2%), O3 correctness is 321/427 (75.2%),
and both-mode correctness is 317/427 (74.2%). The selected 407/407 prefix is a regression score,
not the coverage denominator.

The newly correct workloads are `gh-8185`, signed/unsigned 64-bit scalar intrinsics,
`struct-bit-cast-2`, `metal-pointer-uniform`, and 15 dynamic-dispatch/AnyValue workloads. All 20
files now carry O0/O3 direct lanes, and every native/direct lane in those files passes.

The leading remaining healthy-MVP failure clusters are:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Wave/reconvergence GenericAsm | 19 | 19 |
| Helper ABI/type contract | 16 | 16 |
| Aggregate/pointer/layout transport | 14 | 14 |
| Ordinary numeric/bit operation | 11 | 11 |
| Residual target marker/undefined value | 9 | 9 |
| Atomic/wave operation | 8 | 8 |
| Function identity | 6 | 6 |
| Raw-buffer view access | 4 | 4 |

### Representative and productionization gates

All three representative release gates remain differentially correct. Median standalone compile
time and generated PTX size from three samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 380.8 ms / 8,889 B | 269.5 ms / 6,102 B | 275.8 ms / 919 B |
| Parameter-block layout | 371.5 ms / 8,839 B | 243.9 ms / 917 B | 251.3 ms / 793 B |
| Shared control/barriers | 376.4 ms / 9,190 B | 255.6 ms / 1,940 B | 261.5 ms / 1,404 B |

Across all census lanes, startup-inclusive compile/load/execute/compare median/p90/mean times are
4583.0/4829/4637.6 ms for NVRTC O3, 4419.0/4690/4453.7 ms for direct O0, and
4408.0/4752/4470.3 ms for direct O3. These are not kernel-only runtime measurements.

CUDA 12.9 `ptxas` accepts every representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 workers remain
productionization gaps. The isolated LLVM 14 provider remains compiler-matched and moves forward
with Slang at ABI revision 29.

### Validation and self-review

- Release host, unit-test, and isolated-provider builds pass outside the sandbox.
- The real-provider pointer transport test covers both callbacks and rejected neighboring shapes.
- All 20 promoted fixture files pass native and direct O0/O3 runtime lanes.
- The selected NVVM prefix passes 407/407.
- The final three-mode 452-workload census has zero old-correct regression.
- All representative runtime comparisons pass and direct O3 PTX assembles for SM70/SM80/SM90.

The new-helper inventory contains the exact UserPointer classifier, recursive helper-value
classifier/alignment, helper array/struct/local-pointer classifiers, pointer-bitcast resolver,
null-pointer materializer, provenance-sensitive helper materializer, reachable-type collector,
provider bit-pattern predicate, and fake typed-cast records. Every retained branch consumes a
canonical linked type or producer-owned value and has tests named above. There is no fixture-name
check, syntax reconstruction, compatibility fallback, arbitrary operand-graph search, custom
semantic equivalence, or downstream repair for malformed IR.
