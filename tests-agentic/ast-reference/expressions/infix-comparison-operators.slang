//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
