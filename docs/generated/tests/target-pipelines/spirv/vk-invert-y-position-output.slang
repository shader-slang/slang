//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=ea1dc5e43dbe24afedb64d7530802c7f0537a68697032d3fc655a1128601e739
//META: purpose=invertYOfPositionOutput fires under -fvk-invert-y for a vertex entry point — the SV_Position output's Y component is negated before being written to gl_Position, observable as an OpFNegate / OpFSub on the y lane in the emitted SPIR-V.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the option-set toggle VulkanInvertY gates the
// invertYOfPositionOutput pass (Phase C, row 23). With
// -fvk-invert-y on a vertex entry point, the SV_Position output
// has its Y negated so that the same Slang/HLSL coordinate system
// targets Vulkan's flipped clip space. The emitted SPIR-V still
// declares the Vertex execution model and the gl_Position BuiltIn,
// but the y-component is no longer the raw constant the user wrote.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex -fvk-invert-y

[shader("vertex")]
float4 main(uint vid : SV_VertexID) : SV_Position
{
    return float4(0.0, 1.0, 0.0, 1.0);
}

// CHECK: OpEntryPoint Vertex %main
// CHECK-DAG: OpDecorate %gl_Position BuiltIn Position
// CHECK-DAG: OpFNegate
