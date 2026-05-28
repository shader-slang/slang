//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A for loop with a data-dependent bound survives as SPIR-V structured control flow (OpLoopMerge / OpBranchConditional).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V requires structured control flow. A for loop whose bound
// is data-dependent (not a compile-time constant the simplifier
// can unroll) emits OpLoopMerge plus OpBranchConditional.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;
StructuredBuffer<int> bounds;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;
    for (int i = 0; i < bounds[tid.x]; i++)
        sum += i;
    outBuf[tid.x] = sum;
}

// CHECK: OpLoopMerge
// CHECK: OpBranchConditional
