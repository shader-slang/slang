//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=5fecd0eb9a12a63e0294d7fc25d4ccc9d8e7cc1d7f20cc7a08f9fadaa7ec7ae7
//META: purpose=SV_VertexID lowers to gl_VertexIndex with BuiltIn VertexIndex (Vulkan-style index, not D3D-style ID, hence the rename).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vertex-stage builtin. legalizeEntryPointsForGLSL maps
// HLSL SV_VertexID to gl_VertexIndex (Vulkan's index semantic, which
// includes the base-vertex offset). The decoration is "VertexIndex".

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex

RWStructuredBuffer<uint> outBuf;

[shader("vertex")]
float4 main(uint vid : SV_VertexID) : SV_Position
{
    outBuf[vid] = vid;
    return float4(0, 0, 0, 1);
}

// CHECK: OpDecorate %gl_VertexIndex BuiltIn VertexIndex
