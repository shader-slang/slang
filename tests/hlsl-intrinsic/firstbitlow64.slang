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
    result = result && firstbitlow(0ll) == -1;
    result = result && firstbitlow(1ull) == 0;
    result = result && firstbitlow(2ll) == 1;
    result = result && firstbitlow(3ull) == 0;
    result = result && firstbitlow(4ll) == 2;
    result = result && firstbitlow(5ull) == 0;
    result = result && firstbitlow(6ll) == 1;
    result = result && firstbitlow(7ull) == 0;
    result = result && firstbitlow(8ll) == 3;
    result = result && firstbitlow(1llu << 36) == 36;

    result = result && firstbitlow(-1ll) == 0;
    result = result && firstbitlow(-2ll) == 1;
    result = result && firstbitlow(-3ll) == 0;
    result = result && firstbitlow(-4ll) == 2;
    result = result && firstbitlow(-5ll) == 0;
    result = result && firstbitlow(-6ll) == 1;
    result = result && firstbitlow(-7ll) == 0;
    result = result && firstbitlow(-8ll) == 3;
    result = result && firstbitlow(-9ll) == 0;

    outputBuffer[0] = (result == true) ? 1 : 0;
}

// CHECK: 1
