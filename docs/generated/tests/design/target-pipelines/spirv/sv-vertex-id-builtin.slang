//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=3c3c8f77c7e1de8cb96354a3387d70417eaefe0edb55f1e0d96b67d31d068296
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
