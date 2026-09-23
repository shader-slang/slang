//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=cb835744c66906760cd040d4b867053da3041145426b47d4afb564e26340984f
//META: purpose=A fragment entry point that declares two depth system-value outputs is rejected with E30705 naming the second contributor as conflicting with the first.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the conflicting-depth-output check. Each semantic is individually
// legal, so the conflict is only visible to a pass that collects all depth
// semantics across the signature and counts them. CHECK pins E30705, whose
// message names both the second semantic and the earlier one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

[shader("fragment")]
float4 main(out float d1: SV_Depth, out float d2: SV_DepthGreaterEqual): SV_Target
//CHECK: E30705
{
    d1 = 0.5;
    d2 = 0.5;
    return float4(1, 1, 1, 1);
}
