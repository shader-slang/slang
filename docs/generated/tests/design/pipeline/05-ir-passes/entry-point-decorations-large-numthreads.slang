//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=53d52927e511301064adeecd90563a0571f0588af4d96406f90dc4612a87a935
//META: purpose=Boundary: entry-point-decorations surfaces a large `[numthreads(64, 1, 1)]` value on HLSL — alternative numthreads axis to the (1,1,1) and small default cases.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Numthreads value axis on entry-point-decorations: a workgroup of
// 64 threads per X dimension surfaces in the per-target entry-point
// marker.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = tid.x;
}

// HLSL: numthreads(64
// GLSL: local_size_x = 64
