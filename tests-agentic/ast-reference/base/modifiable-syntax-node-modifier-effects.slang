//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/ast-reference/base.md#modifiablesyntaxnode-syntaxnode
//META: purpose=A ModifiableSyntaxNode owns a Modifiers list; the modifiers attached to a Decl actually take effect during checking (a [mutating] member function is permitted to assign through `this`).
//META: doc_section_digest=c83724cc2e67a0c1757c7e0f6348a823e9a1e94cacf2fe8aab557ba773326bc0
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the modifier list attached to a method declaration
// is honored by the checker: a `[mutating]` method may assign to a
// field of `this`. If the modifier were not stored on the
// ModifiableSyntaxNode (DeclBase) and consulted during checking, the
// assignment would either be rejected or have no effect.

//TEST:INTERPRET(filecheck=CHECK):

struct Box
{
    int v;

    [mutating]
    void setTo(int newV)
    {
        v = newV;
    }
};

void main()
{
    Box b;
    b.v = 0;
    b.setTo(42);
    //CHECK: v=42
    printf("v=%d\n", b.v);
}
