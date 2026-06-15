//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=8d7ee5a379aeb4b393aeb766e8179650d332b2a7a6bcc65b16e7e5c13a614827
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
