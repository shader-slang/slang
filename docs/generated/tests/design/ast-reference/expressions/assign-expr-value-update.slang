//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=An AssignExpr `a = b` updates the l-value `a` to the value of `b`; reading `a` afterward yields the new value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that assignment stores into its left operand. The variable is
// initialised, assigned a new value, and then read; the CHECK asserts the
// second value, so a dropped or reversed assignment is visible.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 0;
    x = 42;
    // CHECK: x=42
    printf("x=%d\n", x);
}
