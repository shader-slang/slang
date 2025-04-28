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

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain2(uint2 tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.y];
}

// CHECK: *(&(dst_0[_S1.x])) = src_0.Load(_S1.x);
// CHECK: *(&(dst_1[_S2.x])) = src_1.Load(_S2.y);