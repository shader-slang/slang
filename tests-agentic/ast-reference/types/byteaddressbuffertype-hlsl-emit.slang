//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#buffer-types
//META: doc_section_digest=c25174867f65b28674f6e50925bd9f916f322862f95fe848ae8bb96cd35ce36b
//META: purpose=An HLSLByteAddressBufferType uniform reaches the HLSL backend as `ByteAddressBuffer` and `RWByteAddressBuffer`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLByteAddressBufferType` and `HLSLRWByteAddressBufferType` are
// the `UntypedBufferResourceType` leaves for `ByteAddressBuffer` /
// `RWByteAddressBuffer`. The user-facing claim: each spelling lowers
// to its HLSL surface form.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ByteAddressBuffer g_in;
RWByteAddressBuffer g_out;

//CHECK: ByteAddressBuffer
//CHECK: RWByteAddressBuffer
[numthreads(1, 1, 1)]
void main()
{
    uint v = g_in.Load(0);
    g_out.Store(0, v);
}
