//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=Boundary probe: `\x` consumes any number of hex digits, and in a string literal the decoded code unit must fit one byte, else it is rejected at decode time (E10009).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two coupled rules: the `\x` escape greedily consumes all
// four hex digits below (it is not capped at two), and because a
// string-literal `\x` escape produces a single byte, the resulting
// value 0xFEDC is out of range for a code unit. The decode-time
// diagnostic is pinned to code E10009. This is kept as the only
// error in the file so the decode-time report is not suppressed by
// scan-time errors.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    String s = "\xFEDC";
    //CHECK: code unit 65244 (0xFEDC) is out of range
    // CHECK: E10009
}
