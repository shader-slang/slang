//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=a4ae0c2435c0e77be2b2eff75eb235733a799b1c91b73d70aa184d15c167c6d0
//META: purpose=Sampler2D (combined texture+sampler) lowers to separate texture2d<T,sample>/sampler parameter pair with [[texture(N)]] and [[sampler(N)]].
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal is in the lowerCombinedTextureSamplers arm. A Slang
// `Sampler2D` splits into a `texture2d<float, access::sample>`
// param and a separate `sampler` param, named
// `combined_texture_<N>` and `combined_sampler_<N>`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Sampler2D combined;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = combined.SampleLevel(float2(tid.xy), 0);
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: texture2d<float, access::sample> combined_texture_{{[0-9]+}}
// CHECK-SAME: texture(
// CHECK-SAME: sampler combined_sampler_{{[0-9]+}}
// CHECK-SAME: sampler(
