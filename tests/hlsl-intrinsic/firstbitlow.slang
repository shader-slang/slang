//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(10, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    if (idx < 5) {
        // Test unsigned values
        uint testValues[5] = {0, 1, 2, 4, 8};
        uint value = testValues[idx];
        uint result = firstbitlow(value);
        outputBuffer[idx] = result;
    } else {
        // Test signed values
        int testValues[5] = {-1, -2, -4, -8, 0}; // 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF8, 0
        int value = testValues[idx - 5];
        uint result = firstbitlow(value);
        outputBuffer[idx] = result;
    }
}

// CHECK: FFFFFFFF
// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: FFFFFFFF
