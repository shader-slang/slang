//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An InfixExpr evaluates a binary arithmetic operator; each operator yields the expected numeric result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists InfixExpr as binary operator (`a + b`). The four
// integer operators below each parse as an InfixExpr and each
// evaluates to the documented arithmetic result.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int add = 7 + 5;
    int sub = 7 - 5;
    int mul = 7 * 5;
    int div = 35 / 5;
    // CHECK: add=12
    printf("add=%d\n", add);
    // CHECK: sub=2
    printf("sub=%d\n", sub);
    // CHECK: mul=35
    printf("mul=%d\n", mul);
    // CHECK: div=7
    printf("div=%d\n", div);
}
