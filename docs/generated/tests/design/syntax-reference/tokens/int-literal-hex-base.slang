//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Boundary - a hexadecimal integer literal (`0xFF`) is a valid IntegerLiteral and decodes to its base-16 value.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: integer literals carry a base prefix (`0x`) that
// the lexer accepts as part of the IntegerLiteral token. `0xFF`
// should decode to decimal 255.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 0xFF;
    // CHECK: v=255
    printf("v=%d\n", v);
}
