//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/ast-reference/base.md#stmt-modifiablesyntaxnode
//META: doc_section_digest=79ffb4284faeb40aaaece2b8fa53ff16f233b766fdb5bf8f30d2558160f8acd0
//META: purpose=Stmt derives from ModifiableSyntaxNode, so a statement can carry one or more modifiers (e.g. an unroll attribute on a for-statement).
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an attribute attaches to a statement. `[unroll]` on a
// for-loop is a Modifier hanging off the for-Stmt's modifier list. If
// Stmt could not carry modifiers, the attribute would be rejected or
// silently dropped; the program runs to completion only because the
// modifier-bearing statement still type-checks.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        sum = sum + i;
    }
    //CHECK: sum=6
    printf("sum=%d\n", sum);
}
