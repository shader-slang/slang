//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#parameter-binding-producing-the-programlayout
//META: doc_section_digest=7da97e6272e7230be6aa860fcb219926e5ecf2fe260c372906e4be02b8c61abb
//META: purpose=When the requested bindless space is already occupied the binding pass warns with E39012 and scans upward for the first unused space.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the failure arm of the bindless-heap placement stage. Set 0 is
// genuinely occupied by an explicitly bound global, so requesting the heap
// there cannot be satisfied: CHECK asserts the E39012 warning fires and
// that the heap ends up in the next free space, set 1 - the "scan upward
// from the requested index" behaviour.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage fragment -bindless-space-index 0

[[vk::binding(0, 0)]] RWStructuredBuffer<float4> outBuf;

[shader("fragment")]
float4 main() : SV_Target
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(uint2(0, 0));
    outBuf[0] = float4(0);
    return texture.Load(int3(0, 0, 0));
}

// SPV-DAG: warning{{.*}}39012
// SPV-DAG: OpDecorate %__slang_resource_heap{{[A-Za-z0-9_]*}} DescriptorSet 1
