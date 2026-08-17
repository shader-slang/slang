//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=6e338f870e1f336f86d9ce297f0955e56a8c161eb5a129e60e2a396aa3aa18b3
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
