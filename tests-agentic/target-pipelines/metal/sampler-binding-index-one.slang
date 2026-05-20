//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Boundary: a second SamplerState in declaration order receives `[[sampler(1)]]`; the sampler slot space progresses independently of `[[texture(N)]]` and `[[buffer(N)]]`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: sampler index 1. Two samplers share one texture;
// `samp0` gets `[[sampler(0)]]`, `samp1` gets `[[sampler(1)]]`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp0;
SamplerState samp1;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp0, float2(tid.xy), 0)
                  + tex.SampleLevel(samp1, float2(tid.xy), 0);
}

// CHECK: sampler(0)
// CHECK-SAME: sampler(1)
