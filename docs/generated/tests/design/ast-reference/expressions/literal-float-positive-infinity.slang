//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=1e8f144399f6073f93949619c72c2230cfb4349def0ab49ac5088e68f85aa4ab
//META: purpose=Verifies that a FloatingPointLiteralExpr arithmetic chain that overflows to +inf compares greater than the largest finite literal.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// +inf boundary. Multiplying the largest representable finite float
// by 10 overflows to +inf; the resulting value must order strictly
// above the finite literal.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float big = 3.4e38f;
    float infv = big * 10.0f;
    // CHECK: gt-max=1
    printf("gt-max=%d\n", int(infv > big));
}
