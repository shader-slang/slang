//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<int16_t, 3>: three int16_t lanes survive arithmetic across the documented signed 16-bit range.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<int16_t, 3> covers signed-16 at N=3. We
// observe both negative and positive lane values, summing to zero
// to confirm signed semantics survive the (int16_t, 3) operand
// pair.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<int16_t, 3> v = vector<int16_t, 3>(int16_t(-1000), int16_t(0), int16_t(1000));
    int16_t s = int16_t(v.x + v.y + v.z);
    // CHECK: s=0
    printf("s=%d\n", int(s));
}
