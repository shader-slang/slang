//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=Member lookup on a DeclRefType delegates to the inheritance walk, so a field declared on a base struct is reachable through a derived value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says member lookup on a DeclRefType delegates to
// `_lookupMembersInSuperTypeFacets`, which iterates each facet of
// the inheritance chain. Verify a field declared on `Base` is
// reachable via a `Derived` receiver.

//TEST:INTERPRET(filecheck=CHECK):

struct Base
{
    int b;
}

struct Derived : Base
{
    int d;
}

void main()
{
    Derived x;
    x.b = 5;   // reached through the SuperType facet to Base
    x.d = 7;
    // CHECK: r=12
    printf("r=%d\n", x.b + x.d);
}
