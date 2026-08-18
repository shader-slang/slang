//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A SelectExpr only evaluates the chosen arm; the unchosen arm's side effects do not happen.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SelectExpr `c ? a : b` evaluates only the chosen branch. We use
// arms with side effects (counter increments) and observe that only
// the chosen branch's counter advanced.

//TEST:INTERPRET(filecheck=CHECK):

int bumpAndReturn(inout int counter, int v) { counter++; return v; }

void main()
{
    int taken = 0;
    int notTaken = 0;
    int r = (1 < 2) ? bumpAndReturn(taken, 11) : bumpAndReturn(notTaken, 22);
    // CHECK: r=11
    printf("r=%d\n", r);
    // CHECK: taken=1
    printf("taken=%d\n", taken);
    // CHECK: notTaken=0
    printf("notTaken=%d\n", notTaken);
}
