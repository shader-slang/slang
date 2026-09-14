//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=9d7a17de2e7a4cacf7a44eefe78eaa8bea0a9a617b555cc1e20d4e784f62d6bf
//META: purpose=A DeferStmt's inner statement still runs when the enclosing scope exits via an early `return`, not only when the scope falls through its end.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a deferred statement still runs when the function exits via
// an early `return`, not only on fall-through. The CHECK asserts the
// deferred print lands between the function body's print and the
// caller's next print.

//TEST:INTERPRET(filecheck=CHECK):

void inner()
{
    defer printf("inner-defer\n");
    printf("inner-body\n");
    return;
}

void main()
{
    inner();
    printf("after-call\n");
    // CHECK: inner-body
    // CHECK-NEXT: inner-defer
    // CHECK-NEXT: after-call
}
