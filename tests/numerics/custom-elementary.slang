// Verify that a user-defined scalar can assemble the IReal conjunction from independent
// arithmetic, elementary-function, and ordering conformances.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces-custom-elementary.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry main -stage compute -o numeric-interfaces-custom-elementary.spv -experimental-feature

import slang.numerics;

RWStructuredBuffer<float> outputBuffer;

// Compose the independent general relation with scalar and component-wise numeric ordering. All
// numeric conformances share the Scalar and Mask witnesses below.
struct CustomReal
    : IScalarShapedType,
      IScalarFractional,
      IComponentwiseOrdered,
      IPartiallyOrdered
{
    float value;

    typealias Scalar = CustomReal;
    typealias Mask = bool;

    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        this.value = __realCast<float>(value);
    }

    __init<TSource : __BuiltinFloatingPointType>(TSource value)
    {
        this.value = __realCast<float>(value);
    }

    static This fromScalar(Scalar value)
    {
        return value;
    }

    static This zero()
    {
        return {0.0};
    }

    static This one()
    {
        return {1.0};
    }

    This adding(This other)
    {
        return {value + other.value};
    }

    This subtracting(This other)
    {
        return {value - other.value};
    }

    This multipliedBy(This other)
    {
        return {value * other.value};
    }

    This dividedBy(This other)
    {
        return {value / other.value};
    }

    This negated()
    {
        return {-value};
    }

    This absoluteValue()
    {
        return {abs(value)};
    }

    This reciprocal()
    {
        return {rcp(value)};
    }

    Mask isEqualTo(This other)
    {
        return value == other.value;
    }

    Mask isLessThan(This other)
    {
        return value < other.value;
    }

    Mask isLessThanOrEqualTo(This other)
    {
        return value <= other.value;
    }

}

extension CustomReal : IExponentialFunctions
{
    This exponential()
    {
        return {exp(value)};
    }

    This exponentialWithBase2()
    {
        return {exp2(value)};
    }

    This exponentialWithBase10()
    {
        return {exp10(value)};
    }

    This logarithm()
    {
        return {log(value)};
    }

    This logarithmWithBase2()
    {
        return {log2(value)};
    }

    This logarithmWithBase10()
    {
        return {log10(value)};
    }

    This raisedToThePowerOf(This exponent)
    {
        return {pow(value, exponent.value)};
    }
}

extension CustomReal : ITrigonometricFunctions
{
    This sin()
    {
        return {sin(value)};
    }

    This cos()
    {
        return {cos(value)};
    }

    This tan()
    {
        return {tan(value)};
    }

    void sincos(out This sineResult, out This cosineResult)
    {
        float sineValue;
        float cosineValue;
        sincos(value, sineValue, cosineValue);
        sineResult = {sineValue};
        cosineResult = {cosineValue};
    }
}

extension CustomReal : IInverseTrigonometricFunctions
{
    This asin()
    {
        return {asin(value)};
    }

    This acos()
    {
        return {acos(value)};
    }

    This atan()
    {
        return {atan(value)};
    }

    This atan2(This denominator)
    {
        return {atan2(value, denominator.value)};
    }
}

extension CustomReal : IHyperbolicFunctions
{
    This sinh()
    {
        return {sinh(value)};
    }

    This cosh()
    {
        return {cosh(value)};
    }

    This tanh()
    {
        return {tanh(value)};
    }

    This asinh()
    {
        return {asinh(value)};
    }

    This acosh()
    {
        return {acosh(value)};
    }

    This atanh()
    {
        return {atanh(value)};
    }
}

extension CustomReal : IRootFunctions
{
    This squareRoot()
    {
        return {sqrt(value)};
    }

    This reciprocalSquareRoot()
    {
        return {rsqrt(value)};
    }
}

extension CustomReal : IRealOrderingFunctions
{
    This minimum(This other)
    {
        return {min(value, other.value)};
    }

    This maximum(This other)
    {
        return {max(value, other.value)};
    }

    This step(This edge)
    {
        return {step(edge.value, value)};
    }
}

T realCurve<T : IReal>(T value, T other)
{
    T sineValue;
    T cosineValue;
    sincos(value, sineValue, cosineValue);
    return min(exp(value) + sin(value) + sqrt(value) + sineValue + cosineValue, other) +
        step(value, other);
}

T constructFromBuiltinTypes<
    T : IFractional,
    TInteger : __BuiltinIntegerType,
    TFloatingPoint : __BuiltinFloatingPointType>(
    TInteger integerValue,
    TFloatingPoint floatingValue)
{
    return T(integerValue) + T(floatingValue);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    CustomReal result = realCurve(
        constructFromBuiltinTypes<CustomReal>(int16_t(0), double(0.5)),
        CustomReal(4.0));
    outputBuffer[0] = result.value;
}
