//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

// This test verifies that DispatchThreadID parameter of different types 
// correctly extracts components from the underlying uint3 value in CUDA.

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name cudaOutputBuffer
RWStructuredBuffer<float> cudaOutputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: uint _S1 = (blockIdx * blockDim + threadIdx).x;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain2(uint2 tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.y];
}
// CHECK: uint2  _S2 = uint2 {(blockIdx * blockDim + threadIdx).x, (blockIdx * blockDim + threadIdx).y};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain3(int2 tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: uint2  _S3 = uint2 {(blockIdx * blockDim + threadIdx).x, (blockIdx * blockDim + threadIdx).y};
// CHECK: int2  _S4 = make_int2 ((int)_S3.x, (int)_S3.y);
// CHECK: int _S5 = _S4.x;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain4(int tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: int _S6 = int((blockIdx * blockDim + threadIdx).x);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain5(int tid: SV_GroupIndex, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: int _S7 = int((threadIdx.z * blockDim.y + threadIdx.y) * blockDim.x + threadIdx.x);
