//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Boundary: two textures share one sampler — texture indices reach `[[texture(0)]]` and `[[texture(1)]]` while the sampler stays at `[[sampler(0)]]` in its independent slot space.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: texture/sampler pair indices. `[[texture(N)]]`
// and `[[sampler(N)]]` are independent slot spaces. Two textures
// each get their own texture index; a single sampler stays at 0.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture2D<float4> tex0;
Texture2D<float4> tex1;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex0.SampleLevel(samp, float2(tid.xy), 0)
                  + tex1.SampleLevel(samp, float2(tid.xy), 0);
}

// CHECK: texture(0)
// CHECK-SAME: sampler(0)
// CHECK-SAME: texture(1)
