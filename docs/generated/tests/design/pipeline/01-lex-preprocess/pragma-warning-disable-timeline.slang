//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#pragma-warning-state-across-files
//META: doc_section_digest=f6dbf82c54f6492f0545c0abfc748655ac88c32761486d61a5b3c371a50f52cb
//META: purpose=`#pragma warning(disable)` state is tracked on a source-location timeline: the same construct warns before the directive and is silent after it.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that warning state is resolved by source position on the
// tracker's timeline: an int64_t-to-int implicit conversion before
// the disable directive emits W30081, while the identical construct
// after the directive stays silent. The CHECK matches the first
// warning; the trailing CHECK-NOT proves no second one follows.

//TEST:SIMPLE(filecheck=CHECK):

int64_t getValue();

void beforeDirective()
{
    int i;
    // CHECK: warning[E30081]
    i = getValue();
}

#pragma warning (disable : 30081)

void afterDirective()
{
    int i;
    // CHECK-NOT: warning[E30081]
    i = getValue();
}
