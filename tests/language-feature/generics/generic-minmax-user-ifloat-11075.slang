//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A user type conforming to `IFloat` needs no new members for the generic `min`/`max` overloads
// (shader-slang/slang#11075): it inherits default `__min`/`__max` that order values with its own
// `lessThan`, as the `IComparable` overloads always did. `lessThan` here is deliberately reversed,
// so the results show which ordering was used.

struct ReversedOrder : IFloat
{
    float v;
    [Differentiable] __init(float value) { v = value; }
    [Differentiable] __init(ReversedOrder other) { v = other.v; }
    [Differentiable] float toFloat() { return v; }
    [Differentiable] ReversedOrder add(ReversedOrder o) { return ReversedOrder(v + o.v); }
    [Differentiable] ReversedOrder sub(ReversedOrder o) { return ReversedOrder(v - o.v); }
    [Differentiable] ReversedOrder mul(ReversedOrder o) { return ReversedOrder(v * o.v); }
    [Differentiable] ReversedOrder div(ReversedOrder o) { return ReversedOrder(v / o.v); }
    [Differentiable] ReversedOrder mod(ReversedOrder o) { return ReversedOrder(v % o.v); }
    [Differentiable] ReversedOrder neg() { return ReversedOrder(-v); }
    [Differentiable] ReversedOrder scale<T : __BuiltinFloatingPointType>(T s) { return ReversedOrder(v * s.toFloat()); }
    bool equals(ReversedOrder o) { return v == o.v; }
    bool lessThan(ReversedOrder o) { return v > o.v; }
    bool lessThanOrEquals(ReversedOrder o) { return v >= o.v; }
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
    outputBuffer[0] = genMin(ReversedOrder(1.0), ReversedOrder(2.0)).v; // CHECK: 2.0
    outputBuffer[1] = genMax(ReversedOrder(1.0), ReversedOrder(2.0)).v; // CHECK: 1.0
}
