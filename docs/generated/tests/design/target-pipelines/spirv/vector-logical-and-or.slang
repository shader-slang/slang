//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=A vector && (legalizeLogicalAndOr's Khronos arm) lowers to OpLogicalAnd on bool3 operands.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// legalizeLogicalAndOr fires for Khronos targets (SPIR-V here).
// The pass canonicalizes vector-typed short-circuit && / || into
// element-wise SPIR-V logical opcodes. For a bool3 operand pair
// the lowered form is OpLogicalAnd on %v3bool.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<bool3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool3 a = bool3(tid.x > 0, tid.y > 0, tid.z > 0);
    bool3 b = bool3(tid.x < 10, tid.y < 10, tid.z < 10);
    outBuf[tid.x] = a && b;
}

// CHECK: OpLogicalAnd
