//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
//META: purpose=Verifies sampling a Texture2D at uv (0.0, 0.0) lowers to tex2DLod with a make_float2(0.0f, 0.0f) coordinate on CUDA emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: Texture2D sample at lower-edge UV (0, 0). The
// `Texture2D` is replaced with `CUtexObject` and the sample call
// becomes `tex2DLod` with `make_float2(0.0f, 0.0f)`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(0.0f, 0.0f), 0);
}

// CHECK: make_float2 (0.0f, 0.0f)
// CHECK: tex2DLod
