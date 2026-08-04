//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#modifiablesyntaxnode-syntaxnode
//META: doc_section_digest=5400392de5f3914b937152b3a687cf2f86b73c6fdd2711ed9c393a4bd6334fdb
//META: purpose=A ModifiableSyntaxNode owns a Modifiers list; the modifiers attached to a Decl actually take effect during checking (a [mutating] member function is permitted to assign through `this`).
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
