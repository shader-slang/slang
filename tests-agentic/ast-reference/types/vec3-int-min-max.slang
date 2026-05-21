//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<int, 3>: three lanes hold int MIN and MAX endpoints; both round-trip through the per-lane storage.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: int3 boundary probe. The documented 32-bit signed
// range bookends (-2147483648 and 2147483647) both survive
// inside a VectorExpressionType<int, 3> lane.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int3 v = int3(int(-2147483648), 0, int(2147483647));
    // CHECK: v=-2147483648,0,2147483647
    printf("v=%d,%d,%d\n", v.x, v.y, v.z);
}
