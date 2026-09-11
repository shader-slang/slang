//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#shape-of-the-emitted-file
//META: doc_section_digest=cd40f48a68f8c4cc01781946c78837919b2702a1a94124525d0a69aadc68ec8c
//META: purpose=The `#pragma pack_matrix(...)` argument follows the compile's matrix-layout mode, so `-matrix-layout-row-major` opens the artifact with `pack_matrix(row_major)` instead of the `slangc` default `column_major`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the front matter's `pack_matrix` argument is not a
// constant but a projection of the requested matrix layout mode.
// The sibling `prelude-pack-matrix-pragma.slang` pins the `slangc`
// default; this file compiles the same cbuffer matrix with
// `-matrix-layout-row-major` and CHECKs that the very first emitted
// line reads `row_major`, with a CHECK-NOT ruling out the default
// spelling anywhere in the artifact.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -matrix-layout-row-major

cbuffer Params
{
    float4x4 m;
};
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(m, float4(1, 2, 3, 4));
}

// CHECK: #pragma pack_matrix(row_major)
// CHECK-NOT: pack_matrix(column_major)
