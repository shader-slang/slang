//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A runtime inf+inf (computed as (a/0)+(a/0) with data-dependent a) emits OpFAdd %float; the operands are derived from a runtime OpFDiv against +0, not folded at compile time.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: inf + inf path. Both operands of the FAdd come
// from a runtime OpFDiv against +0.0, so SCCP cannot fold them.
// IEEE rules say inf + inf = inf; we only assert the SPIR-V
// instruction shape, not the runtime result.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float a = float(tid.x);
    float inf = a / 0.0f;
    outBuf[tid.x] = inf + inf;
}

// CHECK-DAG: %float_0 = OpConstant %float 0
// CHECK: OpFDiv %float
// CHECK: OpFAdd %float
