//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#pragma-warning-state-across-files
//META: doc_section_digest=f6dbf82c54f6492f0545c0abfc748655ac88c32761486d61a5b3c371a50f52cb
//META: purpose=`#pragma warning(push)` saves warning state and `#pragma warning(pop)` restores it, re-enabling a warning disabled inside the push/pop region.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the push/pop arms of the warning-state tracker: W30081
// is disabled inside a push/pop region, so the conversion there must
// not warn (the CHECK-NOT region runs up to the first match), while
// the identical conversion after the pop must warn again because pop
// restored the enabled state saved by push.

//TEST:SIMPLE(filecheck=CHECK):

int64_t getValue();

#pragma warning (push)
#pragma warning (disable : 30081)

void insidePushRegion()
{
    int i;
    // CHECK-NOT: warning[E30081]
    i = getValue();
}

#pragma warning (pop)

void afterPop()
{
    int i;
    // CHECK: warning[E30081]
    i = getValue();
}
