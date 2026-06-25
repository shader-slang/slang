//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr
//META: doc_section_digest=4254abda0523877f7fb67acc9da1ad7de3884b6d869e71f92f12575587230f39
//META: purpose=A ThisTypeExpr `This` refers to the surrounding type; in an interface, a method returning `This` resolves to each conformer's own type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists ThisTypeExpr as the `This` keyword referring to the
// surrounding type. Inside an interface, a method returning `This`
// resolves to each implementing type's own type; the value chain
// preserves the implementer's concrete type.

//TEST:INTERPRET(filecheck=CHECK):

interface ICloneable
{
    This clone();
    int tag();
}

struct A : ICloneable
{
    int v;
    This clone() { A r = { v + 1 }; return r; }
    int tag() { return v; }
}

void main()
{
    A a = {10};
    A b = a.clone();
    // CHECK: t=11
    printf("t=%d\n", b.tag());
}
