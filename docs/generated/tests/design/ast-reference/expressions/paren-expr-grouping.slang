//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A ParenExpr `(e)` groups a sub-expression and overrides the default operator precedence at evaluation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that parentheses form their own expression node that regroups the
// operands rather than being dropped by the parser. `(2 + 3) * 4` is compared
// against the unparenthesized precedence result; the CHECK asserts 20, which
// is only reachable if the grouping survived.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int withParens = (2 + 3) * 4;
    int noParens   = 2 + 3 * 4;
    // CHECK: paren=20
    printf("paren=%d\n", withParens);
    // CHECK: noparen=14
    printf("noparen=%d\n", noParens);
}
