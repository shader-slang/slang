//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces

interface IFoo
{
    static float bar(float a);
}

float zap(float x)
{
    return 2 * x;
}

extension __func_as_type(zap) : IFoo
{
    static float bar(float x)
    {
        return x * x;
    }
}

// The syntax that we'd like eventually:
/*
float zap.bar(float x)
{
    return x * x;
}
*/

float use(IFoo f)
{
    return f.bar(9);
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    // This should just work:
    outputBuffer[tig] = use(zap);
}
