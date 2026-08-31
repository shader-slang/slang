# Slice 165: UserPointer address-space transport

## Motivation

Two healthy discovery workloads failed in the same provider operation even though they combine
different storage classes. Consider the conventional-global case:

```slang
struct BufferSink
{
    UserPointer<int> buffer;
    void store(int index, int value) { buffer[index] = value; }
}

UserPointer<int> outputBuffer;

[shader("compute")]
void computeMain()
{
    BufferSink sink = { outputBuffer };
    sink.store(0, 1);
}
```

The final linked IR puts `outputBuffer` in the synthesized `GlobalParams` conventional global,
loads it, stores it into the pointer-bearing local `BufferSink`, and passes that helper value to
`BufferSink.store`. The group-shared workload performs the complementary combination:

```slang
RWStructuredBuffer<uint> buffer;
groupshared UserPointer<uint> sharedPtr[4];

sharedPtr[group_thread_id.x] = &buffer[group_thread_id.x];
GroupMemoryBarrierWithGroupSync();
*sharedPtr[group_thread_id.x] = group_thread_id.x + 1;
```

Both direct modes reached E52018 for `global-to-generic UserPointer conversion`. The exact common
question was whether a canonical physical device pointer was reaching the existing address-space
conversion, or whether type lowering had already erased its physical role.

## Proposed solution

Make an explicit `NVVMTypeUse::Storage` recursive: fields of an aggregate lowered for ordinary
storage retain Storage use before a possible helper-struct classification is considered. A
`UserPointer<T>` field in conventional global storage consequently lowers as LLVM AS1. The existing
`_getLoweredNVVMHelperValue` boundary then converts that physical value exactly once to the generic
AS0 representation used by pointer-bearing local and helper values.

Keep the provider contract strict. It continues to reject identity casts, mismatched pointees,
unsupported address spaces, foreign values, and invalid insertion points. The revision-31 generic
pointer-cast operation already expresses the required conversion, so neither a callback nor an ABI
revision is warranted.

## Change summary

- Recursive struct lowering now preserves an explicit conventional Storage use ahead of optional
  helper-value classification.
- `force-inline-array.slang` and `groupshared-ptr-of-device.slang` gain permanent direct-NVVM O0
  and O3 differential lanes.
- Frozen and discovery census/Pareto artifacts, a 28-gate measurement manifest, design record,
  capability ledger, completed plan, and this five-part report retain the evidence.
- Provider implementation and ABI revision 31 are unchanged.

## Concepts and vocabulary

**Physical pointer representation** is the LLVM address space implied by a producer-proven storage
role. Ordinary device/global pointers use AS1 and group-shared storage uses AS3.

**Executable helper representation** is the generic AS0 pointer value used inside first-class
pointer-bearing locals, helper aggregates, and helper calls because such values can originate from
more than one physical storage class.

**Type use** is the lowering context that distinguishes the representation of the same Slang type
when used as storage, a helper value, parameter-group storage, or structured-buffer storage.

## Process report

The audit started with final linked IR for both failures and temporary provider instrumentation at
`emitPointerAddressSpaceCast`. Contrary to the diagnostic label, both the supplied value and the
requested result were AS0 `i32*`; their pointees matched and the insertion point was valid. The
provider correctly rejected an identity cast. The instrumentation was then removed and the
provider rebuilt, leaving no diagnostic or permissive branch in production.

Tracing backward found the representation break in
`NVVMTypeLoweringContext::_lowerStructType`. The synthesized `GlobalParams` type is a valid
pointer-bearing helper struct, so the previous field-use selection chose `HelperValue` even when
its caller explicitly requested `NVVMTypeUse::Storage`. Its `UserPointer<Int>` field consequently
became AS0. Separately, the conventional-global load producer correctly marked the loaded value in
`globalUserPointers`, and `_getLoweredNVVMHelperValue` correctly requested physical-to-executable
transport. Those two individually reasonable paths combined into the observed AS0-to-AS0 request.

The exact input shape is canonical and intentionally allowed: `collectEntryPointUniforms` creates
one conventional global aggregate, its field layout is physical immutable storage, and final IR
loads a canonical `UserPointer<Int>` from that field before constructing a local helper value.
The accidental part was not the IR shape but the lowered field representation. Storage is the
producer-supplied source of truth; helper-struct classification merely says another representation
exists for executable values. Giving the explicit use precedence fixes the producer boundary and
keeps downstream emission simple.

After the correction, O0 PTX for the conventional-global workload loads the pointer from constant
storage and emits `cvta.global.u64` before storing the generic helper value. The group-shared
workload loads a device pointer, converts it once, writes that value with `st.shared.u64`, and later
reads it with `ld.shared.u64`. The native NVRTC reference likewise loads the conventional pointer
and performs a global-address conversion, providing independent evidence for the representation.

Three alternatives were rejected. Allowing provider identity casts would weaken a useful typed
contract and hide the compiler error. Removing `globalUserPointers` provenance would discard the
physical role needed by conventional globals and raw entry pointers. Adding another provider
operation would duplicate a generic cast already present in revision 31. None addresses the source
of the invalid AS0 representation.

A synthetic fake-emitter fixture was also attempted and removed. That harness intentionally only
recognizes pointer-bearing fake `GlobalParams` when a resource leaf is present, while its copyable
struct model excludes pointer leaves. Broadening those unrelated fake policies merely to recreate
this cross-layer integration shape would not prove the real storage contract. The two permanent
real-provider differential fixtures instead exercise the canonical producer, type lowering,
provider, PTX assembly, launch, and result comparison together. Existing provider unit coverage
continues to prove valid AS1-to-AS0 conversion and adjacent identity/mismatch rejections.

Both promoted workloads compare correctly against their stable NVRTC references at O0 and O3.
Frozen corpus v1 retains exactly 452 workloads/427 healthy references and remains 396/400/396
O0/O3/both, with no old-correct regression. Discovery retains exactly 82/72 and advances from
64/64/64 to 66/66/66, with exactly the two intended gains and no loss. The selected NVVM unit
prefix passes 433/433.

The 28-gate measurement run produced 140 rows and 140 assembled cubins. At direct O3 SM70, the
conventional-global gate measured 256.4 ms and 630-byte PTX versus 394.0 ms and 8,569 bytes through
NVRTC O3; direct O0 emitted 20,352-byte PTX. The group-shared gate measured 283.7 ms and 991-byte
PTX versus 406.2 ms and 8,747 bytes; direct O0 emitted 1,347-byte PTX. Direct O3 PTX assembled with
CUDA 12.9 for SM70, SM80, and SM90. These one-repetition numbers remain exploratory rather than
controlled benchmark claims.

The final special-case inventory contains one surviving production precedence branch: explicit
Storage use is preserved recursively. It is owned by the type-lowering boundary and is proved by
two distinct real storage combinations. No new helper, fallback, compatibility path, fixture-name
check, syntax reconstruction, operand-graph search, downstream malformed-IR patch, provider
callback, or provider ABI revision survives the slice.
