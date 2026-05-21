//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=Boundary: an SRV Texture2D and an UAV RWTexture2D coexist; they bind to `t` and `u` register classes respectively.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: SRV vs UAV register-class split. The same module
// declares both a Texture2D (read-only, `t` class) and an
// RWTexture2D (read-write, `u` class).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float4> srvTex;
RWTexture2D<float4> uavTex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 v = srvTex.SampleLevel(samp, float2(0.5, 0.5), 0.0);
    uavTex[uint2(0, 0)] = v;
    outBuf[tid.x] = v;
}

// CHECK-DAG: Texture2D{{.*}} srvTex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK-DAG: RWTexture2D{{.*}} uavTex_{{[0-9]+}} : register(u{{[0-9]+}})
