//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=26c797434ab51700cf84e595593cca11917ffceaff2d95c7a0504524e997316e
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
