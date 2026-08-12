//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=21c9155c4ad2f346c0598d3001ce01acb069faa9bdcbd07e447c0eb95df61d51
//META: purpose=Texture1D<float4> maps to the CUDA primitive CUtexObject and SampleLevel lowers to tex1DLod<T>(...).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `lowerBuiltinTypesForKernelEntryPoints` strips Slang's
// Texture1D type from kernel signatures and replaces it with
// the CUDA primitive `CUtexObject`. A SampleLevel call on
// Texture1D lowers to the `tex1DLod<T>(...)` prelude helper.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

Texture1D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float(tid.x), 0);
}

// CHECK: CUtexObject
// CHECK: tex1DLod
