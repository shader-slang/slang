//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=In a string literal a `\x` escape decodes to a single byte spliced into the string's bytes.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies string-literal escape decoding through the emitted C++
// text: "\x48i" decodes byte 0x48 ('H') followed by the literal
// character 'i' (not a hex digit, so it terminates the escape), and
// the decoded string "Hi" appears in the emitted source. slangi
// printf does not support %s, hence the emission observation.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", "\x48i");
    //CHECK: "Hi"
}
