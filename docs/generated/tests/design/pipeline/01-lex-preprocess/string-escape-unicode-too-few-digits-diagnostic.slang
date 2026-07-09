//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=Negative probe: a `\u` escape with fewer than 4 hex digits is diagnosed as an invalid Unicode string escape (E10008).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fixed-width rule of the `\u` escape form: the escape
// below provides only two hex digits before a non-hex character, so
// the ill-formed-Unicode-escape diagnostic fires, pinned to code
// E10008 with the digit-count requirement in its message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    String s = "ab\u12xy";
    //CHECK: \u must be followed by 4 hex digits
    // CHECK: E10008
}
