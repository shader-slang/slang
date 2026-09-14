//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`StructDecl` parses as `'struct' IDENT '{' '}'` with an empty member list.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the smallest legal `StructDecl` production: keyword `struct`,
// identifier, empty brace-delimited body. If the production failed to
// parse the empty body, declaration or usage at module scope would
// fail.

//TEST:INTERPRET(filecheck=CHECK):

struct Empty {}

void main()
{
    Empty e;
    // CHECK: ok
    printf("ok\n");
}
