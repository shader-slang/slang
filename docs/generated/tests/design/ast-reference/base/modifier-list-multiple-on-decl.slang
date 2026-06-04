//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/ast-reference/base.md#modifier-syntaxnode
//META: doc_section_digest=4022b505250fb748fc38ae952be0700e371b5a3704ae0d911aa7fe1b17f68532
//META: purpose=Modifiers form a linked list attached to a ModifiableSyntaxNode; multiple modifiers stack onto a single declaration and all take effect.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that several modifiers attach to a single declaration —
// the linked-list of modifiers on a ModifiableSyntaxNode. Here a
// struct method carries both `[mutating]` and a backslash-line-
// broken second attribute. If the modifier list could only hold one
// modifier, attaching both would silently drop one or diagnose; the
// program compiles and runs only if the list carries both.

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
