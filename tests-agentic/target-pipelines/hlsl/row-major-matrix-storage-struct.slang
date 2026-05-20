//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=99399bec1655fc5eb538f2f70fe7d6d9093a89bbf1f86fbe5246b36cbfc141d8
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
