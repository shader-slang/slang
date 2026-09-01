//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#vkinput_attachment_index-and-descriptor-space-occupancy
//META: doc_section_digest=1e74211450b5d4654bbcd551798ef0ce125da959ab9ff5e6706a38ca792f9335
//META: purpose=An input-attachment index does not occupy a descriptor set, so a SubpassInput entry-point parameter leaves set 0 available to the bindless descriptor heap.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the placeholder space carried by an input-attachment index
// is not treated as a real descriptor-set request. The SubpassInput
// parameter is explicitly bound at set 3 while its attachment index uses
// the placeholder; CHECK asserts the bindless heap still obtains the
// requested set 0 with no unavailable-space warning, and that the
// attachment itself keeps binding 7 / set 3 / index 5.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage fragment -bindless-space-index 0

[shader("fragment")]
float4 main(
    [[vk::binding(7, 3)]]
    [[vk::input_attachment_index(5)]]
    SubpassInput<float4> input)
    : SV_Target
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(uint2(0, 0));
    return input.SubpassLoad() + texture.Load(int3(0, 0, 0));
}

// SPV-NOT: warning{{.*}}39012
// SPV-DAG: OpDecorate %__slang_resource_heap{{[A-Za-z0-9_]*}} DescriptorSet 0
// SPV-DAG: OpDecorate %entryPointParams_input{{[A-Za-z0-9_]*}} Binding 7
// SPV-DAG: OpDecorate %entryPointParams_input{{[A-Za-z0-9_]*}} DescriptorSet 3
// SPV-DAG: OpDecorate %entryPointParams_input{{[A-Za-z0-9_]*}} InputAttachmentIndex 5
