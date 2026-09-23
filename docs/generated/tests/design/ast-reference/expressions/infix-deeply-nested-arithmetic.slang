//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Stresses InfixExpr parsing with a five-deep paren-nested arithmetic chain: ((((1+2)*3)-4)/5) must evaluate to 1.
//META: intent=stress
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Deep nesting stress for InfixExpr / ParenExpr interaction. Five
// levels of parentheses force the parser through repeated descent;
// the result must still equal the documented arithmetic value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int r = ((((1 + 2) * 3) - 4) / 5);
    // CHECK: r=1
    printf("r=%d\n", r);
}
