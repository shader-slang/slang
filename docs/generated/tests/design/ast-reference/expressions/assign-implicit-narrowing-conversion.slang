//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that an AssignExpr accepts a right-hand expression of a different (convertible) type — an implicit float-to-int conversion is inserted and the value is truncated to 3.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on AssignExpr's type-mismatch handling: the checker
// inserts an implicit cast when the RHS is convertible to the LHS
// type. Assigning 3.7f to an int truncates to 3.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x;
    x = 3.7f;
    // CHECK: x=3
    printf("x=%d\n", x);
}
