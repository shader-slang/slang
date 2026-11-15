//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=73b0a54aa16472868adbc586d5469e6a6d226c42e5fd1a1ff6720178eede46b1
//META: purpose=The `[[vk::binding(...)]]` report is gated on the linkage's targets, so the same declaration compiled for SPIR-V is not diagnosed.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the gate on the vk::binding case is real and not decorative:
// the identical parameter that draws E38010 for the HLSL target must stay
// silent for SPIR-V, where the attribute is not dropped. CHECK asserts the
// module was emitted and CHECK-NOT asserts no E38010 appeared in it.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly

[shader("compute")]
[numthreads(1, 1, 1)]
void main([[vk::binding(2)]] uniform RWStructuredBuffer<int> buf)
{
    buf[0] = 1;
}
// CHECK-NOT: E38010
// CHECK: OpEntryPoint
