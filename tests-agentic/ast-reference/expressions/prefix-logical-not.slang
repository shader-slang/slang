//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
