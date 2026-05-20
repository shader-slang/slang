//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
//META: purpose=Verifies the minimum-thread dispatch shape [numthreads(1,1,1)] still produces an extern "C" __global__ void main_0 entry point on CUDA emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: minimum [numthreads] dispatch shape. CUDA kernel
// wrapping is independent of the thread-group size; the single-
// thread shape still emits a valid kernel signature.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: extern "C" __global__ void main
