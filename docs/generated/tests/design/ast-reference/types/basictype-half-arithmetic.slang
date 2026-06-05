//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=BasicExpressionType for half (16-bit float) carries scalar arithmetic; small fractional values are representable and survive evaluation.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the half BasicExpressionType: confirms that the
// 16-bit float scalar built-in is constructible and addable. We
// truncate to int for printf observation rather than rely on %f
// formatting of half.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half a = half(1.5);
    half b = half(2.5);
    half c = a + b;
    // CHECK: c=4
    printf("c=%d\n", int(c));
}
