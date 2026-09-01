//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=Negative probe: a `\x` escape with no hex digits is diagnosed as an invalid string escape (E10007).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the "any number of digits" rule for
// `\x`: at least one hex digit is required, so an `\x` immediately
// followed by non-hex characters is an ill-formed escape, pinned to
// code E10007.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    String s = "ab\xzy";
    //CHECK: invalid string escape: '\x'
    // CHECK: E10007
}
