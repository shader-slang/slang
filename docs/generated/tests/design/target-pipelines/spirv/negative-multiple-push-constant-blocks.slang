//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=9bf971e79309b04b98ac1b07e618148a41a5c833af2e6a7f18f5e4b57eddceb9
//META: purpose=An entry point that uses two push-constant blocks is rejected by checkForMultiplePushConstantBlocksPerEntryPoint with E39033 and no SPIR-V is produced.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: legalizeIRForSPIRV counts, per entry point, the globals legalized into a
// PushConstant variable and diagnoses more than one. The CHECK pins the error code
// E39033 so an unrelated failure cannot satisfy the test.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target spirv-asm -entry main -stage compute

struct P { float4 a; };
[[vk::push_constant]] ConstantBuffer<P> p1;
[[vk::push_constant]] ConstantBuffer<P> p2;
RWStructuredBuffer<float> o;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    o[0] = p1.a.x + p2.a.y;
}

// CHECK: E39033
