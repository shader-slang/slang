//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A user type conforming to `IFloat` needs no new members for the generic `min`/`max` overloads
// (shader-slang/slang#11075): it inherits default `__min`/`__max` that order values with its own
// `lessThan`, as the `IComparable` overloads always did. `lessThan` here is deliberately reversed,
// so the results show which ordering was used.

struct Rev : IFloat, IDifferentiable
{
    float v;
    [Differentiable] __init(float value) { v = value; }
    [Differentiable] __init(Rev other) { v = other.v; }
    [Differentiable] float toFloat() { return v; }
    [Differentiable] Rev add(Rev o) { return Rev(v + o.v); }
    [Differentiable] Rev sub(Rev o) { return Rev(v - o.v); }
    [Differentiable] Rev mul(Rev o) { return Rev(v * o.v); }
    [Differentiable] Rev div(Rev o) { return Rev(v / o.v); }
    [Differentiable] Rev mod(Rev o) { return Rev(v % o.v); }
    [Differentiable] Rev neg() { return Rev(-v); }
    [Differentiable] Rev scale<T : __BuiltinFloatingPointType>(T s) { return Rev(v * s.toFloat()); }
    bool equals(Rev o) { return v == o.v; }
    bool lessThan(Rev o) { return v > o.v; }
    bool lessThanOrEquals(Rev o) { return v >= o.v; }
}

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = genMin(Rev(1.0), Rev(2.0)).v; // CHECK: 2.0
    outputBuffer[1] = genMax(Rev(1.0), Rev(2.0)).v; // CHECK: 1.0
}
