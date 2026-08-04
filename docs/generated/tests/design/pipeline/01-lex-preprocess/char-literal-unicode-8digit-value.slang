//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=A `\UNNNNNNNN` escape (exactly 8 hex digits) in a character literal decodes to the named code point; `'\U00000041'` yields 65.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the 8-hex-digit `\U` Unicode escape form on the char
// value-extraction path. `\U00000041` names U+0041; the decoded
// code point printed as an integer must be 65, proving all eight
// digits were consumed as one escape.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: u8=65
    printf("u8=%d\n", int('\U00000041'));
}
