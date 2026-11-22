//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=6c5b24137d8a00d45340c81231f003fe2cc4825dcc7a3203d181feeb7da8b977
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
