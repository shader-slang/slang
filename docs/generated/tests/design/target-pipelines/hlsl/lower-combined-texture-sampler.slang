//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#lowercombinedtexturesamplers
//META: doc_section_digest=2c12c182458c73ae06317619cf488687f97dcb4e230c94df829602fac6ac0eaf
//META: purpose=Slang's Sampler2D is split by lowerCombinedTextureSamplers into a separable Texture2D + SamplerState pair in HLSL emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL declares textures and samplers separately. The IR pass
// `lowerCombinedTextureSamplers` rewrites a single
// `Sampler2D` parameter into a `combined_texture_<N>` /
// `combined_sampler_<N>` pair, each with its own register.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Sampler2D combined;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = combined.SampleLevel(float2(0.5, 0.5), 0);
}

// CHECK-DAG: Texture2D{{.*}} combined_texture_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK-DAG: SamplerState combined_sampler_{{[0-9]+}} : register(s{{[0-9]+}})
