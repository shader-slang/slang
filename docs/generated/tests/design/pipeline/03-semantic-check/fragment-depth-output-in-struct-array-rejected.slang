//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=cb835744c66906760cd040d4b867053da3041145426b47d4afb564e26340984f
//META: purpose=The depth-output collection unwraps array parameters and recurses into struct fields, so a depth semantic on a field of an `out` struct array still conflicts with a second depth output.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the nesting edge of the depth-output walk. The first depth
// semantic is buried on a field of a struct inside a one-element `out` array,
// so a walk that only looked at parameter types directly would see a single
// depth output and accept the signature. CHECK pins E30705, proving the
// nested contributor was counted.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

struct DepthOut
{
    float d: SV_Depth;
}

[shader("fragment")]
float4 main(out DepthOut a[1], out float d2: SV_DepthLessEqual): SV_Target
//CHECK: E30705
{
    a[0].d = 0.5;
    d2 = 0.5;
    return float4(1, 1, 1, 1);
}
