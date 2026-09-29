//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=Verifies that the int8 base type round-trips the documented MIN (-128) and MAX (127) literal values through evaluation.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Small-int boundary: int8's MIN/MAX edges. The constructor form
// `int8_t(N)` invokes a TypeCastExpr that must preserve the value
// at the boundary of the type's representable range.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int8_t a = int8_t(-128);
    int8_t b = int8_t(127);
    // CHECK: min=-128
    printf("min=%d\n", int(a));
    // CHECK: max=127
    printf("max=%d\n", int(b));
}
