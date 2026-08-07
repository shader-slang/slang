//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=Boundary probe of floating-point value extraction: a literal whose magnitude underflows the type parses as 0.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the underflow arm of floating-point literal decoding:
// 1e-400 is below the smallest representable positive value
// (denormals included), so the extracted value must be exactly
// zero. The equality test prints 1 only for a true zero.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float f = 1e-400;
    // CHECK: zero=1
    printf("zero=%d\n", (f == 0.0) ? 1 : 0);
}
