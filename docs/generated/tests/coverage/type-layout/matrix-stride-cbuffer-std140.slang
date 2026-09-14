//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=e885f1a92beacc0053db90ae24923135a05a098bfc3c282c2580a971645fcec9
//META: purpose=Verifies a float2x3 in a ConstantBuffer is decorated RowMajor with MatrixStride 16 under std140, so a following scalar field lands at offset 48.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the matrix-layout path (GetMatrixLayout, which lays a matrix out as an
// array of its rows under the std140 element rules) for a `float2x3` field in
// a ConstantBuffer. The matrix is emitted with a RowMajor decoration and a
// MatrixStride of 16 (each of the 2 rows padded to a 16-byte std140 array
// stride), giving the matrix a 32-byte footprint, after which the rounding to
// the next 16-byte boundary places the trailing `float tail` at offset 48.
// Verbatim from spirv-asm emit.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

struct S { float2x3 m; float tail; }
ConstantBuffer<S> cb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.m[0][0] + cb.tail;
}

// std140 cbuffer matrix: RowMajor, MatrixStride 16, tail at 48.
//SPV-DAG: OpMemberDecorate %S_std140 0 Offset 0
//SPV-DAG: OpMemberDecorate %S_std140 0 RowMajor
//SPV-DAG: OpMemberDecorate %S_std140 0 MatrixStride 16
//SPV-DAG: OpMemberDecorate %S_std140 1 Offset 48
