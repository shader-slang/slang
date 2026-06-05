//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#floatnonuniformresourceindex
//META: doc_section_digest=e8f525c4f3b39107c481bf03414c7c2ad6a1c96a430e84aae54c4f8e6a580692
//META: purpose=floatNonUniformResourceIndex runs for HLSL (!isSPIRV); the NonUniformResourceIndex(...) intrinsic survives in the HLSL emission of a fragment-stage bindless texture-array access.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Phase C table (row 3) lists floatNonUniformResourceIndex
// with mode `NonUniformResourceIndexFloatMode::Textual` for the
// HLSL `NonUniformResourceIndex(...)` intrinsic. The natural surface
// is a bindless texture-array sample in a graphics-pipeline (fragment)
// shader; the call site must use NonUniformResourceIndex to flag the
// divergent index. The pass keeps the call textually intact on HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage fragment

Texture2D textures[16];
SamplerState samp;

[shader("fragment")]
float4 main(float3 uv : TEXCOORD0) : SV_Target
{
    int idx = int(uv.z);
    return textures[NonUniformResourceIndex(idx)].Sample(samp, uv.xy);
}

// CHECK: NonUniformResourceIndex(
