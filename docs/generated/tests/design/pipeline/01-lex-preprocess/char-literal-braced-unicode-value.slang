//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=Boundary probe of the braced `\u{...}` escape: a code point above the BMP (`'\u{1F600}'`) decodes to 128512 as a 32-bit value.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the braced Unicode escape form with a supplementary-plane
// code point: U+1F600 = 128512 does not fit in 16 bits, so the
// printed value is only correct if the braced form really carries
// up to 32 bits through the char value-extraction helper.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: braced=128512
    printf("braced=%d\n", int('\u{1F600}'));
}
