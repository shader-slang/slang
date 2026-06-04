//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
