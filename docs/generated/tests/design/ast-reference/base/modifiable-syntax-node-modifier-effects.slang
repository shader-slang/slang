//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#modifiablesyntaxnode-syntaxnode
//META: doc_section_digest=5400392de5f3914b937152b3a687cf2f86b73c6fdd2711ed9c393a4bd6334fdb
//META: purpose=The modifier list a ModifiableSyntaxNode owns is consulted during checking, so a mutating member function is permitted to write through this while a plain one is not.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a modifier stored on a declaration changes how that
// declaration is checked, rather than merely being parsed and kept.
// `setTo` is marked `[mutating]` and assigns to a field of `this`;
// CHECK asserts the write is visible to the caller afterwards, which
// requires the checker to have found the modifier on the method decl.

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
