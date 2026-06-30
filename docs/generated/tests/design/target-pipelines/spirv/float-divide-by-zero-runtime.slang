//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
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
