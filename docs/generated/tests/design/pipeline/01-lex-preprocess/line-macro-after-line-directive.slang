//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=d51763c65e99c32e7704e3dec4e69c5776526cb88e95bd9188137ae60ad362b9
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
