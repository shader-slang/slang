//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#ifstmt
//META: doc_section_digest=fa628db8ade3560b33a46074f32ce4b163adb95e07f7892450f1b1f9c7ee55dc
//META: purpose=An IfStmt evaluates its predicate Expr exactly once per execution of the statement; a side-effecting predicate's effect is observed exactly once, regardless of which branch is selected.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the IfStmt holds a single `predicate: Expr*` slot. If
// the checker mis-lowered the predicate into both the test and one
// of the branches, a side-effecting predicate would be observed
// twice. We use an inout counter argument as the side-effecting
// predicate (slangi cannot host non-const static module globals).

//TEST:INTERPRET(filecheck=CHECK):

bool predicate(inout int counter)
{
    counter = counter + 1;
    return true;
}

void main()
{
    int calls = 0;
    if (predicate(calls)) { calls = calls + 10; }
    // calls should be 1 (predicate) + 10 (branch) = 11
    // CHECK: calls=11
    printf("calls=%d\n", calls);
}
