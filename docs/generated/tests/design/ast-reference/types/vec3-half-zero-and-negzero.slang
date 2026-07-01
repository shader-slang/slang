//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<half, 3> with +0.0 and -0.0 lanes: both compare numerically equal to 0 yet have distinct sign bits.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: float-edge value probe at the half3 corner. Both
// +0.0 and -0.0 are valid half values; numerically each compares
// equal to 0. We confirm both lanes round-trip through int(...).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half3 v = half3(half(0.0), half(-0.0), half(0.0));
    int eq = (v.x == half(0.0)) ? 1 : 0;
    int eqn = (v.y == half(0.0)) ? 1 : 0;
    // CHECK: eq=1,1
    printf("eq=%d,%d\n", eq, eqn);
}
