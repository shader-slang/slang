// Verify representative runtime values for generic arithmetic, elementary functions,
// component-wise comparisons, and logical-mask operations on builtin scalar and shaped types.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

bool approximatelyEqual<T : IFractional & IComponentwiseOrdered>(T actual, T expected)
{
    return all(abs(actual - expected) < T(0.001));
}

T arithmeticCurve<T : IFractional>(T left, T right)
{
    return (left + right) * right + left / right;
}

T exponentialCurve<T : IFractional & IExponentialFunctions>()
{
    return exp(T(0.0)) + exp2(T(3.0)) + exp10(T(2.0)) + log(T(1.0)) +
        log2(T(8.0)) + log10(T(100.0)) + pow(T(2.0), T(3.0));
}

T trigonometricCurve<T : IFractional & ITrigonometricFunctions>()
{
    T sine;
    T cosine;
    sincos(T(0.0), sine, cosine);
    return sin(T(0.0)) + cos(T(0.0)) + tan(T(0.0)) + sine + cosine;
}

T inverseTrigonometricCurve<T : IFractional & IInverseTrigonometricFunctions>()
{
    return asin(T(0.0)) + acos(T(1.0)) + atan(T(0.0)) + atan2(T(0.0), T(1.0));
}

T hyperbolicCurve<T : IFractional & IHyperbolicFunctions>()
{
    return sinh(T(0.0)) + cosh(T(0.0)) + tanh(T(0.0)) + asinh(T(0.0)) +
        acosh(T(1.0)) + atanh(T(0.0));
}

T rootCurve<T : IFractional & IRootFunctions>()
{
    return sqrt(T(9.0)) + rsqrt(T(4.0));
}

T.Mask notEqual<T : IComponentwiseEquatable>(T left, T right)
{
    return left != right;
}

bool scalarNotEqual<T : IEquatable>(T left, T right)
{
    return left != right;
}

T.Mask greaterThan<T : IComponentwiseOrdered>(T left, T right)
{
    return left > right;
}

T.Mask greaterThanOrEqual<T : IComponentwiseOrdered>(T left, T right)
{
    return left >= right;
}

M composeMasks<M : IBooleanMask>(M left, M right)
{
    return left.logicalAnd(right).logicalOr(left.logicalNot());
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = int(approximatelyEqual(arithmeticCurve(2.0, 4.0), 24.5));
    outputBuffer[1] = int(approximatelyEqual(
        arithmeticCurve(float3(2.0), float3(4.0)), float3(24.5)));
    outputBuffer[2] = int(approximatelyEqual(
        arithmeticCurve(float2x2(2.0), float2x2(4.0)), float2x2(24.5)));
    outputBuffer[3] = int(approximatelyEqual(exponentialCurve<float>(), 122.0));
    outputBuffer[4] = int(approximatelyEqual(exponentialCurve<float3>(), float3(122.0)));
    outputBuffer[5] = int(approximatelyEqual(trigonometricCurve<float>(), 2.0));
    outputBuffer[6] = int(approximatelyEqual(inverseTrigonometricCurve<float>(), 0.0));
    outputBuffer[7] = int(approximatelyEqual(hyperbolicCurve<float>(), 1.0));
    outputBuffer[8] = int(approximatelyEqual(rootCurve<float2x2>(), float2x2(3.5)));

    let left = float3(1.0, 3.0, 2.0);
    let right = float3(2.0);
    let unequal = notEqual(left, right);
    let greater = greaterThan(left, right);
    let greaterOrEqual = greaterThanOrEqual(left, right);
    outputBuffer[9] = int(unequal.x && unequal.y && !unequal.z);
    outputBuffer[10] = int(!greater.x && greater.y && !greater.z);
    outputBuffer[11] = int(!greaterOrEqual.x && greaterOrEqual.y && greaterOrEqual.z);

    let composed = composeMasks(bool3(true, false, true), bool3(false, true, true));
    outputBuffer[12] = int(!composed.x && composed.y && composed.z);
    outputBuffer[13] = int(scalarNotEqual(1.0, 2.0));
}
