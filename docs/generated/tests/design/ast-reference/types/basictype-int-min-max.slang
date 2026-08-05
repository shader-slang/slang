//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A 32-bit int BasicExpressionType holds its MIN (-2147483648) and MAX (2147483647) boundary values without truncation.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the signed 32-bit arithmetic leaf at both edges.
// Both endpoints are stored in an `int` and printed; CHECK asserts they
// survive as the exact two's-complement extremes, which would not hold
// if the literal or the variable were narrowed.

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
