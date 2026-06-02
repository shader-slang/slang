//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -bindless-space-index 2 -O0

// CHECK-DAG: %[[UINT:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// CHECK-DAG: %[[V2UINT:[A-Za-z0-9_]+]] = OpTypeVector %[[UINT]] 2
// CHECK-NOT: OpTypeRuntimeArray %[[V2UINT]]
// CHECK-NOT: OpTypePointer StorageBuffer %[[V2UINT]]
// CHECK: OpCompositeConstruct %[[V2UINT]]

struct VectorStruct
{
    uint flags;
    uint2 data;
};

struct HandleStruct
{
    uint flags;
    DescriptorHandle<Texture2D<float4>> handle;
};

[[vk::binding(0, 0)]]
ByteAddressBuffer inputBuffer;

[[vk::binding(1, 0)]]
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    VectorStruct v = inputBuffer.Load<VectorStruct>(0);
    HandleStruct h = inputBuffer.Load<HandleStruct>(0);
    HandleStruct hAligned = inputBuffer.LoadAligned<HandleStruct>(0, 8);

    uint2 handleBits = reinterpret<uint2>(h.handle);
    uint2 alignedHandleBits = reinterpret<uint2>(hAligned.handle);

    outputBuffer[0] = v.flags;
    outputBuffer[1] = v.data.x;
    outputBuffer[2] = v.data.y;
    outputBuffer[3] = h.flags;
    outputBuffer[4] = handleBits.x;
    outputBuffer[5] = handleBits.y;
    outputBuffer[6] = alignedHandleBits.x;
    outputBuffer[7] = alignedHandleBits.y;
}
