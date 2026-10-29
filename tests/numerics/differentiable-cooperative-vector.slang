// Verify that floating-point cooperative vectors satisfy the differentiable fractional,
// elementary-function, and real-ordering interfaces.
//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -experimental-feature -o differentiable-cooperative-vector.cpp

import slang.numerics.differentiable;

[Differentiable]
T exerciseDifferentiableReal<T : IDifferentiableReal>(T value, T other)
{
    T sineValue;
    T cosineValue;
    sincos(value, sineValue, cosineValue);

    return abs((value + other) * value / other) + rcp(other) +
        exp(value) + exp2(value) + exp10(value) + log(value) + log2(value) +
        log10(value) + pow(value, other) + sin(value) + cos(value) + tan(value) +
        sineValue + cosineValue + asin(value) + acos(value) + atan(value) +
        atan2(value, other) + sinh(value) + cosh(value) + tanh(value) + asinh(value) +
        acosh(value) + atanh(value) + sqrt(value) + rsqrt(value) + min(value, other) +
        max(value, other) + clamp(value, other, value);
}

[Differentiable]
T exerciseDifferentiableConstruction<T : IDifferentiableFractional>(float value)
{
    let scalar = T.Scalar(value);
    return T.fromScalar(scalar) + T(value);
}

[Differentiable]
[require(cooperative_vector)]
CoopVec<float, 4> useCooperativeVector(CoopVec<float, 4> value, float scalar)
{
    let other = value + CoopVec<float, 4>(1.0);
    return exerciseDifferentiableReal(value, other) +
        exerciseDifferentiableConstruction<CoopVec<float, 4>>(scalar);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let result = fwd_diff(useCooperativeVector)(
        diffPair(CoopVec<float, 4>(1.0), CoopVec<float, 4>(1.0)),
        diffPair(1.0, 1.0));

    var reverseValue = diffPair(CoopVec<float, 4>(1.0), CoopVec<float, 4>(0.0));
    var reverseScalar = diffPair(1.0, 0.0);
    bwd_diff(useCooperativeVector)(
        reverseValue,
        reverseScalar,
        CoopVec<float, 4>(1.0));
}
