//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

public interface IFoo
{
    [Differentiable]
    float bar(float x, bool flag);
}

public struct Foo : IFoo, IDifferentiable
{
    [Differentiable]
    public float _bar(float x)
    {
        return x;
    }

    public float bar(float x, bool flag)
    {
        return _bar(x);
    }

    [Differentiable, PrimalSubstituteOf(bar)]
    public float _dbar(float x, bool flag)
    {
        return x * x * x * x * x;
    }
}

[Differentiable]
float test(IFoo foo, float x)
{
    return foo.bar(x, true);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Foo f;
    // Direct call: must go through `bar` (no primal substitute), giving x.
    outputBuffer[0] = test(f, 2.0);
    // CHECK: 2.0

    // Forward diff: primal must come from the primal substitute `_dbar`, giving x^5.
    outputBuffer[1] = __fwd_diff(test)(f, diffPair(2.0, 1.0)).p;
    // CHECK: 32.0

    // Forward diff: differential must come from `_dbar`, giving d/dx(x^5) = 5 x^4.
    outputBuffer[2] = __fwd_diff(test)(f, diffPair(2.0, 1.0)).d;
    // CHECK: 80.0

    // Backward diff: differential must come from `_dbar`, giving d/dx(x^5) = 5 x^4.
    float grad = 1.0f;
    var pair = diffPair(2.0, 0.0);
    __bwd_diff(test)(f, pair, grad);
    outputBuffer[3] = pair.d;
    // CHECK: 80.0
}
