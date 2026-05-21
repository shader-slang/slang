//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
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
