//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=4b4e0759c8a603e1b1c8bce1cbdccd4b05a30002e4de763fd5617a3d9f9b28cb
//META: purpose=Verifies a maximum-per-dim CUDA dispatch [numthreads(1024,1,1)] still emits as a valid extern "C" __global__ void kernel.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: CUDA documented maximum-per-dimension thread group
// (1024 threads in x). The emit still wraps the entry point as a
// valid CUDA kernel.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1024, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: extern "C" __global__ void main
