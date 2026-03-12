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
    result = result && reversebits(0ull) == 0ull;
    result = result && reversebits(-1ll) == -1ll;
    result = result && reversebits(0x5050505050505050ull) == 0x0A0A0A0A0A0A0A0Aull;
    result = result && reversebits(1ull) == (1ull<<63u);
    result = result && reversebits(1ull<<63u) == 1u;

    outputBuffer[0] = (result == true) ? 1 : 0;
}

// CHECK: 1

