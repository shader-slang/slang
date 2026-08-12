//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that the InfixExpr parser binds `*` tighter than `+`: `2 + 3 * 4` evaluates to 14, not 20.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Precedence corner: the InfixExpr tree for `a + b * c` must have
// `b * c` as the right operand of `+`. Compare with the paren-wrapped
// variant to verify both shapes evaluate as documented.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int r = 2 + 3 * 4;
    // CHECK: r=14
    printf("r=%d\n", r);
}
