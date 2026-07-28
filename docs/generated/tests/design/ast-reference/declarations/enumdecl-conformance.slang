//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=c018493cff4ea0d125fd96032328c7a624df200e9019bef0883c86de143f6642
//META: purpose=An EnumDecl is treated as an AggTypeDecl so enum types can have conformances; an interface-constrained generic accepts an enum value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented note that "EnumDecl is treated as an
// AggTypeDecl so that enum types can have conformances and member
// functions like any other type." An enum extension adds an
// interface conformance; the interface-constrained generic call
// dispatches through the enum value.

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
