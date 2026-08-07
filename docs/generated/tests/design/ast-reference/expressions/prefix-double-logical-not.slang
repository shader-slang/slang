//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that two stacked PrefixExpr `!` operators are an identity on a boolean: `!!true == true` and `!!false == false`.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on stacked PrefixExpr operators. Each `!` is its own
// PrefixExpr node; the parser must handle the leading run as
// nested unary, and the semantic should be identity on bool.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool t = true;
    bool f = false;
    // CHECK: tt=1
    printf("tt=%d\n", int(!!t));
    // CHECK: ff=0
    printf("ff=%d\n", int(!!f));
}
