//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#warning-groups
//META: doc_section_digest=fe2a3c2e311ba8162e26f4c17f4a54e2d2b52665ed3d562b5445ade975ac7e04
//META: purpose=Enabling the `Pedantic` group with `-Wpedantic` lets a warning tagged with the `pedantic` sentinel fire.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the opt-in half of group gating. The same vertex entry point that is
// silent by default is compiled with `-Wpedantic`; CHECK asserts the 38052 code
// and its message now appear, so the group bit is what gates the warning.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry vertMain -stage vertex -Wpedantic

struct VOut { float4 color : COLOR; };

VOut vertMain(uint id : SV_VertexID)
{
    VOut o;
    o.color = float4(float(id), 0, 0, 1);
    return o;
}

// CHECK: E38052
// CHECK: no output with the 'SV_Position' system value semantic
