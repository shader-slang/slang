// Verify that floating-point cooperative vectors compose the base fractional, elementary,
// component-wise ordering, and real-ordering capabilities.

//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -experimental-feature -o cooperative-vector-floating-point.cpp

import slang.numerics;

RWStructuredBuffer<float> outputBuffer;

T realCurve<T : IReal>(T value, T other)
{
    return min(sin(value) + exp(value) + sqrt(value), other);
}

bool allLessThan<T : IComponentwiseOrdered>(T left, T right)
{
    return all(left < right);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<float, 4> value = CoopVec<float, 4>(0.5);
    CoopVec<float, 4> result = realCurve(value, CoopVec<float, 4>(4.0));
    bool ordered = allLessThan(value, CoopVec<float, 4>(1.0));
    outputBuffer[0] = result[0] + float(ordered);
}
