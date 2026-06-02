//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=8d7ee5a379aeb4b393aeb766e8179650d332b2a7a6bcc65b16e7e5c13a614827
//META: purpose=A DeferStmt's inner statement runs even when the enclosing scope exits via early `return`; the IR pipeline materializes it as a scope-exit handler reached on every control-flow path.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says defer runs when the enclosing scope exits "normally or
// via return/break/throw". We observe by writing a defer that prints,
// then taking an early return: the defer must still fire before the
// caller's next statement.

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
