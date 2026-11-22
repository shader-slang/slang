//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=bdab7c96cac9a55381381f37622841c41f387f80bb1577daf4c1cc5d5aa46b5d
//META: purpose=Under `-matrix-layout-row-major` a `column_major float3x4` constant-buffer field is the one that differs from the default, so it is the one wrapped — as `_MatrixStorage_float3x4_ColMajornatural_<N>` whose storage field is `float3 data_<M>[int(4)]`.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the storage-struct rewrite keys off "differs from
// the compile's default layout", not off the `row_major` keyword,
// and that the optional `_ColMajor` infix appears exactly when the
// wrapped matrix is column-major. Compiling a `column_major` field
// under `-matrix-layout-row-major` is the mirror image of the
// default-layout case in
// `matrix-storage-natural-float3x4-field-shape.slang`: the CHECKs
// assert the `_ColMajor` infix is present and that the storage
// extent has flipped to one `float3` per column.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -matrix-layout-row-major

struct P
{
    column_major float3x4 m;
};
ConstantBuffer<P> gCB;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(float3(1, 2, 3), gCB.m);
}

// CHECK: struct _MatrixStorage_float3x4_ColMajornatural_{{[0-9]+}}
// Four columns, each stored as one `float3`.
// CHECK: float3{{ +}}data_{{[0-9]+}}[int(4)];
// CHECK: cbuffer gCB_{{[0-9]+}} : register(b0)
