//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
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
