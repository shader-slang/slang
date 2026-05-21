//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
//META: purpose=Multiple texture variants (Texture1D / Texture3D / TextureCube / Texture2DArray) collapse to the same CUtexObject CUDA primitive when packed into GlobalParams; the per-rank Lod helper differs.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// All read-only Slang Texture* variants share a single CUDA
// emit type — `CUtexObject` — because the CUDA texture-object
// API is rank-agnostic at storage time and only differs at
// fetch time (tex1DLod, tex3DLod, texCubemapLod,
// tex2DLayeredLod, ...). The per-rank distinction is therefore
// visible only at the call site, not at the GlobalParams field
// declaration.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

Texture1D<float4> tex1d;
Texture3D<float4> tex3d;
TextureCube<float4> texCube;
Texture2DArray<float4> tex2dArr;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 a = tex1d.SampleLevel(samp, float(tid.x), 0);
    float4 b = tex3d.SampleLevel(samp, float3(tid), 0);
    float4 c = texCube.SampleLevel(samp, float3(tid), 0);
    float4 d = tex2dArr.SampleLevel(samp, float3(tid), 0);
    outBuf[tid.x] = a + b + c + d;
}

// CHECK: CUtexObject
// CHECK: CUtexObject
// CHECK: CUtexObject
// CHECK: CUtexObject
// CHECK: tex1DLod
// CHECK: tex3DLod
// CHECK: texCubemapLod
// CHECK: tex2DLayeredLod
