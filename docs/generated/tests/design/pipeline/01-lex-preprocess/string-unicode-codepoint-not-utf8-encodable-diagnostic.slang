//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=60402c8d086f96269e6fe5211e0043b74eb989c5d25aea5312e6a4ba5d5f8bf4
//META: purpose=Boundary probe: a Unicode escape naming a code point that cannot be encoded as UTF-8 is rejected at decode time (E10013).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the decode-time range check on the UTF-8 encoding path
// for Unicode escapes in string literals: U+200000 is one past the
// largest sequence UTF-8 can represent in the encoder's range, so
// encoding fails and the diagnostic is pinned to code E10013. Kept
// as the only error in the file so the decode-time report is not
// suppressed by scan-time errors.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    String s = "\U00200000";
    //CHECK: Unicode code point 2097152 (0x200000) cannot be encoded as UTF-8
    // CHECK: E10013
}
