//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#vkinput_attachment_index-and-descriptor-space-occupancy
//META: doc_section_digest=d2fe0dddc6cc4095f50d2538cb7552381a2cbc93b5d58b9250e25e6460a15b15
//META: purpose=Skipping the descriptor-space reservation for an input-attachment index does not weaken overlap detection: two parameters requesting the same index are still diagnosed.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the narrow exemption applies only to whole-space
// occupancy, not to the used-range bookkeeping that drives overlap
// diagnostics. Two SubpassInput parameters ask for the same attachment
// index 5 at different descriptor bindings; the explicit-binding-overlap
// warning must still fire, so the index range was recorded even though the
// placeholder space was not marked used.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage fragment

[shader("fragment")]
float4 main(
    [[vk::binding(7, 3)]] [[vk::input_attachment_index(5)]] SubpassInput<float4> a,
//CHECK:                                                                         ^ see declaration of 'a'
//CHECK:                                                                         ^ see declaration of 'a'
    [[vk::binding(8, 3)]] [[vk::input_attachment_index(5)]] SubpassInput<float4> b)
//CHECK:                                                                         ^ explicit binding overlap
//CHECK:                                                                         ^ explicit binding for parameter 'b' overlaps with parameter 'a'
//CHECK:                                                                         ^ explicit binding overlap
//CHECK:                                                                         ^ explicit binding for parameter 'b' overlaps with parameter 'a'
    : SV_Target
{
    return a.SubpassLoad() + b.SubpassLoad();
}
