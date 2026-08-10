//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=21c9155c4ad2f346c0598d3001ce01acb069faa9bdcbd07e447c0eb95df61d51
//META: purpose=Verifies sampling a Texture2D at a NaN coordinate (gated by tid.x to prevent fold) still lowers through tex2DLod on CUDA emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: Texture2D sample at undefined UV (NaN). Slang does
// not diagnose the value at compile time; the documented behavior
// is that the lowering proceeds as for any other float and CUDA
// runtime semantics define the result.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float uv = (tid.x == 0U) ? (0.0f / 0.0f) : float(tid.x);
    outBuf[tid.x] = tex.SampleLevel(samp, float2(uv, uv), 0);
}

// CHECK: tex2DLod
