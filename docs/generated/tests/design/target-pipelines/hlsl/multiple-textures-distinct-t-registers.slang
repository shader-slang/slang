//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Boundary: three textures of different shape (1D / 3D / Cube) bind to consecutive `t` registers when used in the same module.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: heterogeneous texture variants sharing the SRV
// register space. Each native texture type emits with its own
// `register(tN)` slot.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture1D<float4> tex1;
Texture3D<float4> tex3;
TextureCube<float4> texCube;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 a = tex1.SampleLevel(samp, 0.5, 0.0);
    float4 b = tex3.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);
    float4 c = texCube.SampleLevel(samp, float3(0.0, 1.0, 0.0), 0.0);
    outBuf[tid.x] = a + b + c;
}

// CHECK-DAG: Texture1D{{.*}} tex1_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK-DAG: Texture3D{{.*}} tex3_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK-DAG: TextureCube{{.*}} texCube_{{[0-9]+}} : register(t{{[0-9]+}})
