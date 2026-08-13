//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A runtime float / 0.0 emits as OpFDiv %float against an OpConstant %float 0 (no compile-time fold of the divisor); IEEE behavior is left to the runtime.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: divide-by-zero edge. The dividend is data-dependent
// (from SV_DispatchThreadID), so SCCP cannot fold the FDiv.
// Slang emits OpFDiv against the literal 0.0 constant; the
// IEEE-defined result (+/-inf, or NaN if dividend is 0)
// happens on the device.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float a = float(tid.x);
    outBuf[tid.x] = a / 0.0f;
}

// CHECK-DAG: %float_0 = OpConstant %float 0
// CHECK: OpFDiv %float
