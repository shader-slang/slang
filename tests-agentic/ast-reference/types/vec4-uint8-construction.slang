//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<uint8_t, 4> at the large dim N=4 carries four uint8_t lanes spanning the documented range.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: vector<uint8_t, 4> covers the narrow-unsigned x N=4
// corner of the element-type x dimension cross-product. The
// documented unsigned-8 range [0, 255] occupies four lanes.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    vector<uint8_t, 4> v = vector<uint8_t, 4>(uint8_t(0), uint8_t(64), uint8_t(192), uint8_t(255));
    // CHECK: v=0,64,192,255
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
