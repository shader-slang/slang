//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -cuda -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -mtl -compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -vk -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_0 -shaderobj -render-feature hardware-device
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -llvm -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    bool result = true;
    result = result && firstbithigh(0ll) == -1;
    result = result && firstbithigh(1ull) == 0;
    result = result && firstbithigh(2ll) == 1;
    result = result && firstbithigh(3ull) == 1;
    result = result && firstbithigh(4ll) == 2;
    result = result && firstbithigh(5ull) == 2;
    result = result && firstbithigh(6ll) == 2;
    result = result && firstbithigh(7ull) == 2;
    result = result && firstbithigh(8ll) == 3;
    result = result && firstbithigh(1llu << 36) == 36;

    result = result && firstbithigh(-1ll) == -1;
    result = result && firstbithigh(-2ll) == 0;
    result = result && firstbithigh(-3ll) == 1;
    result = result && firstbithigh(-4ll) == 1;
    result = result && firstbithigh(-5ll) == 2;
    result = result && firstbithigh(-6ll) == 2;
    result = result && firstbithigh(-7ll) == 2;
    result = result && firstbithigh(-8ll) == 2;
    result = result && firstbithigh(-9ll) == 3;

    outputBuffer[0] = (result == true) ? 1 : 0;
}

// CHECK: 1
