//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=ae7166b4e043d6e1a4435c167ea8bdc675ca3080f5f01a62c716172c61bce000
//META: purpose=legalizeSubpassInputsForMetal (inside legalizeIRForMetal) rewrites each `IRGlobalParam` of type `SubpassInput<T>` into a fragment entry-point parameter with a `[[color(N)]]` system-value decoration sourced from the `InputAttachmentIndex` layout decoration; every `kIROp_SubpassLoad` use is rewritten into a direct reference to that new parameter. No `SubpassInput` opcode survives into the Metal emit.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal does not have a native subpassLoad intrinsic; instead
// Slang maps the construct onto Metal's frame-buffer-fetch feature
// (the per-fragment `[[color(N)]]` input). `[[vk::input_attachment_index(N)]]`
// supplies the N. Surviving `SubpassLoad` calls trigger
// `SLANG_DIAGNOSE_UNEXPECTED` at emit time, so observing the
// `[[color(N)]]` annotation in the MSL is equivalent to observing
// that the legalization ran.

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
