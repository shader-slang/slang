//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:36:14Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6d3d353f7acb30664cc90831179bf06d5f613a7e643d2f70a23548558a35ee35
//META: purpose=The `sample` and `centroid` modifiers share their own conflict group, separate from the other interpolation modifiers, so `sample centroid` on one field is rejected (E31202).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the sampling-location group: `sample` and `centroid` name competing
// sample locations, so the pair is rejected even though neither conflicts with
// an interpolation mode. CHECK pins E31202 and the message naming `centroid`
// against `sample`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct V
{
    sample centroid float4 c : COLOR;
//CHECK: E31202
//CHECK: modifier 'centroid' is redundant or conflicting with existing modifier 'sample'
}

void main()
{
}
