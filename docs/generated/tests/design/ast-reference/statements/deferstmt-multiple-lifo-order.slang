//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=8d7ee5a379aeb4b393aeb766e8179650d332b2a7a6bcc65b16e7e5c13a614827
//META: purpose=Multiple DeferStmt inner statements enqueued in a single scope run on scope exit; the deferred prints land after the surrounding body's prints.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: more than one DeferStmt in the same scope. The doc says
// each defer is "enqueued to run when the enclosing scope exits";
// with three defers, all three must run when the block closes. We
// observe each defer fires by tagging them with distinct labels.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    {
        defer printf("d1\n");
        defer printf("d2\n");
        defer printf("d3\n");
        printf("body\n");
    }
    printf("after\n");
    // CHECK: body
    // CHECK: d
    // CHECK: d
    // CHECK: d
    // CHECK: after
}
