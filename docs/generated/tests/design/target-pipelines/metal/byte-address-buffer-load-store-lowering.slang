//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=2a4c97aeaacd2a9728a83c1944ef99c7efe30fe3fe60ef075d8d47044f03eefc
//META: purpose=`legalizeByteAddressBufferOps` runs with Metal options (`scalarizeVectorLoadStore`, `lowerBasicTypeOps`), so a `ByteAddressBuffer.Load<uint>` becomes a pointer-indexed read wrapped in `as_type<uint>`.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal option set for byte-address lowering: with
// `lowerBasicTypeOps=true` the raw 32-bit word read out of the buffer has
// to be reinterpreted rather than loaded as a typed element, which on MSL
// is spelled `as_type<T>`. The kernel loads a word and stores a modified
// word so both directions stay live; CHECK asserts both the load-side and
// store-side `as_type<...>` wrappers appear.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

ByteAddressBuffer src;
RWByteAddressBuffer dst;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint v = src.Load<uint>(tid.x * 4);
    dst.Store<uint>(tid.x * 4, v + 1);
}

// CHECK: as_type<uint>
// CHECK: as_type<uint32_t>
