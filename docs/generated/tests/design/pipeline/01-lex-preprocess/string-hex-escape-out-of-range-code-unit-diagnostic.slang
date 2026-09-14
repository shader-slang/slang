//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
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
