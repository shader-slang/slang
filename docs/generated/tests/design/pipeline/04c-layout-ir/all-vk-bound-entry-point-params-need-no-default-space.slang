//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#explicit-vkbinding-on-entry-point-parameters
//META: doc_section_digest=d594d25adb697c7a448d8fd4e5af2ada74a9ee315fdd20515276d88cbf22323f
//META: purpose=An entry point whose parameters are all explicitly vk::binding-bound no longer forces a default descriptor space, leaving set 0 available.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the default-space calculation subtracts resource kinds that
// explicit annotations already placed. The entry point's only parameter is
// pinned to set 6, and a bindless descriptor handle asks for the heap in
// set 0. CHECK asserts the heap really gets set 0 and that the
// unavailable-space warning never fires - both only hold if no default set
// was reserved on the entry point's behalf.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute -bindless-space-index 0

[shader("compute")]
[numthreads(1, 1, 1)]
void main([[vk::binding(2, 6)]] uniform RWStructuredBuffer<int> outBuf)
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(uint2(0, 0));
    outBuf[0] = int(texture.Load(int3(0, 0, 0)).x);
}

// SPV-NOT: warning{{.*}}39012
// SPV-DAG: OpDecorate %__slang_resource_heap{{[A-Za-z0-9_]*}} DescriptorSet 0
// SPV-DAG: OpDecorate %entryPointParams_outBuf{{[A-Za-z0-9_]*}} DescriptorSet 6
