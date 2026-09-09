//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#ifstmt
//META: doc_section_digest=a6c1740eeba3eb150d93ff559a355c4b73decc79a2b471fc7f1203bf5d838824
//META: purpose=A 5-arm if/else-if/else-if/else-if/else chain (nested IfStmts in each negativeStatement slot) dispatches to exactly the matching arm; later arms do not fire and the final else handles the unmatched fallback.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: an if/else-if chain is built recursively through the
// IfStmt::negativeStatement slot (each `else if` is itself an
// IfStmt inside the negative slot of the prior IfStmt). We exercise
// a 5-arm chain and observe that exactly one arm runs per input.

//TEST:INTERPRET(filecheck=CHECK):

int classify(int v)
{
    if      (v == 0) { return 100; }
    else if (v == 1) { return 101; }
    else if (v == 2) { return 102; }
    else if (v == 3) { return 103; }
    else if (v == 4) { return 104; }
    else             { return 999; }
}

void main()
{
    // CHECK: r0=100
    printf("r0=%d\n", classify(0));
    // CHECK: r2=102
    printf("r2=%d\n", classify(2));
    // CHECK: r4=104
    printf("r4=%d\n", classify(4));
    // CHECK: r9=999
    printf("r9=%d\n", classify(9));
}
