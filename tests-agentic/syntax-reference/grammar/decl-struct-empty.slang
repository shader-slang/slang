//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=1da21f4604b0cc983d0ee89029633af70ee59186fe9ef2a8dafdd1550c122368
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
