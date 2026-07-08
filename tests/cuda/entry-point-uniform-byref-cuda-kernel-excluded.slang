//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Positive-shape lock for the by-reference gate's kernel-attribute exclusion: this
// parameter matches the descriptor-table predicate (struct carrying a fixed-size
// array of pointer-backed structs), but the entry point carries [CudaKernel], whose
// generated host launch code owns the by-value ABI — so the parameter must stay by
// value. Without this test, deleting the attribute exclusion would fail nothing:
// the other by-ref tests use plain [shader("compute")] entry points, and the floor
// tests use bare arrays that the struct-only gate rejects anyway.

struct MyTensor
{
    float* data;
    uint stride;
}

struct TensorList
{
    MyTensor tensors[32];
}

// CHECK: __global__ void computeMain(TensorList_0 list_0, RWStructuredBuffer<float> output_0)

[CudaKernel]
[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform TensorList list, uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = *(list.tensors[tid.x].data);
}
