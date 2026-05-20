//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=b72726890459268d21bfd18ef8977151c7699c0ba49e17682e79c4cb2cfa2ff4
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
