//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
