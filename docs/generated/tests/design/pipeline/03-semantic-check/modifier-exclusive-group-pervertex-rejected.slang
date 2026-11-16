//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:36:14Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6d3d353f7acb30664cc90831179bf06d5f613a7e643d2f70a23548558a35ee35
//META: purpose=The `pervertex` modifier shares the interpolation conflict group with `nointerpolation`, `noperspective` and `linear`, so `pervertex linear` on one field is rejected (E31202).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the refreshed group membership: `pervertex` is a member of the
// first interpolation group, so it conflicts with `linear` just as
// `nointerpolation` does. CHECK pins E31202 and the message naming `linear`
// against `pervertex`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct V
{
    pervertex linear float4 c : COLOR;
//CHECK: E31202
//CHECK: modifier 'linear' is redundant or conflicting with existing modifier 'pervertex'
}

void main()
{
}
