//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#lowercombinedtexturesamplers
//META: doc_section_digest=5307663eae1bdb76f3faae002c59b8e3c52023856a44b77f5bd5a54df4270d41
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
