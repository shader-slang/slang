# Slice 174: Physical parameter-group helper references

## Motivation

The discovery workload stores matrices inside nested constant-buffer records and reads them
through ordinary methods:

```slang
struct DoubleNested
{
    int4x3 matrix;
    int getMatVal(int i, int j) { return matrix[i][j]; }
}

struct Nested
{
    bool values[4];
    DoubleNested doubleNested;
    int getVal(int id) { return (int)values[0] + doubleNested.getMatVal(0, 1); }
}

struct Params
{
    Nested nested;
    int getVal(int id) { return nested.getVal(id) + nested.getVal(id + 1); }
}

RWStructuredBuffer<int4> outputBuffer;
ConstantBuffer<Params> gParams;
uniform DoubleNested* gDoubleNested;

[shader("compute")]
void computeMain(int id : SV_DispatchThreadID)
{
    outputBuffer[0].xyz = gParams.getVal(id) + gDoubleNested.getMatVal(1, 1);
}
```

CUDA buffer-element lowering does not keep `Params`, `Nested`, or `DoubleNested` as ordinary SSA
value structs. It produces decorated `Params_natural`, `Nested_natural`, and
`DoubleNested_natural` physical storage structs, then transforms aggregate method receivers into
exact immutable references to those types. Direct NVVM stopped first at
`BorrowInParam<Params_natural, Read, Generic, DefaultLayout>` and could not preserve the storage
representation through the synthesized global, nested helper calls, and physical matrix fields.

## Proposed solution

Treat a finite decorated physical struct as a parameter-group storage role, not as an ordinary
copyable helper value. Classify only the exact immutable helper reference, generic local pointer,
and CUDA device-pointer spellings whose pointee is that physical struct. Lower every such pointee
through `NVVMTypeUse::ParameterGroupStorage`, and relate calls only when the canonical pointee types
match and the argument producer proves either parameter-group-global or local provenance.

Carry an explicit physical-storage root bit through nested field resolution. The physical matrix
producer keeps the root semantically immutable but spells the selected array element pointer as
read-write. Matching that exact result spelling while retaining separate immutable load semantics
preserves both the upstream IR contract and the storage ABI. Existing revision-32 field, element,
aggregate, load/store, address-space-cast, and call operations express the complete graph, so the
provider ABI does not change.

## Change summary

- Aggregate and parameter-group storage classification admits finite multi-field physical structs
  and their Boolean leaves while keeping the narrow physical-array wrapper classifier separate.
- Exact physical reference, local, and device pointer roles participate in type lowering, pointer
  validation, conventional globals, helper signatures, call relations, and local storage.
- Nested physical field and sequential element resolution preserve the physical root, semantic
  immutability, explicit offsets/strides, and compact parameter-group vector alignment.
- `optimization/arrray-storage-lowering.slang` gains permanent O0/O3 direct-NVVM differential
  lanes.
- Frozen/discovery TSV and Pareto JSON, a representative measurement manifest, design notes, the
  capability ledger, plan, and this report retain the evidence.

## Concepts and vocabulary

**Physical storage struct** is a finite `IRStructType` carrying `IRPhysicalTypeDecoration`; CUDA
buffer-element lowering gives its fields the external parameter-group storage representation.

**Physical helper reference** is the exact
`BorrowInParam<T, Read, Generic, DefaultLayout>` method receiver produced for a physical storage
struct.

**Semantic immutability** describes the root storage contract used for invariant loads. It is
tracked separately from the access operand on a derived pointer instruction when the canonical
producer spells that result as read-write.

## Process report

CUDA buffer-element lowering produces the physical structs and their layout decorations before
the direct backend sees the linked module. Parameter-group lowering places a pointer to
`Params_natural` behind the collected `ConstantBuffer<Params>` global. Transform-parameters-to-
constref gives `Params.getVal`, `Nested.getVal`, and `DoubleNested.getVal` immutable
`BorrowInParam<PhysicalStruct>` receivers. Nested calls either pass that global parameter-group
pointer directly or load a physical child into a decorated `TempCallArgImmutableVar` and pass its
generic local pointer.

The aggregate-storage classifier previously treated every decorated physical struct as if it were
the special one-field physical-array wrapper. That assumption rejected the actual nonempty
multi-field records. The retained change removes the assumption from the general recursive
storage grammar; `asNVVMSupportedPhysicalArrayStructType` still owns and enforces the one-array-
field wrapper shape. Boolean is admitted as a storage leaf because CUDA physical lowering emits it
inside this same external storage algebra. Removing either change restores the physical aggregate
classification failure before helper validation.

`asNVVMSupportedPhysicalStorageReferencePointerType` requires the exact four operands, read access,
generic address space, default layout, and decorated aggregate-storage pointee. The local
classifier requires the exact one-operand read-write generic `Ptr`, and the device classifier
requires the exact four-operand read-write `UserPointer`/default-layout spelling. They are separate
from ordinary copyable/helper pointer families, so supporting a physical storage receiver does not
make arbitrary aggregate references or launch values legal.

Type lowering maps all three pointer pointees through `ParameterGroupStorage`. Conventional-global
and resource layout proof recognizes the device physical pointer as an eight-byte pointer leaf.
Helper validation relates a physical receiver only to an exact parameter group carrying the same
pointee or an exact generic local physical pointer with that pointee. Emission address-space-casts
the global pointer to the generic helper signature; local pointers already use that address space.
Local `var`, aggregate load/store, and alignment paths use the same recursively proven physical
layout rather than executable-value layout.

Nested field resolution originally reused one output variable for the physical-reference and
local-physical classifiers. Both helpers clear their optional output on entry, so the failed local
classifier erased the successful reference result and caused a null struct dereference. The fix
stores each mutually exclusive result independently and selects it only when its classifier
succeeds. This is classifier state ownership, not an alternate representation or failure fallback.

The final physical matrix field has storage type `Array<Vec<int, 3>, 4, stride 12>`. Its field
address is rooted in an immutable physical receiver, but `IRBuilder::emitGetElementPtr` produces an
ordinary read-write generic result pointer. The sequential-element resolver now carries an
explicit `isPhysicalStorage` bit from the exact reference/local root through nested fields. Only an
immutable physical array field uses the observed read-write result spelling; ordinary immutable
aggregate fields retain their existing access check. `isImmutable` remains true, so load emission
retains invariant semantics, and the existing compact parameter-group vector resolver supplies
four-byte scalar alignment for each three-lane vector in the stride-12 matrix storage.

The self-review inventory contains six retained changes. General multi-field physical aggregate
storage survives because removing it restores the decorated-struct rejection while the narrow
array-wrapper classifier remains unchanged. The Boolean leaf survives because it is an actual
field type produced inside the same physical storage records. The three exact pointer classifiers
survive independently at the helper signature, synthesized global, and decorated-local producer
boundaries. The explicit physical-root marker survives because removing it rejects the canonical
matrix element pointer, while broadening the predicate to every immutable aggregate field would
admit shapes this slice does not prove. No source-name check, syntax reconstruction, compatibility
fallback, arbitrary operand search, malformed-IR patch, diagnostic weakening, or provider callback
was added.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. Healthy correctness
stays 413/413/413 O0/O3/both, with zero classification change and zero old-correct regression.
All-row direct totals remain 427 correct, four runtime mismatches, and 21 preflight failures in
each mode. Discovery remains exactly 82 workloads and 72 healthy references and advances from
70/70/70 to 71/71/71, with exactly `optimization/arrray-storage-lowering` gained and no loss.

The selected regression prefix passes 433/433, the permanent `nvvm` category passes 78/78, and the
focused shader passes 4/4. The representative gate compiles and assembles through CUDA 12.9 for
native NVRTC, direct O0 SM70, and direct O3 SM70/SM80/SM90. At SM70, direct O3 PTX is 836 bytes
versus 9,761 bytes native, and median standalone compile time is 257.7 ms versus 376.4 ms. These
remain exploratory measurements rather than a controlled benchmark.
