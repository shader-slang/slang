//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:31+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=a9e6231e60a10097307c960ed42577646899eb49854183d8f45eeb036c42700a
//META: purpose=An unsuffixed decimal literal past INT64_MAX is diagnosed (IntegerLiteralTooLarge) rather than truncated: INT64_MAX+1 is diagnosed unless negated, INT64_MAX+2 and above always.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the seam at INT64_MAX + 1 = 9223372036854775808. The un-negated
// literal is diagnosed (the checker defers it, since unary minus rescues it),
// the negated spelling is the legal int64 minimum and is not, and
// INT64_MAX + 2 is always diagnosed. Only the two un-negated literals carry
// the warning code E40004; the negated one has no CHECK on its line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

void main()
{
    uint64_t plusOne = 9223372036854775808;
//CHECK:               ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned
    int64_t minimum = -9223372036854775808;
    uint64_t plusTwo = 9223372036854775809;
//CHECK:               ^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in a signed integer type, interpreting as unsigned
}
