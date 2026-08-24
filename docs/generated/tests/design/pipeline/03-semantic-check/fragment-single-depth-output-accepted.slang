//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=cb835744c66906760cd040d4b867053da3041145426b47d4afb564e26340984f
//META: purpose=A fragment entry point that declares exactly one depth system-value output is accepted.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "at most one" boundary from the accepting side, so the
// rejection test is known to be counting rather than banning depth outputs
// outright. CHECK asserts the emitted HLSL still carries the depth semantic
// on the output parameter.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl

[shader("fragment")]
float4 main(out float d: SV_Depth): SV_Target
{
    d = 0.5;
    return float4(1, 1, 1, 1);
}
// CHECK: SV_Depth
