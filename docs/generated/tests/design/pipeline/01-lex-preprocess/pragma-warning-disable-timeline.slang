//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#pragma-warning-state-across-files
//META: doc_section_digest=8d50eb65e40894cf0318aa4dcada1f78bcbcabc46b7e3a3d94a0319e9947491d
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
