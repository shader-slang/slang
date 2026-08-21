//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=Verifies that a NaN value (produced by `0.0/0.0`) does not equal itself, matching IEEE-754 semantics for the FloatingPointLiteralExpr family.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// NaN boundary. The LiteralExpr family carries FloatingPointLiteralValue
// for the dividend / divisor literals; the resulting NaN value must
// fail the self-equality test per IEEE-754.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float a = 0.0f;
    float b = 0.0f;
    float nanv = a / b;
    // CHECK: self-eq=0
    printf("self-eq=%d\n", int(nanv == nanv));
}
