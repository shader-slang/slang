//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
//META: purpose=RWTexture2D<float4> maps to the CUDA primitive CUsurfObject and subscript-store / subscript-load lower to surf2Dwrite / surf2Dread.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `lowerBuiltinTypesForKernelEntryPoints` rewrites RWTexture2D
// into a CUDA surface-object field (`CUsurfObject`) — distinct
// from the texture-object family used by read-only Texture*. The
// subscript-store / subscript-load on the read-write 2D texture
// lower to `surf2Dwrite<T>(...)` / `surf2Dread<T>(...)`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWTexture2D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid.xy] = float4(1.0);
    outBuf[tid.x] = tex[tid.xy];
}

// CHECK: CUsurfObject
// CHECK: surf2Dwrite
// CHECK: surf2Dread
