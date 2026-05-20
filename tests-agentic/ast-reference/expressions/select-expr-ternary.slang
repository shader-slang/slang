//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=A SelectExpr `c ? a : b` yields a when c is true and b when c is false.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists SelectExpr as the ternary `c ? a : b`. Both branches
// must be observable independently, picked by the condition.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = (1 < 2) ? 100 : 200;
    int b = (1 > 2) ? 100 : 200;
    // CHECK: a=100
    printf("a=%d\n", a);
    // CHECK: b=200
    printf("b=%d\n", b);
}
