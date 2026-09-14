//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=5af69869d1bb41b9ec6da86b628cac4884b654398df91d571e67dbe22a034ace
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
//CHECK:                   ^^^^^^ references less visible type
//CHECK:                   ^^^^^^ 'expose' references less visible type 'Hidden'.
// CHECK: E30604
{
    return d;
}
