//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#shape-of-the-emitted-file
//META: doc_section_digest=cd40f48a68f8c4cc01781946c78837919b2702a1a94124525d0a69aadc68ec8c
//META: purpose=Boundary: the sample count is appended to the composed multisampled texture type only when it is non-zero, so `Texture2DMS<float4>` emits with no count while `Texture2DMS<float4, 4>` keeps its `4`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the zero/non-zero carve-out in the texture-type
// composition rule, which the existing `Texture2DMS` tests cannot
// see because both of them declare an explicit count. Declaring the
// countless and counted spellings side by side in one module makes
// the difference a property of the type rather than of the compile:
// the CHECKs assert `<float4 >` closes immediately for the first
// and that only the second carries `, 4`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2DMS<float4> msNoCount;
Texture2DMS<float4, 4> msCount4;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = msNoCount.Load(int2(0, 0), 0) + msCount4.Load(int2(0, 0), 3);
}

// CHECK-DAG: {{^}}Texture2DMS<float4 > msNoCount_{{[0-9]+}} : register(t{{[0-9]+}});
// CHECK-DAG: {{^}}Texture2DMS<float4, 4 > msCount4_{{[0-9]+}} : register(t{{[0-9]+}});
