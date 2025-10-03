//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Test that std430 layout is supported for WGSL target
// Uses a struct with float3 + float to demonstrate std430 packing rules

struct Data
{
    float3 a;
    float b;
};

//WGSL: @binding(1) @group(0) var<storage, read_write> outputBuffer
[[vk::binding(1, 0)]]
RWStructuredBuffer<Data, Std430DataLayout> outputBuffer;

//WGSL: @binding(0) @group(0) var<storage, read_write> inputBuffer
[[vk::binding(0, 0)]]
RWStructuredBuffer<Data, Std430DataLayout> inputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    outputBuffer[tid] = inputBuffer[tid];
}
