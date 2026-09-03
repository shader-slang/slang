// Verify that the differentiable numeric refinements preserve the ordinary math spelling for
// builtin scalar, vector, and matrix types, and that user-defined conformances use the same
// generic wrapper path.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces.hlsl -experimental-feature

import slang.numerics.differentiable;

[Differentiable]
T exerciseDifferentiableNumerics<
    T : IDifferentiableFloatingPoint & IDifferentiableReal>(T value, T other)
{
    T sineValue;
    T cosineValue;
    sincos(value, sineValue, cosineValue);

    T arithmetic = -((value + other) * value / other);
    T floatingPoint = frac(value) + fract(value) + fmod(value, other) +
        copysign(value, other) + value % other;
    T exponential = exp(value) + exp2(value) + exp10(value) + log(value) +
        log2(value) + log10(value) + pow(value, other);
    T trigonometric = sin(value) + cos(value) + tan(value) + sineValue + cosineValue;
    T inverseTrigonometric = asin(value) + acos(value) + atan(value) + atan2(value, other);
    T hyperbolic = sinh(value) + cosh(value) + tanh(value) +
        asinh(value) + acosh(value) + atanh(value);
    T roots = sqrt(value) + rsqrt(value);
    T ordered = min(value, other) + max(value, other) + clamp(value, other, value);
    T derived = saturate(value) + fdim(other, value) + divide(value, other) +
        lerp(value, other, value) + max3(value, other, value) +
        min3(value, other, value) + median3(value, other, value) +
        fmax(value, other) + fmax3(value, other, value) + fmin(value, other) +
        fmin3(value, other, value) + fmedian3(value, other, value) +
        powr(value, other) + smoothstep(value, other, value);
    return abs(arithmetic) + rcp(other) + floatingPoint + exponential + trigonometric +
        inverseTrigonometric + hyperbolic + roots + ordered + derived;
}

[Differentiable]
T genericDifferentiableSin<T : IDifferentiableTrigonometricFunctions>(T value)
{
    return sin(value);
}

[Differentiable]
T constructFromBuiltinFloatingPoint<
    T : IDifferentiableFractional,
    TSource : __BuiltinFloatingPointType>(TSource value)
{
    return T(value);
}

[Differentiable]
float3 useDifferentiableConstruction(half value)
{
    return constructFromBuiltinFloatingPoint<float3>(value);
}

struct CustomTrigonometric : IDifferentiableTrigonometricFunctions
{
    float value;

    [Differentiable]
    This sin()
    {
        return {::sin(value)};
    }

    [Differentiable]
    This cos()
    {
        return {::cos(value)};
    }

    [Differentiable]
    This tan()
    {
        return {::tan(value)};
    }

    [Differentiable]
    void sincos(out This sineResult, out This cosineResult)
    {
        sineResult = this.sin();
        cosineResult = this.cos();
    }
}

[Differentiable]
float useScalar(float value)
{
    return exerciseDifferentiableNumerics(value, value + 1.0);
}

[Differentiable]
float3 useVector(float3 value)
{
    return exerciseDifferentiableNumerics(value, value + 1.0);
}

[Differentiable]
float2x2 useMatrix(float2x2 value)
{
    return exerciseDifferentiableNumerics(value, value + float2x2(1.0));
}

[Differentiable]
CustomTrigonometric useCustom(CustomTrigonometric value)
{
    return genericDifferentiableSin(value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> scalarResult =
        fwd_diff(useScalar)(DifferentialPair<float>(1.0, 1.0));
    DifferentialPair<float3> vectorResult =
        fwd_diff(useVector)(DifferentialPair<float3>(float3(1.0), float3(1.0)));
    DifferentialPair<float2x2> matrixResult = fwd_diff(useMatrix)(
        DifferentialPair<float2x2>(float2x2(1.0), float2x2(1.0)));
    DifferentialPair<CustomTrigonometric> customResult = fwd_diff(useCustom)(
        DifferentialPair<CustomTrigonometric>({1.0}, {1.0}));
    DifferentialPair<float3> constructedResult = fwd_diff(useDifferentiableConstruction)(
        DifferentialPair<half>(half(1.0), half(1.0)));

    var scalarReverse = DifferentialPair<float>(1.0, 0.0);
    bwd_diff(useScalar)(scalarReverse, 1.0);

}
