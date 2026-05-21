//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
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
