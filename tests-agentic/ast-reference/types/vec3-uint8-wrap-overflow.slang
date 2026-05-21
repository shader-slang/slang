//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<uint8_t, 3>: scalar lane addition of uint8_t MAX (255) and 1 wraps to 0 per documented unsigned-wrap semantics.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: boundary at the uint8_t lane of a vector. The
// per-lane unsigned-wrap behavior documented for BasicExpression
// integer types holds inside a vector lane.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<uint8_t, 3> v = vector<uint8_t, 3>(uint8_t(255), uint8_t(255), uint8_t(255));
    uint8_t w = uint8_t(v.x + uint8_t(1));
    // CHECK: w=0
    printf("w=%d\n", int(w));
}
