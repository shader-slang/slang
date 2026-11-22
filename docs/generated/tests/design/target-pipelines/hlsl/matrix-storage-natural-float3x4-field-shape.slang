//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=bdab7c96cac9a55381381f37622841c41f387f80bb1577daf4c1cc5d5aa46b5d
//META: purpose=A `row_major float3x4` field of a `ConstantBuffer` element struct becomes `struct _MatrixStorage_float3x4natural_<N> { float4 data_<M>[int(3)]; }` — the layout rule spells `natural` on HLSL and the storage field is an array of one row-vector per row.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the full name and body of the storage struct that a
// buffer-reached matrix is rewritten into when its layout differs
// from the compile default. `row-major-matrix-storage-struct.slang`
// only pins the `_MatrixStorage_float4x4` prefix; this file uses the
// non-square `float3x4` so the CHECKs can assert three separate
// facts the prefix alone cannot: the `natural` layout-rule suffix
// that HLSL spells, the absence of a `_ColMajor` infix under the
// default column-major compile, and the `float4 data[int(3)]`
// element shape (one vector per row, row count in the extent).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct P
{
    row_major float3x4 m;
};
ConstantBuffer<P> gCB;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(float3(1, 2, 3), gCB.m);
}

// The storage struct carries the layout-rule suffix, not a `row_major` keyword.
// CHECK: struct _MatrixStorage_float3x4natural_{{[0-9]+}}
// CHECK-NOT: _ColMajor
// One `float4` per row of the 3x4 matrix.
// CHECK: float4{{ +}}data_{{[0-9]+}}[int(3)];
// CHECK: cbuffer gCB_{{[0-9]+}} : register(b0)
