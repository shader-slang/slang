//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=b092f350bdbde298b8860c3da710d6672d2e5f235d75620d903bddf8a6158c3b
//META: purpose=`EnumDecl`'s name is optional, so an anonymous enum declares its cases directly into the enclosing scope.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `IDENT?` slot of EnumDecl. With no name there is no type to
// qualify the enumerators with, so the test names `AnonA` bare; resolving it
// proves an anonymous enum was declared and given a generated name rather
// than being rejected for a missing identifier. Printing 18 also shows the
// explicit tag expression survived on an unnamed enum.

//TEST:INTERPRET(filecheck=CHECK):

enum
{
    AnonA = 18,
    AnonB
}

void main()
{
    // CHECK: r=18
    printf("r=%d\n", int(AnonA));
}
