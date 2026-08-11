//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-packbranchtypeexpr
//META: doc_section_digest=1d025242d1797d4e136aa1be3b95725504ad5b96bc6526725ad99442cfbb5be0
//META: purpose=A written `This` is an ordinary name resolved by lookup, not a dedicated type-expression node; a requirement returning `This` binds to each conformer's own type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `This` behaves like any other name in type position: it is
// looked up in the enclosing scope and resolves to the surrounding type
// rather than being a special keyword form. An interface requirement returns
// `This`; the implementation returns its own struct type and the result is
// used as that concrete type, so the CHECK only matches if the lookup bound
// `This` to the conformer.

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
