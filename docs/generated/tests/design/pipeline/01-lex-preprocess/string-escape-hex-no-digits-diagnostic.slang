//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
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
