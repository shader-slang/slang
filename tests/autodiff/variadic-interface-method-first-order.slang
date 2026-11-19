// Forward derivative through a generic wrapper of a variadic interface method called with a
// non-empty pack. `sumSquares(x, x) = 2 x^2`, so `d/dx = 4 x = 8` at `x = 2`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)

// `a^2` plus the squares of the pack elements.
interface IVariadic
{
    [Differentiable]
    float sumSquares<each T>(float a, expand each T v)
        where T == float;
}

struct Variadic : IVariadic
{
    [Differentiable]
    [NoDiffThis]
    float sumSquares<each T>(float a, expand each T v)
        where T == float
    {
        float s = a * a;
        expand s = s + (each v) * (each v);
        return s;
    }
}

[Differentiable]
float evalVariadic<C : IVariadic>(no_diff C c, float x) { return c.sumSquares(x, x); }

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Variadic variadic;
    // CHECK: {{^}}8.000000{{$}}
    outputBuffer[0] = __fwd_diff(evalVariadic<Variadic>)(variadic, diffPair(2.0, 1.0)).d;
}
