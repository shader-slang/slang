//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature

// Regression test for shader-slang/slang#11075: with `slang.numerics.differentiable` imported,
// `min`/`max` on a generic that names `IDifferentiableRealOrderingFunctions` next to a core
// interface binds the numerics overload (rank -10). The core `IFloat` overload sits below it at
// -11 and the core `IComparable` overload at -13, so neither spelling is ambiguous.

import slang.numerics.differentiable;

T maxComparableOrdering<T : IComparable & IDifferentiableRealOrderingFunctions>(T a, T b)
{
    return max(a, b);
}
T minFloatOrdering<T : IFloat & IDifferentiableRealOrderingFunctions>(T a, T b)
{
    return min(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float3 a = float3(1, 5, 3), b = float3(4, 2, 6);
    float3 mx = maxComparableOrdering(a, b);
    float3 mn = minFloatOrdering(a, b);

    int i = 0;
    outputBuffer[i++] = mx.x; // CHECK: 4.0
    outputBuffer[i++] = mx.y; // CHECK-NEXT: 5.0
    outputBuffer[i++] = mx.z; // CHECK-NEXT: 6.0
    outputBuffer[i++] = mn.x; // CHECK-NEXT: 1.0
    outputBuffer[i++] = mn.y; // CHECK-NEXT: 2.0
    outputBuffer[i++] = mn.z; // CHECK-NEXT: 3.0
}
