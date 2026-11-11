//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=6c5b24137d8a00d45340c81231f003fe2cc4825dcc7a3203d181feeb7da8b977
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
