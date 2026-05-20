//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType carries independent row and column counts; a 3-row by 2-column matrix has the documented rows*cols=6 elements addressable.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the rectangular case of MatrixExpressionType:
// rows != cols. A `float3x2` has 3 rows and 2 columns, addressable
// via [r][c] where r in 0..2 and c in 0..1.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float3x2 m = float3x2(1, 2, 3, 4, 5, 6);
    // CHECK: m=1,2|3,4|5,6
    printf("m=%d,%d|%d,%d|%d,%d\n",
        int(m[0][0]), int(m[0][1]),
        int(m[1][0]), int(m[1][1]),
        int(m[2][0]), int(m[2][1]));
}
