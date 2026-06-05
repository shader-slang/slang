//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType at the smallest standard square dim (2x2) holds 2*2=4 elements addressable via [r][c].
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the small end of MatrixExpressionType:
// a `float2x2` square matrix has 4 elements, each independently
// addressable. Complements the existing `float2x3` smoke
// (matrixtype-row-col).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2x2 m = float2x2(1, 2, 3, 4);
    // CHECK: m=1,2,3,4
    printf("m=%d,%d,%d,%d\n",
        int(m[0][0]), int(m[0][1]),
        int(m[1][0]), int(m[1][1]));
}
