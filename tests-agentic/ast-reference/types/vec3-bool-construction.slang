//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<bool, 3>: three bool lanes are independently addressable.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: bool3 covers (bool, 3). Each lane independently
// holds true/false.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool3 b = bool3(true, false, true);
    // CHECK: b=1,0,1
    printf("b=%d,%d,%d\n", b.x ? 1 : 0, b.y ? 1 : 0, b.z ? 1 : 0);
}
