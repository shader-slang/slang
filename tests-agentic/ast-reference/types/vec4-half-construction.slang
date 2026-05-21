//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<half, 4> at the largest standard dim N=4 carries four addressable half components.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: half4 at the large end of the standard vector
// dimension range. Confirms (half, 4) is a valid VectorExpressionType
// operand pair.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half4 v = half4(half(10.0), half(20.0), half(30.0), half(40.0));
    // CHECK: v=10,20,30,40
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
