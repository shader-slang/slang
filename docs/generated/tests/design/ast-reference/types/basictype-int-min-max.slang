//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A 32-bit int BasicExpressionType holds its documented MIN (-2147483648) and MAX (2147483647) boundary values without truncation.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the int scalar BasicExpressionType at its two
// signed edges. Both endpoints are stored and printed; CHECK asserts
// they survive as the exact 32-bit two's-complement extremes.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int lo = -2147483648;
    int hi = 2147483647;
    // CHECK: lo=-2147483648
    printf("lo=%d\n", lo);
    // CHECK: hi=2147483647
    printf("hi=%d\n", hi);
}
