//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`EnumDecl` accepts an optional `'class'` keyword after `'enum'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the optional `'class'` token in the EnumDecl production. The
// enumerators are still reached through the type name and still carry their
// implicit ordinals, so converting `Color.Green` and printing 1 proves the
// `enum class` spelling produced an ordinary enum declaration rather than
// dispatching to the struct/class parser.

//TEST:INTERPRET(filecheck=CHECK):

enum class Color
{
    Red,
    Green
}

void main()
{
    // CHECK: r=1
    printf("r=%d\n", int(Color.Green));
}
