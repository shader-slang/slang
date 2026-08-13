//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=Negative - a CharLiteral with more than one character (`'AB'`) is rejected with the "illegal character literal" diagnostic.
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe: CharLiteral is documented as a "single-quoted
// character literal". Multi-byte content must produce a diagnostic
// rather than silently encoding the bytes; the caret span covers the
// whole `'AB'` literal.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = int('AB');
//CHECK:        ^^^^ illegal character literal
// CHECK: E10001
}
