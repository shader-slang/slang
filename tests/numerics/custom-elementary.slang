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

    __init(int value)
    {
        this.value = float(value);
    }

    __init(float value)
    {
        this.value = value;
    }

    static This fromScalar(Scalar value)
    {
        return value;
    }

    static Scalar getZero()
    {
        return {0.0};
    }

    static Scalar getOne()
    {
        return {1.0};
    }

    This add(This other)
    {
        return {value + other.value};
    }

    This sub(This other)
    {
        return {value - other.value};
    }

    This mul(This other)
    {
        return {value * other.value};
    }

    This div(This other)
    {
        return {value / other.value};
    }

    This neg()
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

    Mask isEqual(This other)
    {
        return value == other.value;
    }

    Mask isLessThan(This other)
    {
        return value < other.value;
    }

    Mask isLessThanOrEqual(This other)
    {
        return value <= other.value;
    }

    bool equals(This other)
    {
        return isEqual(other);
    }

    bool lessThan(This other)
    {
        return isLessThan(other);
    }

    bool lessThanOrEquals(This other)
    {
        return isLessThanOrEqual(other);
    }
}

extension CustomReal : IExponentialFunctions
{
    This exp()
    {
        return {exp(value)};
    }

    This exp2()
    {
        return {exp2(value)};
    }

    This exp10()
    {
        return {exp10(value)};
    }

    This log()
    {
        return {log(value)};
    }

    This log2()
    {
        return {log2(value)};
    }

    This log10()
    {
        return {log10(value)};
    }

    This pow(This exponent)
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
    This sqrt()
    {
        return {sqrt(value)};
    }

    This rsqrt()
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

T constructFromLiteralTypes<T : IFractional>(int integerValue, float floatingValue)
{
    return T(integerValue) + T(floatingValue);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    CustomReal result = realCurve(constructFromLiteralTypes<CustomReal>(0, 0.5), CustomReal(4.0));
    outputBuffer[0] = result.value;
}
