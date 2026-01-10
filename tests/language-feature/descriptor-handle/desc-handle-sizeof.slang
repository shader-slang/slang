// Test that sizeof(DescriptorHandle<T>) returns the correct size (8 bytes)
// and can be used in compile-time contexts like array sizes.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<int> outputBuffer;

// Test struct containing DescriptorHandle to verify struct layout calculation
struct TestStruct
{
    float4x4 mat;                    // 64 bytes
    DescriptorHandle<Texture2D> tex; // 8 bytes (aligned to 8)
};

// Test that sizeof(DescriptorHandle) works in compile-time context (array size)
struct CompileTimeTest
{
    uint8_t data[sizeof(DescriptorHandle<Texture2D>)];
};

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = asint(dispatchThreadID.x);

    int size = 0;
    switch (idx)
    {
        // Test sizeof for various DescriptorHandle types
        case 0: size = sizeof(DescriptorHandle<Texture2D>); break;
        case 1: size = sizeof(Texture2D.Handle); break;  // Shorthand syntax
        case 2: size = sizeof(DescriptorHandle<SamplerState>); break;
        case 3: size = sizeof(DescriptorHandle<StructuredBuffer<float4>>); break;

        // Test alignof
        case 4: size = alignof(DescriptorHandle<Texture2D>); break;

        // Test struct containing DescriptorHandle
        case 5: size = sizeof(TestStruct); break;

        // Test compile-time usage (array size)
        case 6: size = sizeof(CompileTimeTest); break;

        // Test sizeof(float4x4) for reference
        case 7: size = sizeof(float4x4); break;
    }

    // Expected output (hex):
    // CHECK: 8
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    // CHECK-NEXT: 48
    // CHECK-NEXT: 8
    // CHECK-NEXT: 40
    outputBuffer[idx] = size;
}
