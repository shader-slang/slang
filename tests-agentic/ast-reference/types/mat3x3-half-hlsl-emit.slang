//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<half, 3, 3> survives type-checking and lowers to a half matrix type on the HLSL backend.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: square 3x3 shape with half element type. The
// interpreter lacks lane-extraction handlers for half matrices
// (see Compiler-bug observations in README.md), so we observe
// the type via HLSL text emission instead.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<float> outBuf)
{
    matrix<half, 3, 3> m = matrix<half, 3, 3>(
        half(1), half(0), half(0),
        half(0), half(2), half(0),
        half(0), half(0), half(3));
    outBuf[0] = float(m[0][0]) + float(m[1][1]) + float(m[2][2]);
    // CHECK: matrix<half
}
