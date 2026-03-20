// Can't test -cpu due to missing data layout support on C++ target.
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-llvm -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<int> outputBuffer;

struct Foo
{
    int a;
    float3 b;
};

uint testGeneric<L:IBufferDataLayout>()
{
    return sizeof(Foo[2], L);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // BUF: 10
    outputBuffer[0] = sizeof(Foo);
    // BUF: 10
    outputBuffer[1] = sizeof(Foo, ScalarDataLayout);
    // BUF: 1C
    outputBuffer[2] = sizeof(Foo, Std430DataLayout);
    // BUF: 20
    outputBuffer[3] = testGeneric<ScalarDataLayout>();
    // BUF: 3C
    outputBuffer[4] = testGeneric<Std430DataLayout>();
    // BUF: 20
    outputBuffer[5] = testGeneric<DefaultDataLayout>();
}
