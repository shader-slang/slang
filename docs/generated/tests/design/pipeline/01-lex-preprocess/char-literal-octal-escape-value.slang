//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=An octal `\NNN` escape in a character literal decodes to the code point it names; `'\101'` yields 65.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies octal-escape decoding: `\101` is octal for 65 ('A').
// If the decoder read the digits as decimal (101) or mis-handled
// the octal sub-loop, the printed value would differ from 65.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: octal=65
    printf("octal=%d\n", int('\101'));
}
