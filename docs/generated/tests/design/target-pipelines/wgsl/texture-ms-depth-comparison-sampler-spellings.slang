//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:39:19+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#texture-sampler-and-texture-intrinsic-spellings
//META: doc_section_digest=3a94acd8dd9a59970d7d0e3c3f2fe42bd83171ea939d74747983a3775ddfe2eb
//META: purpose=Texture2DMS emits texture_multisampled_2d<f32>, a depth texture emits texture_depth_2d with no type parameter, SamplerComparisonState emits sampler_comparison, and SampleCmpLevelZero maps to textureSampleCompareLevel
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

Texture2DMS<float4> t;
DepthTexture2D sh;
SamplerComparisonState cs;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = t.Load(int2(1,1), 0) + sh.SampleCmpLevelZero(cs, float2(0.5,0.5), 0.5f);
}

// CHECK: var t_0 : texture_multisampled_2d<f32>;
// CHECK: var sh_0 : texture_depth_2d;
// CHECK: var cs_0 : sampler_comparison;
// CHECK: textureSampleCompareLevel
