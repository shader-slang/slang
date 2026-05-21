//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<double, 4> at the large dim N=4 carries four addressable double components.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: double4 covers the (double, 4) operand pair for
// VectorExpressionType. All four components are observable.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    double4 v = double4(100.0lf, 200.0lf, 300.0lf, 400.0lf);
    // CHECK: v=100,200,300,400
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
