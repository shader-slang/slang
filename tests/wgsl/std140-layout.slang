//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Test that std140 layout is supported for WGSL target
// Uses a struct that requires @align attributes to meet std140 requirements

struct Data
{
    float x;
    float3 y;  // std140 requires array elements and structs to be aligned to 16 bytes
};

//WGSL-DAG: @binding(2) @group(0) var<storage, read_write> outputBuffer
[[vk::binding(2, 0)]]
RWStructuredBuffer<float4> outputBuffer;

//WGSL-DAG: @binding(0) @group(0) var<uniform> constBuffer
[[vk::binding(0, 0)]]
ConstantBuffer<Data, Std140DataLayout> constBuffer;

//WGSL-DAG: @binding(1) @group(0) var<storage, read_write> structBuffer
[[vk::binding(1, 0)]]
RWStructuredBuffer<Data, Std140DataLayout> structBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    outputBuffer[tid] = float4(constBuffer.x, constBuffer.y) + float4(structBuffer[tid].x, structBuffer[tid].y);
}
