//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
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
