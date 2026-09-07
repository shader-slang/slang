//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=Vector || (the OR companion to && in legalizeLogicalAndOr's Khronos arm) lowers to OpLogicalOr on bool3.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the OR half of the same legalizeLogicalAndOr claim: on a Khronos
// target a vector-typed || becomes an element-wise logical operation. The
// operands are runtime comparisons on a buffer-loaded int3 so the optimizer
// cannot fold them; CHECK asserts OpLogicalOr on %v3bool. Storing through an
// int3 buffer keeps OpTypeBool out of the StorageBuffer block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int3> outBuf;
StructuredBuffer<int3> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int3 v = inBuf[tid.x];
    bool3 lo = v < 0;
    bool3 hi = v > 100;
    outBuf[tid.x] = int3(lo || hi);
}

// CHECK: OpLogicalOr %v3bool
