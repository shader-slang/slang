//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<uint16_t, 4>: four uint16_t lanes carry the documented unsigned 16-bit range.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<uint16_t, 4> covers unsigned-16 at N=4. The
// lane values 0, 32768, and 65535 (max) all round-trip through
// the per-lane storage.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<uint16_t, 4> v = vector<uint16_t, 4>(
        uint16_t(0), uint16_t(1), uint16_t(32768), uint16_t(65535));
    // CHECK: v=0,1,32768,65535
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
