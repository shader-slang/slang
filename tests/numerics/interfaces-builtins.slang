// Verify that the public numeric interfaces support the same generic source shape for builtin
// scalars, vectors, and matrices. Keep these cases together so specialization cycles between
// distinct builtin conformances cannot hide behind single-instantiation tests.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces-builtins.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry main -stage compute -o numeric-interfaces-builtins.spv -experimental-feature
//TEST:SIMPLE: -target wgsl -entry wgslMain -stage compute -o numeric-interfaces-builtins.wgsl -experimental-feature

import slang.numerics;

RWStructuredBuffer<float> outputBuffer;

T splatValue<T : INumericShapedType>(T.Scalar value)
{
    return T.fromScalar(value);
}

T zeroValue<T : IAdditive>()
{
    return T.fromScalar(T.getZero());
}

T oneValue<T : INumeric>()
{
    return T.fromScalar(T.getOne());
}

T fromIntegerLiteralType<T : INumeric>(int value)
{
    return T(value);
}

T fromFloatingPointLiteralType<T : IFractional>(float value)
{
    return T(value);
}

T addValues<T : IAdditive>(T left, T right)
{
    return left + right;
}

T multiplyValues<T : INumeric>(T left, T right)
{
    return left * right;
}

T integerQuotientAndRemainder<T : IIntegerType>(T left, T right)
{
    return left / right + left % right;
}

T integerBitwise<T : IIntegerType>(T left, T right)
{
    return (left & right) | (left ^ ~right);
}

T integerShift<T : IIntegerType>(T value, T.ShiftAmount left, T.ShiftAmount right)
{
    value <<= left;
    return value >> right;
}

T signedIntegerValue<T : ISignedIntegerType>(T value)
{
    return -value;
}

T unsignedIntegerValue<T : IUnsignedIntegerType>(T value)
{
    return value;
}

T negateValue<T : ISignedNumeric>(T value)
{
    return -value;
}

T absoluteValue<T : ISignedNumeric>(T value)
{
    return abs(value);
}

T.Mask lessThanValues<T : IComponentwiseOrdered>(T left, T right)
{
    return left < right;
}

bool allEqual<T : IComponentwiseEquatable>(T left, T right)
{
    return all(left == right);
}

T fractionalCurve<T : IFractional>(T value)
{
    return abs(value) + rcp(value) + value * value + value / value + divide(value, value);
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
T floatingPointRounding<T : IFloatingPoint>(T value)
{
    return floor(value) + ceil(value) + round(value) + rint(value) + trunc(value);
}

T floatingPointFloorOnly<T : IFloatingPoint>(T value)
{
    return floor(value);
}

T floatingPointFractionalParts<T : IFloatingPoint>(T value)
{
    return frac(value) + fract(value);
}

T floatingPointBinaryRepresentationOperations<T : IFloatingPoint>(T value, T other)
{
    return fmod(value, other) + copysign(value, other);
}

T floatingPointCompoundOperations<T : IFloatingPoint>(T value, T other)
{
    value += other;
    value -= other;
    value *= other;
    value /= other;
    value %= other;
    return value % other;
}

T floatingPointIncrementOperations<T : IFloatingPoint>(T value)
{
    T result = +value;
    result += ++value;
    result += value++;
    result += --value;
    result += value--;
    return result;
}

T floatingPointSplit<T : IFloatingPoint>(T value, out T integralPart)
{
    return modf(value, integralPart);
}

T.Mask floatingPointIsNaN<T : IFloatingPoint>(T value)
{
    return isnan(value);
}

T.Mask floatingPointIsFinite<T : IFloatingPoint>(T value)
{
    return isfinite(value);
}

T.Mask floatingPointIsInfinite<T : IFloatingPoint>(T value)
{
    return isinf(value);
}

T elementaryCurve<T : IFractional & IElementaryFunctions>(T value)
{
    T sineValue;
    T cosineValue;
    sincos(value, sineValue, cosineValue);

    return exp(value) + exp2(value) + exp10(value) + log(value) + log2(value) + log10(value) +
        sin(value * value) + cos(value) + tan(value) + asin(value) + acos(value) + atan(value) +
        atan2(value, value) + sinh(value) + cosh(value) + tanh(value) + asinh(value) +
        acosh(value + value + value) + atanh(value) + pow(value, value) + sqrt(value) +
        rsqrt(value) + powr(value, value) + sineValue + cosineValue;
}

T elementaryBase10Log<T : IExponentialFunctions>(T value)
{
    return log10(value);
}

T realMinimum<T : IReal>(T left, T right)
{
    return min(left, right);
}

T realMaximum<T : IReal>(T left, T right)
{
    return max(left, right);
}

T realThreeWayExtrema<T : IReal>(T x, T y, T z)
{
    return min3(x, y, z) + max3(x, y, z) + median3(x, y, z);
}

T realPositiveDifference<T : IReal>(T x, T y)
{
    return fdim(x, y);
}

T floatingPointExtrema<T : IFloatingPoint & IReal>(T x, T y, T z)
{
    return fmin(x, y) + fmax(x, y) + fmin3(x, y, z) + fmax3(x, y, z) +
        fmedian3(x, y, z);
}

T realClamp<T : IReal>(T value, T minimumValue, T maximumValue)
{
    return clamp(value, minimumValue, maximumValue);
}

T realSaturate<T : IReal>(T value)
{
    return saturate(value);
}

T fractionalLerp<T : IFractional>(T x, T y, T factor)
{
    return lerp(x, y, factor);
}

T realSmoothstep<T : IReal>(T minimumValue, T maximumValue, T value)
{
    return smoothstep(minimumValue, maximumValue, value);
}

T realStep<T : IReal>(T edge, T value)
{
    return step(edge, value);
}

T legacyArithmeticIdentity<T : IArithmetic>(T value)
{
    return value;
}

T legacyIntegerIdentity<T : IInteger>(T value)
{
    return value;
}

vector<T, 2> legacyBuiltinFloatVector<T : __BuiltinFloatingPointType>(vector<T, 2> value)
{
    return legacyArithmeticIdentity(value);
}

matrix<T, 2, 2> legacyBuiltinFloatMatrix<T : __BuiltinFloatingPointType>(matrix<T, 2, 2> value)
{
    return legacyArithmeticIdentity(value);
}

vector<T, 2> legacyBuiltinIntegerVector<T : __BuiltinIntegerType>(vector<T, 2> value)
{
    return legacyIntegerIdentity(value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int integerScalar = addValues(1, 2);
    int3 integerVector = multiplyValues(int3(1, 2, 3), int3(4, 5, 6));
    int3 signedIntegerVector = negateValue(integerVector);
    int3 absoluteIntegerVector = absoluteValue(signedIntegerVector);
    int integerDivision = integerQuotientAndRemainder(7, 3);
    uint3 integerBits = integerBitwise(uint3(1, 2, 3), uint3(3, 2, 1));
    int shiftedScalar = integerShift(7, 1, 1);
    int3 shiftedVector = integerShift(int3(7, 10, 12), int3(1, 2, 3), int3(0, 1, 2));
    int signedInteger = signedIntegerValue(1);
    uint unsignedInteger = unsignedIntegerValue(1u);

    bool scalarOrder = lessThanValues(1.0, 2.0);
    vector<bool, 3> vectorOrder = lessThanValues(float3(1.0), float3(2.0));
    bool vectorEquality = allEqual(float3(1.0), float3(1.0));

    float3 splatVector = splatValue<float3>(2.0);
    int3 zeroVector = zeroValue<int3>();
    int3 literalIntegerVector = fromIntegerLiteralType<int3>(3);
    float2x2 literalFloatingMatrix = fromFloatingPointLiteralType<float2x2>(0.5);
    float2x2 oneMatrix = oneValue<float2x2>();
    float scalarFractional = fractionalCurve(2.0);
    float3 vectorFractional = fractionalCurve(float3(2.0));
    float2x2 matrixFractional = fractionalCurve(float2x2(2.0));

    float3 vectorRounding = floatingPointRounding(float3(1.5));
    float2x2 matrixFractionalParts = floatingPointFractionalParts(float2x2(1.5));
    float scalarBinaryRepresentation = floatingPointBinaryRepresentationOperations(1.5, -1.0);
    float3 vectorBinaryRepresentation = floatingPointBinaryRepresentationOperations(
        float3(1.5), float3(-1.0));
    float2x2 matrixBinaryRepresentation = floatingPointBinaryRepresentationOperations(
        float2x2(1.5), float2x2(-1.0));
    float scalarCompound = floatingPointCompoundOperations(1.5, 2.0);
    float3 vectorCompound = floatingPointCompoundOperations(float3(1.5), float3(2.0));
    float2x2 matrixCompound =
        floatingPointCompoundOperations(float2x2(1.5), float2x2(2.0));
    float scalarIncremented = floatingPointIncrementOperations(1.5);
    float3 vectorIncremented = floatingPointIncrementOperations(float3(1.5));
    float2x2 matrixIncremented = floatingPointIncrementOperations(float2x2(1.5));
    float scalarIntegralPart;
    float scalarSplitFractional = floatingPointSplit(1.5, scalarIntegralPart);
    float3 vectorIntegralPart;
    float3 vectorSplitFractional = floatingPointSplit(float3(1.5), vectorIntegralPart);
    float2x2 matrixIntegralPart;
    float2x2 matrixSplitFractional = floatingPointSplit(float2x2(1.5), matrixIntegralPart);
    matrix<bool, 2, 2> matrixNaN = floatingPointIsNaN(float2x2(1.5));
    vector<bool, 3> vectorFinite = floatingPointIsFinite(float3(1.5));
    matrix<bool, 2, 2> matrixInfinite = floatingPointIsInfinite(float2x2(1.0 / 0.0));
    float2x2 matrixElementary = elementaryCurve(float2x2(0.5));

    float scalarMinimum = realMinimum(1.0, 2.0);
    float3 vectorMaximum = realMaximum(float3(1.0), float3(2.0));
    float scalarThreeWayExtrema = realThreeWayExtrema(1.0, 2.0, 3.0);
    float3 vectorThreeWayExtrema =
        realThreeWayExtrema(float3(1.0), float3(2.0), float3(3.0));
    float2x2 matrixThreeWayExtrema =
        realThreeWayExtrema(float2x2(1.0), float2x2(2.0), float2x2(3.0));
    float scalarPositiveDifference = realPositiveDifference(3.0, 1.0);
    float3 vectorPositiveDifference = realPositiveDifference(float3(3.0), float3(1.0));
    float2x2 matrixPositiveDifference =
        realPositiveDifference(float2x2(3.0), float2x2(1.0));
    float scalarFloatingPointExtrema = floatingPointExtrema(1.0, 2.0, 3.0);
    float3 vectorFloatingPointExtrema =
        floatingPointExtrema(float3(1.0), float3(2.0), float3(3.0));
    float2x2 matrixFloatingPointExtrema =
        floatingPointExtrema(float2x2(1.0), float2x2(2.0), float2x2(3.0));
    float scalarClamped = realClamp(2.0, 0.0, 1.0);
    float3 vectorSaturated = realSaturate(float3(0.5));
    float3 vectorInterpolated = fractionalLerp(float3(1.0), float3(3.0), float3(0.5));
    float3 vectorSmoothed = realSmoothstep(float3(0.0), float3(1.0), float3(0.5));
    float scalarStep = realStep(1.0, 0.5);
    float3 vectorStep = realStep(float3(0.0), float3(0.5));
    float2 legacyFloatVector = legacyBuiltinFloatVector(float2(1.0));
    float2x2 legacyFloatMatrix = legacyBuiltinFloatMatrix(float2x2(1.0));
    int2 legacyIntegerVector = legacyBuiltinIntegerVector(int2(1));

    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorLeft = 1.0;
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorRight = 2.0;
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorMinimum =
        realMinimum(rowMajorLeft, rowMajorRight);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorClamped =
        realClamp(rowMajorRight, rowMajorLeft, rowMajorRight);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorSmoothed =
        realSmoothstep(rowMajorLeft, rowMajorRight, rowMajorRight);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorStep =
        realStep(rowMajorRight, rowMajorLeft);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorNegative = -1.0;
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorAbsolute =
        absoluteValue(rowMajorNegative);
    matrix<bool, 2, 2, kRowMajorMatrixLayout> rowMajorFinite =
        floatingPointIsFinite(rowMajorLeft);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorRounding =
        floatingPointRounding(rowMajorLeft);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorFractionalParts =
        floatingPointFractionalParts(rowMajorLeft);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorBinaryRepresentation =
        floatingPointBinaryRepresentationOperations(rowMajorLeft, rowMajorRight);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorIntegralPart;
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorSplitFractional =
        floatingPointSplit(rowMajorLeft, rowMajorIntegralPart);
    matrix<float, 2, 2, kRowMajorMatrixLayout> rowMajorElementary =
        elementaryCurve(rowMajorLeft);

    outputBuffer[0] =
        float(integerScalar + signedIntegerVector.x + absoluteIntegerVector.x + integerDivision +
            int(integerBits.x) + shiftedScalar + shiftedVector.x + signedInteger +
            int(unsignedInteger)) +
        float(scalarOrder) + float(vectorOrder.x) + float(vectorEquality) + splatVector.x +
        float(zeroVector.x + literalIntegerVector.x) + literalFloatingMatrix[0][0] +
        oneMatrix[0][0] + scalarFractional + vectorFractional.x +
        matrixFractional[0][0] + vectorRounding.x + matrixFractionalParts[0][0] +
        scalarBinaryRepresentation + vectorBinaryRepresentation.x +
        matrixBinaryRepresentation[0][0] + scalarCompound + vectorCompound.x +
        matrixCompound[0][0] + scalarIncremented + vectorIncremented.x +
        matrixIncremented[0][0] + scalarIntegralPart + scalarSplitFractional +
        vectorIntegralPart.x + vectorSplitFractional.x + matrixIntegralPart[0][0] +
        matrixSplitFractional[0][0] + float(matrixNaN[0][0]) +
        float(vectorFinite.x) + float(matrixInfinite[0][0]) + matrixElementary[0][0] +
        scalarMinimum + vectorMaximum.x + scalarThreeWayExtrema + vectorThreeWayExtrema.x +
        matrixThreeWayExtrema[0][0] + scalarPositiveDifference + vectorPositiveDifference.x +
        matrixPositiveDifference[0][0] + scalarFloatingPointExtrema +
        vectorFloatingPointExtrema.x + matrixFloatingPointExtrema[0][0] + rowMajorMinimum[0][0] +
        rowMajorAbsolute[0][0] + float(rowMajorFinite[0][0]) + rowMajorRounding[0][0] +
        rowMajorFractionalParts[0][0] +
        rowMajorBinaryRepresentation[0][0] + rowMajorIntegralPart[0][0] +
        rowMajorSplitFractional[0][0] + rowMajorElementary[0][0] + scalarClamped +
        vectorSaturated.x + rowMajorClamped[0][0] + vectorInterpolated.x + vectorSmoothed.x +
        rowMajorSmoothed[0][0] + scalarStep + vectorStep.x + rowMajorStep[0][0] +
        legacyFloatVector.x + legacyFloatMatrix[0][0] + float(legacyIntegerVector.x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void wgslMain()
{
    float value = 1.5;
    float2x2 matrixBase10Log = elementaryBase10Log(float2x2(value));
    outputBuffer[0] = floatingPointFloorOnly(value) + floatingPointFractionalParts(value) +
        float(floatingPointIsFinite(value)) + matrixBase10Log[0][0];
}
