// Verify that a user-defined scalar can expose floating-point representation operations through
// the public generic entry points without also claiming elementary functions or ordering.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces-custom-floating-point.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry main -stage compute -o numeric-interfaces-custom-floating-point.spv -experimental-feature

import slang.numerics;

RWStructuredBuffer<float> outputBuffer;

struct FloatingPointOnly : IScalarFloatingPoint
{
    float value;

    typealias Scalar = FloatingPointOnly;
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
        return {value < 0.0 ? -value : value};
    }

    This reciprocal()
    {
        return {1.0 / value};
    }

    This floor()
    {
        return {floor(value)};
    }

    This ceil()
    {
        return {ceil(value)};
    }

    This round()
    {
        return {round(value)};
    }

    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundEven()
    {
        return {rint(value)};
    }

    This truncate()
    {
        return {trunc(value)};
    }

    This fractionalPart()
    {
        return {frac(value)};
    }

    This remainder(This divisor)
    {
        return {fmod(value, divisor.value)};
    }

    This copySign(This signSource)
    {
        return {copysign(value, signSource.value)};
    }

    This splitFraction(out This integralPart)
    {
        float integralValue;
        float fractionalValue = modf(value, integralValue);
        integralPart = {integralValue};
        return {fractionalValue};
    }

    bool isNaN()
    {
        return isnan(value);
    }

    bool isFinite()
    {
        return isfinite(value);
    }

    bool isInfinite()
    {
        return isinf(value);
    }

}

[require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
T classifyAndRound<T : IFloatingPoint>(
    T value,
    out T.Mask finite,
    out T.Mask infinite,
    out T.Mask notANumber)
{
    finite = isfinite(value);
    infinite = isinf(value);
    notANumber = isnan(value);
    return floor(value) + ceil(value) + round(value) + rint(value) + trunc(value);
}

T fractionalParts<T : IFloatingPoint>(T value)
{
    return frac(value) + fract(value);
}

T binaryRepresentationOperations<T : IFloatingPoint>(T value, T other)
{
    return fmod(value, other) + copysign(value, other);
}

T splitRepresentation<T : IFloatingPoint>(T value, out T integralPart)
{
    return modf(value, integralPart);
}

T compoundRepresentationOperations<T : IFloatingPoint>(T value, T other)
{
    value += other;
    value -= other;
    value *= other;
    value /= other;
    value %= other;
    return value % other;
}

T incrementRepresentationOperations<T : IFloatingPoint>(T value)
{
    T result = +value;
    result += ++value;
    result += value++;
    result += --value;
    result += value--;
    return result;
}

T constructFromLiteralTypes<T : IFractional>(int integerValue, float floatingValue)
{
    return T(integerValue) + T(floatingValue);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    bool finite;
    bool infinite;
    bool notANumber;
    FloatingPointOnly rounded =
        classifyAndRound(FloatingPointOnly(1.5), finite, infinite, notANumber);
    FloatingPointOnly fractional = fractionalParts(FloatingPointOnly(1.5));
    FloatingPointOnly binary =
        binaryRepresentationOperations(FloatingPointOnly(1.5), FloatingPointOnly(-1.0));
    FloatingPointOnly compound = compoundRepresentationOperations(
        FloatingPointOnly(1.5), FloatingPointOnly(2.0));
    FloatingPointOnly incremented = incrementRepresentationOperations(FloatingPointOnly(1.5));
    FloatingPointOnly integralPart;
    FloatingPointOnly splitFractional = splitRepresentation(FloatingPointOnly(1.5), integralPart);
    FloatingPointOnly constructed = constructFromLiteralTypes<FloatingPointOnly>(1, 0.5);
    outputBuffer[0] = rounded.value + fractional.value + binary.value + compound.value +
        incremented.value + integralPart.value + splitFractional.value + constructed.value +
        float(finite) + float(infinite) + float(notANumber);
}
