//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
//META: purpose=Negative - an unescaped newline inside a double-quoted literal body is one of the two failures the string-literal helper reports itself, as `newline in literal` (E10005).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the string-literal body helper refuses to run past a raw newline.
// The helper steps over escaped quotes but has no way to find a closing quote
// once a physical line ends, so it must stop and diagnose rather than swallow
// the rest of the file. The CHECK pins the message text; the count of
// diagnostics is left open because recovery re-enters the literal on the
// following line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    printf("abc
");
// CHECK: E10005
}
