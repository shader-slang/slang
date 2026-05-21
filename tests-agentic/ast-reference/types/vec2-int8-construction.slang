//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<int8_t, 2> instantiation; the documented int8_t MIN (-128) and MAX (127) survive in vector components.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<int8_t, 2> covers a narrow signed integer
// element type. Storing MIN/MAX at the lane edges and reading
// back via int(...) widening confirms the per-lane storage.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<int8_t, 2> v = vector<int8_t, 2>(int8_t(-128), int8_t(127));
    // CHECK: v=-128,127
    printf("v=%d,%d\n", int(v.x), int(v.y));
}
