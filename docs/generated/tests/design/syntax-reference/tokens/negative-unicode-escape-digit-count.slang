//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
//META: purpose=Negative - a `\u` escape with fewer than four hex digits is rejected at decode time with the invalid-unicode-escape diagnostic (E10008).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `\u` form's digit-count rule is enforced when a consumer
// asks for the literal's value, not while scanning for the closing quote.
// The literal below is well-formed as a token (the lexer finds its end), so the
// only way the diagnostic can appear is the deferred decode path; the CHECK
// pins the digit-count wording that distinguishes it from a generic bad escape.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = int('\u12');
//CHECK:        ^^^^^^ error
// CHECK: 10008
    printf("%d\n", x);
}
