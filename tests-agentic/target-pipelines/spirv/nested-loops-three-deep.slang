//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=Three nested data-dependent for loops emit exactly three OpLoopMerge instructions; structured control flow preserves the loop nesting.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: nested loop depth >= 3. Each loop must carry its own
// OpLoopMerge marker; simplifyIRForSpirvLegalization's
// removeTrivialSingleIterationLoops cannot collapse them because
// the bounds are data-dependent.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    int sum = 0;
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < x; j++) {
            for (int k = 0; k < x; k++) {
                sum += i * j + k;
            }
        }
    }
    outBuf[tid.x] = sum;
}

// CHECK: OpLoopMerge
// CHECK: OpLoopMerge
// CHECK: OpLoopMerge
