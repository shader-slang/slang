//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=b72726890459268d21bfd18ef8977151c7699c0ba49e17682e79c4cb2cfa2ff4
//META: purpose=An EnumDecl declares a tagged-union enum and each EnumCaseDecl is one named case; case selection picks the appropriate branch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented roles of EnumDecl and EnumCaseDecl. The
// enum's cases compare distinctly and a function dispatched by case
// produces the value associated with each branch.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red = 1, Green = 2, Blue = 3 }

int describe(Color c)
{
    if (c == Color::Red)   return 10;
    if (c == Color::Green) return 20;
    return 30;
}

void main()
{
    //CHECK: 10 20 30
    printf("%d %d %d\n",
        describe(Color::Red),
        describe(Color::Green),
        describe(Color::Blue));
}
