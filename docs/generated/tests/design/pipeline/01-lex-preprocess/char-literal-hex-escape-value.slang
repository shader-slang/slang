//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=A `\xNN` escape in a character literal decodes to the code point it names; `'\x41'` yields 65.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies hex-escape decoding on the char-literal value-extraction
// path: the raw token text `'\x41'` is kept verbatim by the scanner,
// and decoding it yields the code point 0x41. Printing the value as
// an integer must give 65; any other value means the escape decoder
// mis-parsed the hex digits.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: hex=65
    printf("hex=%d\n", int('\x41'));
}
