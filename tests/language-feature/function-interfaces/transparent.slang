//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Transparent function interfaces allow extensions to be defined on them directly.

[__transparent_function_interface]
__function_interface IFoo
{
    float bar(float);
}

float zap(float x)
{
    return 2 * x;
}

float bar(float x) for zap : IFoo
{
    return x * x;
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[tig] = bar(0);
}
