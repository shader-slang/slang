//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=0e1e5b6a05be01d6e2a0fbe1d41469c918569522adb5871ac6efa8d2a0811ba4
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
