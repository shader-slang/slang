//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=5fecd0eb9a12a63e0294d7fc25d4ccc9d8e7cc1d7f20cc7a08f9fadaa7ec7ae7
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
