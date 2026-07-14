//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
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
