//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:39:00Z
//META: source_commit=199a52d71d360a84e8a13837b913746113a9b510
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=28a5007e7990e447eb6cc6141908413f26bb0721bb1b7dcb8e454fd24d38dbb8
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
