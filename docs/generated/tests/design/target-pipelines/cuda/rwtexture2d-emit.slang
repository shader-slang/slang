//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=568f76ee2394f54152926bb0c6b02911e89b9fe2666189b0e859fd9e3af15aa1
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
