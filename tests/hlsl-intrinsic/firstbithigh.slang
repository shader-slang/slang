//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    bool result = true;
    result = result && firstbithigh(0) == -1;
    result = result && firstbithigh(1) == 0;
    result = result && firstbithigh(2) == 1;
    result = result && firstbithigh(3) == 1;
    result = result && firstbithigh(4) == 2;
    result = result && firstbithigh(5) == 2;
    result = result && firstbithigh(6) == 2;
    result = result && firstbithigh(7) == 2;
    result = result && firstbithigh(8) == 3;

    result = result && firstbithigh(-1) == -1;
    result = result && firstbithigh(-2) == 0;
    result = result && firstbithigh(-3) == 1;
    result = result && firstbithigh(-4) == 1;
    result = result && firstbithigh(-5) == 2;
    result = result && firstbithigh(-6) == 2;
    result = result && firstbithigh(-7) == 2;
    result = result && firstbithigh(-8) == 2;
    result = result && firstbithigh(-9) == 3;

    outputBuffer[0] = (result == true) ? 1 : 0;
}

// CHECK: 1
