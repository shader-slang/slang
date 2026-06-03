//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=Verifies that `x % 1` evaluates to 0 for any integer x — the lower-edge boundary of the InfixExpr modulo operator.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `% 1` is a documented identity: every integer is 0 mod 1. The
// InfixExpr `%` must compute that even at the divisor boundary 1.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int r1 = 7 % 1;
    int r2 = (-7) % 1;
    // CHECK: r1=0
    printf("r1=%d\n", r1);
    // CHECK: r2=0
    printf("r2=%d\n", r2);
}
