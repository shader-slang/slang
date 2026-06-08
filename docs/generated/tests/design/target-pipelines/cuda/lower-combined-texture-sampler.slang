//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
//META: purpose=lowerBuiltinTypesForKernelEntryPoints replaces Texture2D with the CUDA primitive CUtexObject in kernel-related signatures.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Row 20 of the Phase B table:
// `lowerBuiltinTypesForKernelEntryPoints` strips Slang shader types
// from kernel signatures, replacing `Texture2D` with the CUDA
// primitive `CUtexObject` (and `SamplerState` is kept by name).

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(0, 0), 0);
}

// CHECK: CUtexObject
// CHECK: tex2DLod
