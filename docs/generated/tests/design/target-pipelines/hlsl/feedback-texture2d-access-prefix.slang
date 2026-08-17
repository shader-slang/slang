//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#shape-of-the-emitted-file
//META: doc_section_digest=cd40f48a68f8c4cc01781946c78837919b2702a1a94124525d0a69aadc68ec8c
//META: purpose=The `Feedback` access prefix of the HLSL texture-type composition emits as `FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP>` bound to a `u` register, with its `WriteSamplerFeedback` method spelling carried through unchanged.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the last unexercised row of the access-prefix
// enumeration. `Feedback` differs from the other prefixes in that
// its type argument is a sampler-feedback mode name rather than an
// element type, so the CHECK asserts the mode name survives inside
// the angle brackets rather than being rewritten to an integer, and
// that the resource is classified as unordered-access.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> fb;
Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    fb.WriteSamplerFeedback(tex, samp, float2(0.5, 0.5));
    outBuf[tid.x] = tex.SampleLevel(samp, float2(0.5, 0.5), 0);
}

// CHECK: FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP > fb_{{[0-9]+}} : register(u{{[0-9]+}});
// CHECK: WriteSamplerFeedback
