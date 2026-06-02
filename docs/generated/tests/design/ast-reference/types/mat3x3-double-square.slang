//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=MatrixExpressionType<double, 3, 3>: square shape with double element type; diagonal cells are independently readable.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: square 3x3 shape with double element type. Probes
// the (double, 3, 3) operand triple.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    matrix<double, 3, 3> m = matrix<double, 3, 3>(
        1.0lf, 0.0lf, 0.0lf,
        0.0lf, 2.0lf, 0.0lf,
        0.0lf, 0.0lf, 3.0lf);
    // CHECK: d=1,2,3
    printf("d=%d,%d,%d\n", int(m[0][0]), int(m[1][1]), int(m[2][2]));
}
