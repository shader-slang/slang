//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:40Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=25ecf192e380725d185511e33ffea330856108b33c0f2d39992576f85bcc7cd3
//META: purpose=A synthesized differential type propagates an `internal` parent's visibility, so a `public` function naming that `Differential` in its signature is rejected with `use-of-less-visible-type` (30604).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative half of differential visibility propagation:
// the parent struct is explicitly `internal` (the module here uses the
// legacy `public` default, so an un-propagated synthesized type would
// have defaulted to public and slipped through). A `public` signature
// naming `Hidden.Differential` must therefore reference a less-visible
// type; CHECK pins the E30604 rejection at the function declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

internal struct Hidden : IDifferentiable
{
    internal float x;
}

public Hidden.Differential expose(Hidden.Differential d)
// CHECK: E30604
{
    return d;
}
