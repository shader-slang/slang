//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
//META: purpose=Negative - a raw string whose delimiter starts with a bare `"` (as in `R""(...)"`) is rejected with the quote-cannot-be-delimiter diagnostic (E10010).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the raw-string delimiter grammar excludes the quote
// character: writing a second `"` immediately after the `R"` opener
// (so the would-be delimiter begins with `"`) must be diagnosed
// instead of being folded into the delimiter text. The CHECK pins
// code E10010 so the failure mode is the documented rejection; the
// rejected literal then runs to end of file, so the follow-on
// end-of-file-in-literal diagnostic is annotated too.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    let s = R""(x)";
//CHECK: E10010
//CHECK: end of file in literal
}
