//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<double, 2> instantiation; double2 elementwise multiplication yields elementwise products.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: double element type at N=2. Elementwise multiplication
// is a documented arithmetic operation on VectorExpressionType.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    double2 a = double2(3.0lf, 4.0lf);
    double2 b = double2(5.0lf, 6.0lf);
    double2 c = a * b;
    // CHECK: c=15,24
    printf("c=%d,%d\n", int(c.x), int(c.y));
}
