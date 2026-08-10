//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=9d7a17de2e7a4cacf7a44eefe78eaa8bea0a9a617b555cc1e20d4e784f62d6bf
//META: purpose=A DeferStmt placed at function entry has the entire function body as its surrounding scope; the deferred inner statement runs after every other statement in the function but before the caller resumes.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: defer at the top of a function body. The enclosing
// scope is the function body itself, so the defer fires when the
// function returns. The caller must observe the deferred print
// before the post-call print.

//TEST:INTERPRET(filecheck=CHECK):

void worker()
{
    defer printf("worker-cleanup\n");
    printf("worker-step1\n");
    printf("worker-step2\n");
}

void main()
{
    worker();
    printf("main-after\n");
    // CHECK: worker-step1
    // CHECK-NEXT: worker-step2
    // CHECK-NEXT: worker-cleanup
    // CHECK-NEXT: main-after
}
