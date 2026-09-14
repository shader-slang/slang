//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`EnumDecl`'s case list is optional, so an enum may be declared with an empty `{ }` body.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the zero-case boundary of the EnumDecl body, whose case list is
// parenthesised as optional in the production. An enum with no enumerators is
// declared at module scope and the program still runs to completion; printing
// the sentinel proves the empty brace pair was accepted rather than treated as
// a missing case list.

//TEST:INTERPRET(filecheck=CHECK):

enum Empty
{
}

void main()
{
    // CHECK: r=14
    printf("r=14\n");
}
