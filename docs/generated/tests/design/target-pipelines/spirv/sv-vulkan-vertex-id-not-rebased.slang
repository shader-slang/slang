//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=f8f9c6c9ed941a5154d83b06809c18c71fb5d69313dfbb7b6096dcb44af0485e
//META: purpose=Boundary: `SV_VulkanVertexID` maps straight to the bare `gl_VertexIndex` and is not rebased, so neither the subtraction nor the `DrawParameters` capability appears.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that makes the rebasing opt-out-able: a user who
// wants Vulkan's own index asks for it by name and gets it untouched. This is
// the negative half of the SV_VertexID pair -- the leading CHECK-NOT covers
// the capability block that precedes the decorations, and the trailing one
// covers the function body where the subtraction would otherwise appear.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex

[shader("vertex")]
float4 main(uint vid : SV_VulkanVertexID) : SV_Position
{
    return float4(float(vid), 0, 0, 1);
}

// CHECK-NOT: OpCapability DrawParameters
// CHECK: BuiltIn VertexIndex
// CHECK-NOT: OpISub
