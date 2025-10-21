//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry compute_main -stage compute
//TEST:SIMPLE(filecheck=SPV): -target spirv

// Test that we can defer loading big structured buffer elements.

struct Bottom
{
    float bigArray[1024];
    float bottomGetValue(int index) { return bigArray[index]; }
}

struct Root
{
    Bottom bottom;
}

StructuredBuffer<Root> sb;

RWStructuredBuffer<float> outputBuffer;

// Check that we don't load the entire `Root` struct and then do ElementExtract to get to `bigArray[0]`.
// Instead we use access chain all the way to point to the required array element, and load just a single float.

// SPV: OpEntryPoint
// SPV: %[[SBPTRARRAY:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer__arr_float_int_1024
// SPV: %[[SBPTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_float %[[SBPTRARRAY]]
// SPV: %[[VALUE:[A-Za-z0-9_]+]] = OpLoad %float %[[SBPTR]]
// SPV: OpStore %{{.*}} %[[VALUE]]

// CUDA: __device__ float Bottom_bottomGetValue{{.*}}(uint [[PARAM0:[A-Za-z0-9_]+]], int [[PARAM1:[A-Za-z0-9_]+]])
// CUDA:  __ldg(&(&(&(globalParams_0->sb_0){{\[}}[[PARAM0]]{{\]}})->bottom_0)->bigArray_0{{\[}}[[PARAM1]]{{\]}});

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = sb[tid.x].bottom.bottomGetValue(0);
}
