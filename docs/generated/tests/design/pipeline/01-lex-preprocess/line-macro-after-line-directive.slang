//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=377365e730e5a4dfaf556ef64127fd8adbee9dab99b45e23293376d8f95d3072
//META: purpose=Boundary probe of `__LINE__`: after a `#line 1000` directive, `__LINE__` reports a large value (boundary far from physical line 1).
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `#line 1000` directive shifts the logical line counter. The
// subsequent `__LINE__` constructed token must reflect the shifted
// value, so we observe the large-line boundary without having to
// pad the test file out to 1000 physical lines.

//TEST:INTERPRET(filecheck=CHECK):

#line 1000
void main()
{
    // CHECK: line=1003
    printf("line=%d\n", __LINE__);
}
