//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=b092f350bdbde298b8860c3da710d6672d2e5f235d75620d903bddf8a6158c3b
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
