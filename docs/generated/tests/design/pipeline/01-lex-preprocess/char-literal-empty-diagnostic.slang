//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=Negative probe: an empty character literal is diagnosed at decode time as an illegal character literal (E10001).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "body too short to hold a character" arm of the
// one-character rule that the char value-extraction helper owns. The
// scanner only finds the closing quote; the rejection happens when
// the value is decoded, so the diagnostic is pinned to E10001 with a
// caret spanning the two quotes the scanner recorded.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    uint c = '';
//CHECK:     ^^ illegal character literal
// CHECK: E10001
}
