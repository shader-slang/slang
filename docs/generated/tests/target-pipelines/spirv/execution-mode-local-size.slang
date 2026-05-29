//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=[numthreads(8,4,2)] lowers to OpExecutionMode %main LocalSize 8 4 2 in SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSL [numthreads(X,Y,Z)] attribute survives Phase A/B/C and
// lowers in Phase D's word emission to OpExecutionMode LocalSize
// X Y Z attached to the entry-point function.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: OpExecutionMode %{{[a-zA-Z_0-9]+}} LocalSize 8 4 2
