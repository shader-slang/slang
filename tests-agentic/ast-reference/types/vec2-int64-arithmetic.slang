//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<int64_t, 2>: two int64_t lanes carry a 64-bit-range signed value at N=2.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<int64_t, 2> covers 64-bit signed at N=2.
// Each lane is independently addressable and arithmetic with
// values beyond the 32-bit range is preserved.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<int64_t, 2> a = vector<int64_t, 2>(int64_t(1000), int64_t(-2000));
    vector<int64_t, 2> b = vector<int64_t, 2>(int64_t(500), int64_t(2000));
    int64_t sx = a.x + b.x;
    int64_t sy = a.y + b.y;
    // CHECK: s=1500,0
    printf("s=%d,%d\n", int(sx), int(sy));
}
