# Slice 172: Canonical descriptor-handle AnyValue marshalling

## Motivation

The selected dynamic-dispatch workloads store a structured-buffer handle in a concrete interface
payload. One representative is:

```slang
struct BufferSource : IDataSource
{
    StructuredBuffer<float>.Handle dataHandle;
    float offset;

    float getValue(int index)
    {
        return dataHandle[index] + offset;
    }
}
```

Native CUDA compiled and ran the array, dispatch, and two-handle variants correctly. Direct NVVM
failed in both O0 and O3 with `Unsupported value size`. The census output contract categorized
that abort as a runtime mismatch, but its preserved compiler diagnostic identified the first
actual failing operation: AnyValue's 16-byte descriptor payload reached aggregate bit-cast
lowering as one opaque leaf.

## Proposed solution

Preserve the canonical `DescriptorHandle<StructuredBuffer<T>> <-> uint4` bit cast until the direct
emitter has lowered the handle to its real provider type. Classify that one bidirectional shape
exactly. Legalize it in the compiler with existing generic aggregate extraction/construction,
vector extraction/construction, pointer-bit transport, integer conversion, shift, and bitwise-or
operations. Do not revise provider ABI revision 32.

This boundary is target-owned rather than a workaround for malformed IR. Common AnyValue
marshalling intentionally emits the bit cast because CUDA source represents the handle as its
native resource value. Common IR intentionally keeps the handle opaque. Only direct type lowering
knows that the selected raw-buffer value is `{global T*, uint64 count}` and can decompose it
without inventing a second semantic representation.

## Change summary

- Direct-only aggregate bit-cast normalization preserves the exact 16-byte unsigned descriptor
  payload operation instead of attempting to scalarize an opaque handle.
- Direct NVVM preflight recognizes only a supported raw-buffer descriptor and unsigned `uint4`
  payload in either direction, and records every integer recipe operation before provider use.
- Emission splits/reconstructs the pointer and 64-bit count using existing revision-32 operations.
- The three fixtures gain permanent direct O0/O3 differential lanes.
- Frozen/discovery TSV and Pareto JSON, a three-gate measurement manifest, design documentation,
  the capability ledger, plan, and this report retain the evidence.

## Concepts and vocabulary

**Descriptor handle** is the opaque semantic `DescriptorHandle<T>` value. On CUDA bindless
targets it has the native physical representation of `T`; it is not an integer descriptor index.

**AnyValue payload** is the fixed unsigned-word storage used to transport a concrete value through
dynamic interface dispatch.

**Raw-buffer view** is the direct provider representation `{global element*, uint64 count}` used
for structured and byte-address buffers.

## Process report

The input-shape audit began at `AnyValueMarshallingContext::emitMarshallingCode`. For a bindless
descriptor, `TypePackingContext::marshalDescriptorHandle` loads the native handle and bit-casts it
to enough UInt32 lanes to cover its target layout. The inverse
`TypeUnpackingContext::marshalDescriptorHandle` constructs those words and bit-casts them back.
For a structured buffer, `getNaturalSizeAndAlignment` returns 16 bytes, so the exact canonical IR
operations are `bitCast<DescriptorHandle<StructuredBuffer<float>>, vector<uint,4>>` and its
inverse. This producer is shared by all three workloads; no source or fixture name participates.

The next consumer was `lowerBitCast`, invoked again by `CodeGenContext::emitNVVMForEntryPoints`
because AnyValue helpers are generated after the earlier common pass inventory. `readObject`
correctly decomposes the `uint4` result, but `extractValueAtOffset` sees the opaque source handle as
one 16-byte leaf. `bitCastLeafValue` only supports scalar leaves of one, two, four, or eight bytes
and therefore raises `Unsupported value size`. Extending that scalar helper to 16 bytes would be
wrong: the first eight bytes are a typed pointer, not an integer scalar leaf.

The retained pass change is deliberately narrow. When direct NVVM sees a same-size bit cast
between a descriptor handle and unsigned `uint4`, aggregate bit-cast normalization leaves it for
the target emitter. Existing eight-byte texture/sampler handle paths continue through their prior
normalization, and other payload types are unchanged. Direct preflight then calls
`_getNVVMRawBufferDescriptorBitCast`, which additionally requires
`asNVVMSupportedDescriptorHandleType`, `getNVVMSupportedRawBufferType`, unsigned 32-bit lanes, and
exactly four lanes. A merely 16-byte unsupported handle does not acquire support from the pass
exception; it receives the existing exact preflight diagnostic.

`NVVMTypeLoweringContext::_lowerRawBufferType` is the physical source of truth. It lowers the
selected handle/resource to `{global element*, i64}`. Packing extracts fields zero and one,
transports the pointer to `uint2`, splits the i64 count with conversion and logical right shift,
and constructs `uint4`. Unpacking extracts all four words, reconstructs the pointer through the
same `uint2` contract, zero-extends and combines the count words, and constructs the exact lowered
handle aggregate. The original semantic bit cast is therefore implemented as a physical ABI
recipe only after its type is known.

The self-review inventory contains three retained specializations. The lower-bit-cast deferral
survives because removing it reproduces `Unsupported value size` before preflight, and it is
restricted to the producer's complete unsigned 16-byte shape. The classifier survives because
removing it changes the same probes to deterministic `bitCast type: DescriptorHandle ->
vector<uint,4>` preflight failures, proving direct representation legalization owns the operation.
The emission recipe survives because removing either the pointer or count half prevents the
native resource view from round-tripping; all three differential fixtures exercise recovered
buffer access, arrays, and multiple fields. No fallback, syntax reconstruction, operand-graph
search, fixture check, or provider callback was added.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy references. Healthy correctness
advances from 409/409/409 to 412/412/412 O0/O3/both, with exactly the three selected gains and zero
old-correct regression. All-row direct totals become 426 correct, four runtime mismatches, and 22
preflight failures in each mode. Discovery remains exactly 82 workloads/72 healthy references and
69/69/69, with no changed row.

The selected regression prefix passes 433/433 and the permanent `nvvm` category passes 72/72.
Every promoted gate compiles and assembles through CUDA 12.9 for native NVRTC, direct O0 SM70, and
direct O3 SM70/SM80/SM90. At SM70, direct O3 PTX is 3,014 bytes versus 11,385 native for the array
workload, 2,523 versus 10,784 for dispatch, and 3,013 versus 11,405 for the two-handle workload.
Median standalone compile times remain exploratory rather than controlled benchmarks.
