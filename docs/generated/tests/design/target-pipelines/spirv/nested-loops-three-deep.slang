//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
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
