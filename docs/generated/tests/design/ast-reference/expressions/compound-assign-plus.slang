//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A compound-assignment InfixExpr `a += b` updates `a` to `a + b`; the resulting value is the sum.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The InfixExpr Nodes row covers binary operators including the
// compound-assignment forms. `+=` updates the l-value in place; the
// read after the statement returns the updated value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 10;
    x += 5;
    // CHECK: x=15
    printf("x=%d\n", x);
}
