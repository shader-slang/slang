//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<uint64_t, 3>: three unsigned 64-bit lanes survive add into a positive scalar.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<uint64_t, 3> covers 64-bit unsigned at N=3.
// Lane addition uses scalar adds because the interpreter lacks a
// dedicated vectorised uint64 add handler.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<uint64_t, 3> v = vector<uint64_t, 3>(uint64_t(1), uint64_t(2), uint64_t(3));
    uint64_t s = v.x + v.y + v.z;
    // CHECK: s=6
    printf("s=%d\n", int(s));
}
