//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=c018493cff4ea0d125fd96032328c7a624df200e9019bef0883c86de143f6642
//META: purpose=An EnumDecl is an aggregate type declaration, so an enum can be given an interface conformance and dispatched through a constrained generic.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an enum participates in the same conformance machinery as a
// struct. An extension adds an interface conformance to the enum, and an
// interface-constrained generic is called with an enum value; the CHECK
// asserts the value the conformance's method returns for that case.

//TEST:INTERPRET(filecheck=CHECK):

interface IRanked { int rank(); }

enum Color { Red, Green, Blue }

extension Color : IRanked
{
    int rank()
    {
        if (this == Color::Red)   return 1;
        if (this == Color::Green) return 2;
        return 3;
    }
}

int dispatch<T : IRanked>(T x) { return x.rank(); }

void main()
{
    //CHECK: 2
    printf("%d\n", dispatch(Color::Green));
}
