//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=8cbc53779444646ce64a0d14a5a1f170a96bcb2eec03025e1840f339a2581e40
//META: purpose=An EmptyStmt gives a concrete statement for an empty slot; a loop whose body is a bare `;` is accepted and iterates via its side-effect expression alone.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a loop body consisting of only `;` is a valid statement
// slot: all iteration work happens in the for-header, the body is a
// bare semicolon, and the loop still runs to completion. The CHECK
// asserts the counter advanced through every iteration, which is only
// possible if the empty body parsed as a real statement and the loop
// executed normally.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 0;
    for (; n < 4; n = n + 1)
        ;
    // CHECK: n=4
    printf("n=%d\n", n);
}
