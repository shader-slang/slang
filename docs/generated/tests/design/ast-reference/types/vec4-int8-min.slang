//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<int8_t, 4>: four int8_t lanes carry the documented signed [-128, 127] range; both edges survive per-lane.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<int8_t, 4> covers the narrow signed corner
// at N=4. The documented MIN (-128) and MAX (127) live in
// adjacent lanes; readback via int(...) widening preserves
// signedness.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<int8_t, 4> v = vector<int8_t, 4>(
        int8_t(-128), int8_t(-1), int8_t(0), int8_t(127));
    // CHECK: v=-128,-1,0,127
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
