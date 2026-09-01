//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=5cfb04763d734c87abb0c612990b5db758ffc4cce1785fc806d10db967096a26
//META: purpose=Verifies sampling a Texture2D at uv (1.0, 1.0) lowers to tex2DLod with make_float2(1.0f, 1.0f) on CUDA emit (upper-edge UV).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: Texture2D sample at upper-edge UV (1, 1). The
// `Texture2D` is replaced with `CUtexObject` and the sample call
// becomes `tex2DLod`; CUDA texture sampling at (1,1) is well-
// defined and the literal `1.0f` survives constant folding.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(1.0f, 1.0f), 0);
}

// CHECK: make_float2 (1.0f, 1.0f)
// CHECK: tex2DLod
