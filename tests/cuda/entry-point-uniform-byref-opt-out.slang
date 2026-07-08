//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none -cuda-entry-point-params-by-value

// With -cuda-entry-point-params-by-value the legacy ABI is restored: the
// descriptor-table-bearing parameter is passed by value in kernel-argument space
// (and its runtime indexing is covered by the dynamic-index local-copy floor
// instead — see kernel-param-dynamic-index-nested-struct.slang).

struct MyTensor
{
    float* data;
    uint stride;
}

struct TensorList
{
    MyTensor tensors[32];
}

// CHECK: __global__ void computeMain(TensorList_0 list_0, StructuredBuffer<uint> indices_0, RWStructuredBuffer<float> output_0)

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform TensorList list,
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = *(list.tensors[indices[tid.x]].data + tid.x);
}
