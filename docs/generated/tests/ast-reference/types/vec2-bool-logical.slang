//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<bool, 2>: two bool lanes are independently addressable; per-component values are observable.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<bool, 2> at the small-N=2 dim. Bool is one of
// the documented scalar element types and pairs with N=2.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool2 b = bool2(true, false);
    // CHECK: b=1,0
    printf("b=%d,%d\n", b.x ? 1 : 0, b.y ? 1 : 0);
}
