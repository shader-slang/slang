//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#modifier-syntaxnode
//META: doc_section_digest=ce8e569744478afff097f76c56095e0d6452e8ce6193cb8b0f043fa21c0270e5
//META: purpose=Modifiers form a linked list attached to a ModifiableSyntaxNode; multiple modifiers stack onto a single declaration and all take effect.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that several modifiers attach to a single declaration —
// the linked-list of modifiers on a ModifiableSyntaxNode. Here a
// struct method carries both `[mutating]` and `[ForceInline]`. If the
// modifier list could only hold one modifier, attaching both would
// silently drop one or diagnose; the program compiles and runs (the
// mutating write survives) only if the list carries both.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int n;

    [mutating]
    [ForceInline]
    void bump()
    {
        n = n + 1;
    }
};

void main()
{
    Counter c;
    c.n = 10;
    c.bump();
    c.bump();
    //CHECK: n=12
    printf("n=%d\n", c.n);
}
