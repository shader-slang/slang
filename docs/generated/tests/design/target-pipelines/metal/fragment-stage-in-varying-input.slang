//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#resource-type-spellings
//META: doc_section_digest=0f39500a5c2260589167928aaa1a2c794b4b5a7ab8e224fd5cf5024f96aac3ee
//META: purpose=A varying-input parameter takes `[[stage_in]]` instead of a numbered slot, and the fragment entry point carries the `[[fragment]]` stage attribute.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the varying-input half of the attribute rule: interpolated inputs
// are not bound to a numbered slot the way a buffer or texture is, they
// arrive through Metal's single stage-in aggregate. Passing a struct with an
// interpolated field forces that aggregate to be synthesized, and the same
// signature shows the fragment stage attribute. Both attributes are matched
// as bare substrings because a literal double bracket is a FileCheck
// variable reference.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry fragMain -stage fragment

struct VIn
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("fragment")]
float4 fragMain(VIn vin) : SV_Target
{
    return float4(vin.uv, 0, 1);
}

// CHECK: fragment{{.*}} fragMain(
// CHECK-SAME: stage_in
