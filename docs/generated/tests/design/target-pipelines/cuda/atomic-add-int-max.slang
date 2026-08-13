//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=4b70b0a669cc9b83124a252d2add0ed8fb3787692f5a88feaade1d3780fda053
//META: purpose=Verifies an atomic operation at int MAX 2147483647 still lowers to atomicAdd on CUDA emit and the constant is wrapped in int(...).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: atomic-add operand at signed-int MAX. The lowering
// to `atomicAdd` is unaffected by the operand boundary value.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    InterlockedAdd(outBuf[0], int(2147483647));
}

// CHECK: atomicAdd
// CHECK-SAME: int(2147483647)
