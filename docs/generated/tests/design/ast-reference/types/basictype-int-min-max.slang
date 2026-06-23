//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=BasicExpressionType for int holds the full 32-bit signed range; MIN (-2147483648) and MAX (2147483647) round-trip without distortion.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the int BasicExpressionType: the documented MIN
// (-2^31) and MAX (2^31 - 1) both survive evaluation in the printf
// path. These are the canonical bookends of the 32-bit signed type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int lo = int(-2147483648);
    int hi = int(2147483647);
    // CHECK: lo=-2147483648
    printf("lo=%d\n", lo);
    // CHECK: hi=2147483647
    printf("hi=%d\n", hi);
}
