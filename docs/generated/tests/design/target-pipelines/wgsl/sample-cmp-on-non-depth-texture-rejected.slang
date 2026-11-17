//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:39:19+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#texture-sampler-and-texture-intrinsic-spellings
//META: doc_section_digest=3a94acd8dd9a59970d7d0e3c3f2fe42bd83171ea939d74747983a3775ddfe2eb
//META: purpose=SampleCmp on a non-depth texture is rejected by the static_assert inside the WGSL intrinsic body
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target wgsl -entry main -stage compute

Texture2D<float> tex;
SamplerComparisonState cs;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleCmpLevelZero(cs, float2(0.5,0.5), 0.5f);
}

// CHECK: E41400
// CHECK: WGSL comparison sampling
