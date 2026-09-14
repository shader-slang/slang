//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=4d7da86c9111decda82bc36fc42891b6498a808e7662d276403e78bf864b8dba
//META: purpose=Negative: a multisampled `SubpassInputMS` load on Metal reports warning E56107 because frame-buffer fetch cannot do per-sample reads, and the sample index is ignored.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented multisampled carve-out of
// `legalizeSubpassInputsForMetal`. Metal's frame-buffer-fetch input is a
// resolved per-fragment value, so a per-sample `SubpassLoad(sampleIndex)`
// cannot be expressed and Slang must say so rather than silently emitting
// a wrong read. The test pins the stable code E56107 for that warning plus
// E40100 for the entry-point rename, which are the only two diagnostics
// this compile produces, plus the caret-anchored span form the runner
// reports on the sample-index argument itself.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry main -stage fragment

[[vk::input_attachment_index(0)]] SubpassInputMS<float4> inputMS;

[shader("fragment")]
float4 main() : SV_Target
{
    return inputMS.SubpassLoad(0);
//CHECK:                      ^ Metal framebuffer fetch does not support per-sample reads. The sample index is ignored and the resolved value is returned.
}

//CHECK: E56107
//CHECK: E40100
