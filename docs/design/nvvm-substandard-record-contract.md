# NVVM internal substandard-float records

Slice270 qualifies flat internal records containing integer scalar fields together with scalar
FloatE4M3, FloatE5M2, BFloat16 or vector<BFloat16,2>. At least one substandard field is required.
The [slice report](../../issue-nvvm-backend/report.slice-270-fp8-aggregate.md) owns validation and
limitations. Existing [FP8 scalar semantics](nvvm-fp8-scalar-contract.md) and
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

`asNVVMSupportedSubstandardRecordType` owns the flat value-domain classification.
`asNVVMSupportedLocalSubstandardRecordType` combines it with the existing local-only BF16 record
family. Exact Generic local Ptr, OutParam and BorrowInOutParam roots qualify memory access; field
addresses are resolved by canonical keys. No recursive copyable/helper classifier is widened.

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

The new domain excludes nested records, record arrays, readonly record references, device/resource/
shared record storage and exported record signatures. Bare scalar FP8 pointers remain unsupported;
physical Storage leaf lowering does not grant pointer admission. Previously supported BF16 local
storage and internal pointer roles remain as before. Newly admitted local-record pointer helper
results are explicitly excluded, independently of internal record-value results.

Generic pointer-result exclusions are verified by code review rather than executed canonical-IR
tests: public source pointer returns lower as UserPointer and do not reach that exact synthetic
shape. This limitation is recorded in270 evidence. Both Value/Storage cache visitation orders for the mixed Payload record, including its BF2 leaf,
are exercised by real GPU roundtrips; AlignedPair is storage-first. This is not inferred from the
pointer-result exclusion.

No provider ABI change, FP8 arithmetic or new numerical conversion is introduced. Bit transport
preserves every encoding, including signed zeros, infinities and NaN payloads. Conversion semantics
remain governed by the leaf contracts; raw transport does not grant NaN-payload guarantees to casts.
