//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/base.md#declbase-modifiablesyntaxnode
//META: doc_section_digest=9683ae36f3d54e970d99e22024c0739ff2e17e32e3095366751562dd84c368d4
//META: purpose=A struct's fields group through the same DeclBase group node as a comma-separated local, and modifiers written once in front of a grouped member declaration apply to every name it declares.
//META: intent=boundary
//META: pipeline_stage=parser
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc gives `int a = 1, b = 2;` as the everyday grouping case and then
// states that "a struct's fields group the same way (`int x, y;`)". That
// sentence is the corner this test covers: the group node is a member of the
// AST base hierarchy rather than a statement-level construct, so it appears
// inside a struct body too, and it is a `ModifiableSyntaxNode`, so modifiers
// written once in front of the group reach every name.
//
// Both halves are observed as values rather than as spellings: the two fields
// are written independently and read back, which they could not be if the group
// produced one field; and the two `static const` members are read through the
// type, which requires the `static const` written once to have applied to `Q`
// as well as to `P`.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    static const int P = 3, Q = 4;
    int x, y;
}

void main()
{
    S s;
    s.x = 1;
    s.y = 2;
    // CHECK: 1 2 3 4
    printf("%d %d %d %d\n", s.x, s.y, S.P, S.Q);
}
