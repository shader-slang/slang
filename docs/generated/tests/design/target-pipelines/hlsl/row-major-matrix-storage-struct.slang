//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=8041b8729de3f94a856ac55d2d8461ddffd491518349fcd47c2422a784e25a3a
//META: purpose=A row_major matrix in a cbuffer is rewritten into a _MatrixStorage_<spelling>natural_<N> struct on HLSL emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The literal `row_major` keyword does not survive HLSL emit;
// the matrix is reified as a `_MatrixStorage_<dims><nat>_<N>`
// struct that holds a flat array, and the cbuffer field is
// retyped to point at the storage struct.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params {
    row_major float4x4 m;
};
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(m, float4(1, 2, 3, 4));
}

// CHECK: struct _MatrixStorage_float4x4{{.*}}
// CHECK: cbuffer Params_{{[0-9]+}} : register(b{{[0-9]+}})
