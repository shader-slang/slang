//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<float, 4, 2>: 4 rows by 2 columns, 8 elements addressable via [r][c].
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: rectangular matrix shape (R=4, C=2). This is the
// transpose-shape of float2x4; both must be valid distinct
// MatrixExpressionType operand triples.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4x2 m = float4x2(1, 2, 3, 4, 5, 6, 7, 8);
    // CHECK: m=1,2|3,4|5,6|7,8
    printf("m=%d,%d|%d,%d|%d,%d|%d,%d\n",
        int(m[0][0]), int(m[0][1]),
        int(m[1][0]), int(m[1][1]),
        int(m[2][0]), int(m[2][1]),
        int(m[3][0]), int(m[3][1]));
}
