//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=9d7a17de2e7a4cacf7a44eefe78eaa8bea0a9a617b555cc1e20d4e784f62d6bf
//META: purpose=A DeferStmt's inner statement runs when the enclosing scope exits rather than at the position the `defer` is written.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a deferred statement does not run where it is written but at
// the exit of the block containing it. The CHECK asserts the ordering
// body -> deferred -> after-block, which is what distinguishes
// scope-exit execution from in-place execution.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    {
        defer printf("deferred\n");
        printf("body\n");
    }
    printf("after-block\n");
    // CHECK: body
    // CHECK-NEXT: deferred
    // CHECK-NEXT: after-block
}
