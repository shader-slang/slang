//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A struct with no user __init still has a synthesized member-init constructor; positional-initializer syntax fills fields in declaration order.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of synthesized ConstructorDecl
// flavors. A struct without an explicit __init still accepts a
// brace-list initializer that fills the fields in declaration order;
// the values come back out unchanged.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair
{
    int a;
    int b;
}

void main()
{
    Pair p = {3, 4};
    //CHECK: 7
    printf("%d\n", p.a + p.b);
}
