//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
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
