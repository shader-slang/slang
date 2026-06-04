//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=A PostfixExpr `a++` is the unary postfix increment operator; its value is the pre-increment value of the operand.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// PostfixExpr's example in the doc is `a++`. After `int v = i++` we
// expect `v` to be the pre-increment value and `i` to have been
// incremented by 1.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 10;
    int v = i++;
    // CHECK: v=10
    printf("v=%d\n", v);
    // CHECK: i=11
    printf("i=%d\n", i);
}
