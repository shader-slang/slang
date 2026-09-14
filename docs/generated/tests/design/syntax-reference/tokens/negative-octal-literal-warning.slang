//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
//META: purpose=Negative - a leading-digit run after `0` is lexed base-8 only after the lexer reports the `'0' prefix indicates octal literal` warning (W10002).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the leading-zero branch diagnoses before it commits to base 8.
// `0777` is a legal octal literal, so nothing downstream complains; the only
// observable trace of the base-8 decision is the lexer warning, so the caret
// span over the whole literal plus the 10002 code is what proves the arm ran.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 0777;
//CHECK:    ^^^^ warning
// CHECK: 10002
    printf("%d\n", x);
}
