//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An AssignExpr `a = b` updates the l-value `a` to the value of `b`; reading `a` afterward yields the new value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists AssignExpr with `left: Expr*`, `right: Expr*`. The
// effect is observable by reading the assigned variable after the
// assignment statement runs.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 0;
    x = 42;
    // CHECK: x=42
    printf("x=%d\n", x);
}
