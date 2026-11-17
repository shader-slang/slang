// Verify that a user-defined scalar can expose floating-point representation operations through
// the public generic entry points without also claiming elementary functions or ordering.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces-custom-floating-point.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry main -stage compute -o numeric-interfaces-custom-floating-point.spv -experimental-feature
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o numeric-interfaces-custom-integer-part.hlsl -experimental-feature -DTEST_INTEGER_PART
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o numeric-interfaces-custom-integer-part.spv -experimental-feature -DTEST_INTEGER_PART
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=INTEGER): -cpu -output-using-type -Xslang -experimental-feature -Xslang -DTEST_INTEGER_PART
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o numeric-interfaces-custom-wide-integer-part.hlsl -experimental-feature -DTEST_INTEGER_PART -DTEST_WIDE_INTEGER_PART
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o numeric-interfaces-custom-wide-integer-part.spv -experimental-feature -DTEST_INTEGER_PART -DTEST_WIDE_INTEGER_PART
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=INTEGER): -cpu -output-using-type -Xslang -experimental-feature -Xslang -DTEST_INTEGER_PART -Xslang -DTEST_WIDE_INTEGER_PART

import slang.numerics;

#ifdef TEST_INTEGER_PART
// The variants without `TEST_INTEGER_PART` require only the floating-output capability.
// Defining `TEST_INTEGER_PART` opts the same type into integer outputs and also checks its
// floating-output split.
//TEST_INPUT:ubuffer(data=[-2.75 0.0 4294967296.0], stride=4):name=inputBuffer
StructuredBuffer<float> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
// INTEGER: type: float
// INTEGER-NEXT: {{^}}1.000000{{$}}
// INTEGER-NEXT: {{^}}1.000000{{$}}
// INTEGER-NEXT: {{^}}1.000000{{$}}
// INTEGER-NEXT: {{^}}1.000000{{$}}
// INTEGER-NEXT: {{^}}1.000000{{$}}
// INTEGER-NEXT: {{^}}1.000000{{$}}
// INTEGER-NEXT: {{^}}1.000000{{$}}
#endif

RWStructuredBuffer<float> outputBuffer;

struct FloatingPointOnly : IScalarFloatingPoint
{
    float value;

    typealias Scalar = FloatingPointOnly;
    typealias Mask = bool;

    __init<TSource : IBuiltinScalarIntegerType>(TSource value)
    {
        this.value = __realCast<float>(value);
    }

    __init<TSource : IBuiltinScalarFloatingPointType>(TSource value)
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

    This plus(This other)
    {
        return {value + other.value};
    }

    This minus(This other)
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

    This ceiling()
    {
        return {ceil(value)};
    }

    This rounded()
    {
        return {round(value)};
    }

    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundedToNearestEven()
    {
        return {rint(value)};
    }

    This truncated()
    {
        return {trunc(value)};
    }

    This fractionalPart()
    {
        return {frac(value)};
    }

    This remainderWhenDividedBy(This divisor)
    {
        return {fmod(value, divisor.value)};
    }

    This copyingSignFrom(This signSource)
    {
        return {copysign(value, signSource.value)};
    }

    This fractionalPart(out This integralPart)
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

#ifdef TEST_INTEGER_PART
extension FloatingPointOnly : IScalarFloatingPointWithIntegerPart
{
#ifdef TEST_WIDE_INTEGER_PART
    typealias IntegerPart = int64_t;
#else
    typealias IntegerPart = int;
#endif

    /// Split the wrapped value into its fractional part and integer whole part.
    This fractionalPart(out IntegerPart integralPart)
    {
        // We preserve the wrapped `float` split and convert only its whole part to `IntegerPart`.
        float wholeValue;
        float fractionalValue = modf(value, wholeValue);
        integralPart = IntegerPart(wholeValue);
        return {fractionalValue};
    }
}

/// Split an opaque custom shape through its associated integer-output type.
T splitIntegerShape<T : IFloatingPointWithIntegerPart>(T value, out T.IntegerPart integralPart)
{
    return modf(value, integralPart);
}

/// Split an opaque custom scalar through its chosen scalar integer-output type.
T splitIntegerScalar<T : IScalarFloatingPointWithIntegerPart>(
    T value,
    out T.IntegerPart integralPart)
{
    return modf(value, integralPart);
}
#endif

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

T constructFromBuiltinTypes<
    T : IFractional,
    TInteger : IBuiltinScalarIntegerType,
    TFloatingPoint : IBuiltinScalarFloatingPointType>(
    TInteger integerValue,
    TFloatingPoint floatingValue)
{
    return T(integerValue) + T(floatingValue);
}

// The test harness runs `computeMain`; the floating-output variants explicitly compile `main`.
[shader("compute")]
[numthreads(1, 1, 1)]
#ifdef TEST_INTEGER_PART
void computeMain()
#else
void main()
#endif
{
#ifdef TEST_INTEGER_PART
    FloatingPointOnly.IntegerPart integerPart;
#ifdef TEST_WIDE_INTEGER_PART
    // We use both signs of 2^32 to prove that neither generic split narrows its integer output
    // through `int`. Integer comparisons occur before the Boolean results are stored as floats.
    float inputValue = -inputBuffer[2];
    float expectedFraction = 0.0;
    FloatingPointOnly.IntegerPart expectedWhole = -int64_t(4294967296);
#else
    float inputValue = inputBuffer[0];
    float expectedFraction = -0.75;
    FloatingPointOnly.IntegerPart expectedWhole = -2;
#endif
    FloatingPointOnly shapeFraction =
        splitIntegerShape(FloatingPointOnly(inputValue), integerPart);
    outputBuffer[0] = float(shapeFraction.value == expectedFraction);
    outputBuffer[1] = float(integerPart == expectedWhole);
    FloatingPointOnly scalarFraction =
        splitIntegerScalar(FloatingPointOnly(-inputValue), integerPart);
    outputBuffer[2] = float(scalarFraction.value == -expectedFraction);
    outputBuffer[3] = float(integerPart == -expectedWhole);

    FloatingPointOnly floatingPart;
    FloatingPointOnly floatingFraction =
        splitRepresentation(FloatingPointOnly(inputBuffer[0]), floatingPart);
    outputBuffer[4] = float(floatingFraction.value == -0.75);
    outputBuffer[5] = float(floatingPart.value == -2.0);

    // We compare bits because the sign of a zero fraction is not observed by numeric equality.
    FloatingPointOnly negativeZero = FloatingPointOnly(-inputBuffer[1]);
    scalarFraction = splitIntegerScalar(negativeZero, integerPart);
    floatingFraction = splitRepresentation(negativeZero, floatingPart);
    outputBuffer[6] = float(asuint(scalarFraction.value) == asuint(floatingFraction.value));
#else
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
    FloatingPointOnly constructed =
        constructFromBuiltinTypes<FloatingPointOnly>(uint64_t(1), half(0.5));
    outputBuffer[0] = rounded.value + fractional.value + binary.value + compound.value +
        incremented.value + integralPart.value + splitFractional.value + constructed.value +
        float(finite) + float(infinite) + float(notANumber);
#endif
}
