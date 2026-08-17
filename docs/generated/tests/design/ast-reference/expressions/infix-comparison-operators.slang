//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=An InfixExpr can be a comparison operator; the result is a bool that reflects the comparison.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// InfixExpr is documented as the binary-operator AST kind. The
// comparison operators yield bool results, which we print as 0/1.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool a = 3 < 5;
    bool b = 5 < 3;
    bool c = 5 == 5;
    bool d = 5 != 5;
    // CHECK: a=1
    printf("a=%d\n", int(a));
    // CHECK: b=0
    printf("b=%d\n", int(b));
    // CHECK: c=1
    printf("c=%d\n", int(c));
    // CHECK: d=0
    printf("d=%d\n", int(d));
}
