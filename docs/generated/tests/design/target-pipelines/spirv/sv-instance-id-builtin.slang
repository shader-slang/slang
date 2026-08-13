//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=94ee0f151ec728ab5cca6e847c6705a05442b8b88f430ce9485fc71aeb34cc94
//META: purpose=SV_InstanceID lowers to gl_InstanceIndex with BuiltIn InstanceIndex (Vulkan-style index).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: a second vertex-stage builtin from the
// legalizeEntryPointsForGLSL table. SV_InstanceID maps to
// gl_InstanceIndex (the Vulkan instance index, which adds
// gl_BaseInstance to the raw id).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex

RWStructuredBuffer<uint> outBuf;

[shader("vertex")]
float4 main(uint vid : SV_VertexID, uint iid : SV_InstanceID) : SV_Position
{
    outBuf[vid] = iid;
    return float4(0, 0, 0, 1);
}

// CHECK: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
