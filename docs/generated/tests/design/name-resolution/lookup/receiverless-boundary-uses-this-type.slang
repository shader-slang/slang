//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=43e51aa8f8900cb25c2253ee8c4a700f294af52f3a04c9121c8e46e771ee0e72
//META: purpose=Crossing a static method makes a receiver value unavailable; _lookUpInScopes records ThisType, and ConstructLookupResultExpr replays it to resolve an unqualified static member.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A static method is a receiverless declaration boundary. `_lookUpInScopes` therefore records
// `ThisType`, not `ThisValue`, when it reaches the enclosing struct while resolving `factor`.
// `ConstructLookupResultExpr` replays that step as the enclosing type and finds the static field
// without synthesizing a receiver value.

//TEST:INTERPRET(filecheck=CHECK):

struct U
{
    static const int factor = 4;
    static int scale(int x) { return x * factor; }
}

void main()
{
    // CHECK: r=20
    printf("r=%d\n", U::scale(5));
}
