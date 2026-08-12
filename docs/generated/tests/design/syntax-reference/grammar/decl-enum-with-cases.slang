//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`EnumDecl` parses `'enum' IDENT '{' EnumCase (',' EnumCase)* ','? '}'` with an optional trailing comma.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the EnumDecl production: an enum with two cases, a trailing
// comma, and a case that has an explicit `= Expr` initializer.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red = 1, Green, Blue, }

void main()
{
    // The enum parses with three cases and a trailing comma. We observe
    // the parse succeeded by exercising case equality (which requires the
    // identifiers to have been bound).
    Color c = Color.Green;
    int r = (c == Color.Red) ? 1 : (c == Color.Blue) ? 3 : 2;
    // CHECK: c=2
    printf("c=%d\n", r);
}
