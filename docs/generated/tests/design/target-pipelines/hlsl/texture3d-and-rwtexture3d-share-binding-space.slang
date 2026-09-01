//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=6e338f870e1f336f86d9ce297f0955e56a8c161eb5a129e60e2a396aa3aa18b3
//META: purpose=Boundary: a Texture3D (SRV) and an RWTexture3D (UAV) in the same module bind to disjoint `t` and `u` register classes.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: the 3D texture variants share dimensionality but
// not register class. The SRV variant lands in `t`, the UAV
// variant lands in `u`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture3D<float4> srcTex;
RWTexture3D<float4> dstTex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 v = srcTex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);
    dstTex[uint3(0, 0, 0)] = v;
    outBuf[tid.x] = v;
}

// CHECK-DAG: Texture3D{{.*}} srcTex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK-DAG: RWTexture3D{{.*}} dstTex_{{[0-9]+}} : register(u{{[0-9]+}})
