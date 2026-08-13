//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=9574aa361d7b18b1da022473e0e53acd215b9af06dae0cb39490ca13610d03bf
//META: purpose=Member lookup on a DeclRefType drives the facet walk, so a field declared on a base struct is reachable through a derived value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that member lookup does not stop at the self facet. `b` is
// declared only on Base, so reaching it through a Derived receiver
// requires the walk to visit the base facet and prepend the super-type
// navigation step. CHECK asserts 5 + 7, so both the inherited and the
// direct field must have resolved.

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
    x.b = 5;
    x.d = 7;
    // CHECK: r=12
    printf("r=%d\n", x.b + x.d);
}
