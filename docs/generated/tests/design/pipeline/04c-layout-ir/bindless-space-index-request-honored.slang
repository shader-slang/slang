//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#option-gates
//META: doc_section_digest=bea2fb603c1de20170ed67ed6232c995842edb98c845f257d62a1f812d1fca63
//META: purpose=The -bindless-space-index option places the system bindless resource heap in the requested descriptor space when that space is unused.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the option gate that feeds the bindless-heap placement stage.
// The shader occupies set 0 with an ordinary global, then asks for the heap
// in set 5; CHECK asserts the heap variable is decorated with
// DescriptorSet 5 and that no unavailable-space warning is produced, so the
// request was satisfied rather than silently relocated.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage fragment -bindless-space-index 5

[[vk::binding(0, 0)]] RWStructuredBuffer<float4> outBuf;

[shader("fragment")]
float4 main() : SV_Target
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(uint2(0, 0));
    outBuf[0] = float4(0);
    return texture.Load(int3(0, 0, 0));
}

// SPV-NOT: warning{{.*}}39012
// SPV: OpDecorate %__slang_resource_heap{{[A-Za-z0-9_]*}} DescriptorSet 5
