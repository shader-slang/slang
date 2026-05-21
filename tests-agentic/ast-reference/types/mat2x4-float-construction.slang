//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<float, 2, 4>: 2 rows by 4 columns, 8 elements addressable via [r][c].
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: rectangular matrix shape (R=2, C=4) at the wide
// edge of the standard column count. Every cell is addressable.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2x4 m = float2x4(1, 2, 3, 4, 5, 6, 7, 8);
    // CHECK: m=1,2,3,4|5,6,7,8
    printf("m=%d,%d,%d,%d|%d,%d,%d,%d\n",
        int(m[0][0]), int(m[0][1]), int(m[0][2]), int(m[0][3]),
        int(m[1][0]), int(m[1][1]), int(m[1][2]), int(m[1][3]));
}
