//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=ByteAddressBuffer.Load<T>/Store<T> lower under Metal's legalizeByteAddressBufferOps options (scalarizeVectorLoadStore=true, lowerBasicTypeOps=true) into pointer-indexed accesses with as_type<T> bitcasts.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal options for `legalizeByteAddressBufferOps`:
// scalarizeVectorLoadStore=true, lowerBasicTypeOps=true,
// translateToStructuredBufferOps=false. The emitted text shows
// pointer-indexed array accesses with `as_type<T>` wrappers.

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
