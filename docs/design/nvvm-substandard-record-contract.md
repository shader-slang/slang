# NVVM internal substandard-float records

Slice270 qualifies flat internal records containing integer scalar fields together with scalar
FloatE4M3, FloatE5M2, BFloat16 or vector<BFloat16,2>. Slice279 extends this to finite nonempty
nested records with the same leaves. At least one substandard
descendant is required; integer-only child records may accompany it.
The [nested-record report279](../../issue-nvvm-backend/report.slice-279-nested-records.md) owns the
new qualification and limits; [report270](../../issue-nvvm-backend/report.slice-270-fp8-aggregate.md)
retains the original flat-record evidence. Existing [FP8 scalar semantics](nvvm-fp8-scalar-contract.md) and
[BF16 vector/storage semantics](nvvm-bf16-vector-contract.md) remain distinct leaf contracts.

## Canonical values and local memory

Consider the concrete type behind a dynamic interface payload:

```slang
struct Value
{
    FloatE4M3 first;
    FloatE5M2 second;
}
```

AnyValue unpacking creates a local Value, writes each canonical field from the corresponding payload
byte through a semantic bitcast, then loads and returns the complete record. Its IRStructType and
field keys are the semantic source of truth. No alternate record type or syntax reconstruction is
needed. Both leaf formats use physical i8 while preserving distinct semantic types. Internal record
construction, extraction, calls, returns and phi transport use ordinary aggregate builder operations.

The selected domain has identical physical register and local-memory representations. Scalar BF16
uses i16 and BF2 uses <2 x i16> in both roles. This does not extend to BF3/BF4 whole-record values:
their existing local fields use component-array storage, which remains a separate qualified domain.

`asNVVMSupportedSubstandardRecordType` owns the bounded value-domain classification.
Nested membership walks canonical field types and rejects recursive, empty or unqualified records.
`asNVVMSupportedLocalSubstandardRecordType` combines it with the existing local-only BF16 record
family. Exact Generic local Ptr, OutParam and BorrowInOutParam roots qualify memory access; field
addresses are resolved by canonical keys. A nested field inherits local substandard storage permission
only from a qualified parent address. No recursive copyable/helper classifier is widened.

## Layout and cache identity

AnyValue payload packing and CUDA local allocation use different layout rules. A record containing
uint16_t, BF2, uint16_t has Natural offsets0/2/6 and size8/alignment2, but CUDA offsets0/4/8 and
size12/alignment4. `getSizeAndAlignment` caches each rule separately. Local allocation uses the
existing aggregate layout walk to prove CUDA-compatible field offsets, size and alignment before
emission; whole-record loads and stores use that proven allocation alignment.

Value and Storage caches remain separate, with role validation before cache lookup. Selected records
need no load/store conversion because their physical field representations agree. BF2 component
addresses require an actual field of a qualified local record, preserving existing exact pointer,
access, address-space, index and element-type checks. This also composes with BF2 fields beside
existing local BF3/BF4 fields; it does not qualify BF3/BF4 component-pointer access.

## Boundaries

The domain excludes record arrays, readonly record references, device/resource/
shared record storage and exported record signatures. Bare scalar FP8 pointers remain unsupported;
physical Storage leaf lowering does not grant pointer admission. Previously supported BF16 local
storage and internal pointer roles remain as before. Newly admitted local-record pointer helper
results are explicitly excluded, independently of internal record-value results.

Generic pointer-result exclusions are verified by code review rather than executed canonical-IR
tests: public source pointer returns lower as UserPointer and do not reach that exact synthetic
shape. This limitation is recorded in270 evidence. Slice270 exercises both Value/Storage cache visitation orders for its flat mixed Payload record, including its BF2 leaf,
with real GPU roundtrips; AlignedPair is storage-first. This is not inferred from the
pointer-result exclusion.

No provider ABI change, FP8 arithmetic or new numerical conversion is introduced. Bit transport
preserves every encoding, including signed zeros, infinities and NaN payloads. Conversion semantics
remain governed by the leaf contracts; raw transport does not grant NaN-payload guarantees to casts.

## Nested store padding

The physical provider splits stores at direct nested-struct boundaries while retaining whole array,
flat struct, scalar and vector stores. Installed libNVVM12.9 miscompiles valid whole nested stores by combining
narrow fields across padding. For `{uint16_t prefix; Child child;}` with
`Child {uint16_t first; uint last;}`, fields at byte0/4 must not become a contiguous uint16x2 store.
The same defect reproduces for already-supported integer records, independently of FP8/BF16.

Provider `_emitStore` validates the original operation before constructing canonical LLVM field
addresses/extractions. LLVM DataLayout determines field offsets, and commonAlignment derives each
field's guarantee from the actual parent alignment. Canonical Slang/LLVM aggregate types, ABI42,
allocation layouts and whole-value loads/transport remain unchanged. This is a bounded target compiler
workaround, not a semantic representation repair. Root arrays and array subtrees stay opaque.

[Slice285](../../issue-nvvm-backend/report.slice-285-nested-array-stores.md) extends the physical store
correction to nested struct boundaries hidden inside those arrays. At a terminal whole store,
`_containsNestedStructLayout` follows canonical array element and struct member types, stopping at
pointers. If an immediate struct-in-struct boundary remains, the store receives the conservative
alignment guarantee of one byte. Allocation/load alignment, the stored SSA value and authored LLVM signatures stay
unchanged. A saved value remains valid even after its source storage changes; no source reread is used.
Array length does not expand provider instructions, and flat-record arrays retain their alignment.

Three integer GPU fixtures qualify root arrays, guarded wrappers and multidimensional wrappers, each
with independent field expectations over all 65,536 low16-bit patterns, fresh out copies, saved values
and both return choices. NVVM O0/O3 pass; NVRTC's separate optimized copy defect remains open. A direct
LLVM annotation-only gate additionally checks constructed/phi values and real alignment-one storage
with canaries. Native serialization tests cover 39 shape/alignment combinations, including 65,536
nested elements represented by one store. This does not admit previously rejected FP8/BF16 record
arrays or establish arbitrary packed/address-space forms. Both original and annotation-only large
O3 modules exceed the same 120-second/4 GiB compile bound; no large GPU execution is claimed.

Packed structs are not constructed by the current provider API. Conservative alignment can change
copy instruction width and generated code; no compile-speed or GPU performance claim follows.

The nested fixture checks all65536 scalar bit patterns, both branch results and both BF2
component indices across three record depths. Its integer-only sibling exercises the provider issue
without substandard types. Natural/CUDA layout checks remain separate; qualification does not claim
both cache visitation orders for these new nested records. Exact synthetic Generic pointer-result
exclusion remains source-reviewed, distinct from public UserPointer negative tests.
