//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=2ff8fbd6c14c1930e6df45b007c07bb9e931f77d21a4e28207d5df091a83100d
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
