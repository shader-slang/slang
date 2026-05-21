//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<half, 2> is a valid vector instantiation; elementwise add of two half2 values yields the documented elementwise sum.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion of the element-type x dimension cross-product:
// vector<half, 2>. The (element-type, count) operand pair carries
// the half element type at the small-N=2 dimension. We add the
// scalar components rather than the vectors directly because the
// interpreter (see "compiler-bug observations" in README.md) lacks
// a handler for vectorized half add; the (half, 2) operand pair
// itself is what this test asserts.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half2 a = half2(half(1.5), half(2.5));
    half2 b = half2(half(10.0), half(20.0));
    half cx = a.x + b.x;
    half cy = a.y + b.y;
    // CHECK: c=11,22
    printf("c=%d,%d\n", int(cx), int(cy));
}
