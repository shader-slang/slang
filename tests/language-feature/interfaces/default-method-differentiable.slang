//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

// Test that a default interface method can be differentiable.

interface IFoo<int v>
{
    [Differentiable]
    float getVal(float x);

    [Differentiable]
    float getGreaterVal<int y>(float x)
    {
        return getVal(x) + y + v;
    }
}

struct Impl : IFoo<2>
{
    [Differentiable]
    float getVal(float x)
    {
        return x*x;
    }
    
    // Using the default implementation for getGreaterVal.
}

[Differentiable]
float test<int y, T:IFoo<y>>(T v, float x) { return v.getGreaterVal<1>(x); }

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> resultBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Impl impl = {};
    var dpx = diffPair(3.0);
    bwd_diff(test)(impl, dpx, 1.0f);
    resultBuffer[0] = dpx.d;
    // CHECK: 6.0
}
