//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:39:00Z
//META: source_commit=199a52d71d360a84e8a13837b913746113a9b510
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=28a5007e7990e447eb6cc6141908413f26bb0721bb1b7dcb8e454fd24d38dbb8
//META: purpose=Negative - an empty character literal (`''`) is rejected eagerly with the "illegal character literal" diagnostic (E10001).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lexer's eager one-character rule for char literals from
// the empty side: a ''-quoted body with zero characters must fire the
// illegal-character-literal diagnostic at lex time. The CHECK pins the
// diagnostic code E10001 so a silent acceptance or a different error
// both fail the test. (The multi-character side of the same rule is
// probed by negative-multi-char-literal.slang.)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = int('');
//CHECK:        ^^ illegal character literal
// CHECK: E10001
}
