//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<double, 2, 3>: rectangular shape with double element type; cells round-trip via int(...) widening.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: rectangular shape (R=2, C=3) with double element
// type. Confirms the documented (element-type, rows, cols)
// encoding handles 64-bit float at a non-square shape.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    matrix<double, 2, 3> m = matrix<double, 2, 3>(
        1.0lf, 2.0lf, 3.0lf,
        4.0lf, 5.0lf, 6.0lf);
    // CHECK: m=1,2,3|4,5,6
    printf("m=%d,%d,%d|%d,%d,%d\n",
        int(m[0][0]), int(m[0][1]), int(m[0][2]),
        int(m[1][0]), int(m[1][1]), int(m[1][2]));
}
