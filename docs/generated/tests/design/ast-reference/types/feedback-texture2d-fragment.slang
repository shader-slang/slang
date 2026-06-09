//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource--texture--sampler-types
//META: doc_section_digest=1fea6aaeec3d54a63a589bef03489a7dd7676c39ed93549751670b987cd46065
//META: purpose=A FeedbackType is the AST type for sampler-feedback textures (`FeedbackTexture2D<...>`); the HLSL backend renders the user spelling in a fragment entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `FeedbackType` is the `BuiltinType` leaf for sampler-feedback
// resource types (`FeedbackTexture2D<...>`, `FeedbackTexture2DArray<...>`).
// Observable via HLSL emit text at a fragment entry point: the
// resource declaration survives lowering with its surface name.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage fragment -entry main -profile sm_6_5

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> feedbackTex;
Texture2D<float> tex;
SamplerState samp;

//CHECK: FeedbackTexture2D
//CHECK: WriteSamplerFeedback
float4 main() : SV_Target
{
    feedbackTex.WriteSamplerFeedback(tex, samp, float2(0.0, 0.0));
    return float4(1, 0, 0, 1);
}
