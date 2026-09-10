//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metal-specific-runtime-predicates
//META: doc_section_digest=275b7da5949f8c2296fb59114278d0adffc4d57d97d8e80bf7f6abfea9486334
//META: purpose=Negative: Vulkan cross-API options are filtered out of the Metal pipeline, so `-fvk-invert-y` and `-fvk-use-dx-position-w` leave the emitted MSL unchanged.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the filtered-out half of the Metal predicate table by asking
// for two Khronos-only rewrites and showing neither happens: the
// Y-inversion pass would negate the position's y component and the
// position-w pass would introduce a reciprocal. The vertex position is
// derived from a `uniform` so a rewrite could not be constant-folded away;
// CHECK asserts the emitted position assignment carries neither a negation
// of the uniform nor a `1.0f /` reciprocal.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage vertex -fvk-invert-y -fvk-use-dx-position-w

uniform float yScale;

struct VOut
{
    float4 pos : SV_Position;
}

[shader("vertex")]
VOut main(uint vid : SV_VertexID)
{
    VOut o;
    o.pos = float4(float(vid), yScale, 0.0, 1.0);
    return o;
}

// CHECK: vertex
// CHECK-SAME: main_0
// CHECK-NOT: 1.0f /
// CHECK-NOT: - yScale
