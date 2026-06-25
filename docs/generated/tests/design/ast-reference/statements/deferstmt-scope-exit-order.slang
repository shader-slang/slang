//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=8d7ee5a379aeb4b393aeb766e8179650d332b2a7a6bcc65b16e7e5c13a614827
//META: purpose=A DeferStmt enqueues its inner statement to run when the enclosing scope exits; the deferred side effect is observed after the surrounding scope's other statements complete.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says DeferStmt "is not executed at the position it appears,
// but instead enqueued to run when the enclosing scope exits". We
// place a `defer` early in a block and a non-deferred print after it:
// the deferred print must land after the block-closing point of the
// inner scope.

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
