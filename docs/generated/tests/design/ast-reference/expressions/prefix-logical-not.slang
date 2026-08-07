//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=A PrefixExpr `!x` is the logical-not unary prefix operator; it flips a bool value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// PrefixExpr covers `!x` (the example given in the doc's Nodes row).
// Negating a true and a false confirms both polarities.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool t = true;
    bool f = false;
    // CHECK: !t=0
    printf("!t=%d\n", int(!t));
    // CHECK: !f=1
    printf("!f=%d\n", int(!f));
}
