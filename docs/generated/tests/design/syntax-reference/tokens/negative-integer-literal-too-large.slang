//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Negative - an integer literal too wide for any built-in integer type triggers the documented "integer literal is too large" diagnostic.
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe: a 20-hex-digit literal exceeds the widest
// integer type. The lexer accepts the token shape but the checker
// reports the documented diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 0xFFFFFFFFFFFFFFFFFFFF;
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type
// CHECK: E10012
}
