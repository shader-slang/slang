//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -bindless-space-index 2 -O0
//TEST:SIMPLE(filecheck=CHECK_NV): -target spirv-asm -entry computeMainNV -stage compute -bindless-space-index 2 -capability spvBindlessTextureNV -DNV_HANDLE -O0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=RESULT): -vk -compute -shaderobj -xslang -bindless-space-index -xslang 2

// CHECK-DAG: %[[UINT:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// CHECK-DAG: %[[V2UINT:[A-Za-z0-9_]+]] = OpTypeVector %[[UINT]] 2
// CHECK-DAG: %[[ULONG:[A-Za-z0-9_]+]] = OpTypeInt 64 0
// CHECK-NOT: OpTypeRuntimeArray %[[V2UINT]]
// CHECK-NOT: OpTypePointer StorageBuffer %[[V2UINT]]
// CHECK-NOT: OpStore %_ptr_StorageBuffer_v2uint
// CHECK-NOT: OpTypeRuntimeArray %[[ULONG]]
// CHECK-NOT: OpTypePointer StorageBuffer %[[ULONG]]
// CHECK: OpCompositeConstruct %[[V2UINT]]
// CHECK: OpBitwiseOr %[[ULONG]]

// CHECK_NV-DAG: %[[ULONG:[A-Za-z0-9_]+]] = OpTypeInt 64 0
// CHECK_NV-NOT: OpTypeRuntimeArray %[[ULONG]]
// CHECK_NV-NOT: OpTypePointer StorageBuffer %[[ULONG]]
// CHECK_NV: OpBitwiseOr %[[ULONG]]

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
//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7], stride=4):name=inputBuffer
ByteAddressBuffer inputBuffer;

[[vk::binding(1, 0)]]
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[[vk::binding(2, 0)]]
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputByteBuffer
RWByteAddressBuffer outputByteBuffer;

#if defined(NV_HANDLE)

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainNV()
{
    DescriptorHandle<Texture2D<float4>> h =
        inputBuffer.Load<DescriptorHandle<Texture2D<float4>>>(4);
    outputByteBuffer.Store<DescriptorHandle<Texture2D<float4>>>(4, h, 8);

    uint64_t handleBits = (uint64_t)h;

    outputBuffer[0] = uint(handleBits);
    outputBuffer[1] = uint(handleBits >> 32);
    outputBuffer[2] = outputByteBuffer.Load<uint>(4);
    outputBuffer[3] = outputByteBuffer.Load<uint>(8);
}

#else

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    VectorStruct v = inputBuffer.Load<VectorStruct>(0);
    HandleStruct h = inputBuffer.Load<HandleStruct>(0);
    HandleStruct hAligned = inputBuffer.LoadAligned<HandleStruct>(0, 8);
    uint dynamicOffset = dispatchThreadID.x;
    HandleStruct hDynamic = inputBuffer.LoadAligned<HandleStruct>(dynamicOffset, 8);
    uint64_t u64 = inputBuffer.Load<uint64_t>(4);

    outputByteBuffer.Store<HandleStruct>(dynamicOffset, h, 8);
    outputByteBuffer.Store<uint64_t>(20, u64);

    uint2 handleBits = reinterpret<uint2>(h.handle);
    uint2 alignedHandleBits = reinterpret<uint2>(hAligned.handle);
    uint2 dynamicHandleBits = reinterpret<uint2>(hDynamic.handle);

    outputBuffer[0] = v.flags;
    outputBuffer[1] = v.data.x;
    outputBuffer[2] = v.data.y;
    outputBuffer[3] = h.flags;
    outputBuffer[4] = handleBits.x;
    outputBuffer[5] = handleBits.y;
    outputBuffer[6] = alignedHandleBits.x;
    outputBuffer[7] = alignedHandleBits.y;
    outputBuffer[8] = hDynamic.flags;
    outputBuffer[9] = dynamicHandleBits.x;
    outputBuffer[10] = dynamicHandleBits.y;
    outputBuffer[11] = outputByteBuffer.Load<uint>(0);
    outputBuffer[12] = outputByteBuffer.Load<uint>(4);
    outputBuffer[13] = outputByteBuffer.Load<uint>(8);
    outputBuffer[14] = uint(u64);
    outputBuffer[15] = uint(u64 >> 32);
    outputBuffer[16] = outputByteBuffer.Load<uint>(20);
    outputBuffer[17] = outputByteBuffer.Load<uint>(24);
}

#endif

// RESULT: 0
// RESULT-NEXT: 1
// RESULT-NEXT: 2
// RESULT-NEXT: 0
// RESULT-NEXT: 1
// RESULT-NEXT: 2
// RESULT-NEXT: 1
// RESULT-NEXT: 2
// RESULT-NEXT: 0
// RESULT-NEXT: 1
// RESULT-NEXT: 2
// RESULT-NEXT: 0
// RESULT-NEXT: 1
// RESULT-NEXT: 2
// RESULT-NEXT: 1
// RESULT-NEXT: 2
// RESULT-NEXT: 1
// RESULT-NEXT: 2
