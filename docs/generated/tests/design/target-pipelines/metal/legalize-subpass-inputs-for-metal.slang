//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=1b40e6975a461c5c97ead4202c2a70e579c2e6e882749ceb49d57be34bac586c
//META: purpose=`legalizeSubpassInputsForMetal` turns each `SubpassInput<T>` global into a fragment parameter carrying `[[color(N)]]` taken from its input-attachment index, and rewrites every `SubpassLoad` into a reference to it.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the frame-buffer-fetch mapping. Metal has no `subpassLoad`
// intrinsic, so the construct is mapped onto the per-fragment `[[color(N)]]`
// input instead, and any `SubpassLoad` that survived would abort the Metal
// emitter outright. Two attachments with different indices are read so the
// index really comes from the layout decoration rather than declaration
// order; CHECK asserts both `color(0)` and `color(3)` appear and that no
// `SubpassInput` / `SubpassLoad` token is left in the MSL.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage fragment

[[vk::input_attachment_index(0)]] SubpassInput<float4> inputColor;
[[vk::input_attachment_index(3)]] SubpassInput<float4> inputHigh;

[shader("fragment")]
float4 main() : SV_Target
{
    return inputColor.SubpassLoad() + inputHigh.SubpassLoad();
}

// CHECK: #include <metal_stdlib>
// CHECK: fragment{{.*}} main_0
// The two SubpassInputs land on separate `[[color(N)]]` attachments:
// CHECK-DAG: color(0)
// CHECK-DAG: color(3)
// CHECK-NOT: SubpassInput
// CHECK-NOT: SubpassLoad
