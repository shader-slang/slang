//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Texture2D<T> and SamplerState declared separately survive as separate texture2d<T,sample> and sampler parameters on Metal.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal natively supports separate textures and samplers. A
// Slang `Texture2D<T>` + `SamplerState` pair emits as
// `texture2d<T, access::sample>` and `sampler`, each with their
// own positional binding slot.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(tid.xy), 0);
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: texture2d<float, access::sample> tex_{{[0-9]+}}
// CHECK-SAME: texture(
// CHECK-SAME: sampler samp_{{[0-9]+}}
// CHECK-SAME: sampler(
