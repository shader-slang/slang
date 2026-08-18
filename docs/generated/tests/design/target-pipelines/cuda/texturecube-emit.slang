//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=5cfb04763d734c87abb0c612990b5db758ffc4cce1785fc806d10db967096a26
//META: purpose=TextureCube<float4> maps to the CUDA primitive CUtexObject and SampleLevel lowers to texCubemapLod<T>(...).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `lowerBuiltinTypesForKernelEntryPoints` replaces Slang's
// TextureCube with the CUDA primitive `CUtexObject`. A
// SampleLevel call on TextureCube lowers to the
// `texCubemapLod<T>(...)` prelude helper.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

TextureCube<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(tid), 0);
}

// CHECK: CUtexObject
// CHECK: texCubemapLod
