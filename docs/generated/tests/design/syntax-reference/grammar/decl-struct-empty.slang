//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=49a7ad484e2dfba8ca655298b70ff54f8a19782584f47ca1a503b190c156a5cf
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
