//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
//META: purpose=Non-entry-point Slang functions are emitted with __device__ decoration in CUDA emit so nvrtc compiles them as device code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The CUDASourceEmitter writes `__device__` (and `__global__` for
// entry points) decorations so nvrtc routes non-kernel functions
// to the device side of the CUDA C++ compilation.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

int square(int x) { return x * x; }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = square(int(tid.x));
}

// CHECK: __device__ {{.*}}square
