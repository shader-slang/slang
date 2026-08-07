//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=21c9155c4ad2f346c0598d3001ce01acb069faa9bdcbd07e447c0eb95df61d51
//META: purpose=RWTexture3D<float4> maps to the CUDA primitive CUsurfObject and subscript-store / subscript-load lower to surf3Dwrite / surf3Dread.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Read/write 3D textures lower to the surface-object family on
// CUDA: `lowerBuiltinTypesForKernelEntryPoints` rewrites the
// RWTexture3D parameter into a `CUsurfObject` field on
// GlobalParams, and the subscript operators lower to the
// CUDA `surf3Dwrite<T>(...)` / `surf3Dread<T>(...)` prelude
// helpers.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWTexture3D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid] = float4(1.0);
    outBuf[tid.x] = tex[tid];
}

// CHECK: CUsurfObject
// CHECK: surf3Dwrite
// CHECK: surf3Dread
