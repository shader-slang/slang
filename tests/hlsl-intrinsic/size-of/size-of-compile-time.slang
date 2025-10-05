//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<int> outputBuffer;

struct Thing<T>
{
    uint8_t data[sizeof(T)];
};

struct AlignThing<T>
{
    uint8_t data[alignof(T)];
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = asint(dispatchThreadID.x);

    int size = 0;
    
    switch (idx)
    {   
        case 0: size = sizeof(Thing<float>); break;
        case 1: size = sizeof(Thing<float3>); break;
        case 2: size = sizeof(AlignThing<float>); break;
        case 3: size = sizeof(AlignThing<float3>); break;
    }

    // CHECK: 4
    // CHECK-NEXT: C
    // CHECK: 4
    // CHECK-NEXT: 4
    outputBuffer[idx] = size;
}
