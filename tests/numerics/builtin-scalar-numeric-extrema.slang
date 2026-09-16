//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o builtin-scalar-numeric-extrema.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o builtin-scalar-numeric-extrema.spv -experimental-feature

// Verify that builtin scalar numeric constraints include extrema operations across signed-integer,
// unsigned-integer, and floating-point domains.

// CHECK: type: int32_t
// CHECK-NEXT: 10
// CHECK-NEXT: 20
// CHECK-NEXT: 2
// CHECK-NEXT: 7
// CHECK-NEXT: 12
// CHECK-NEXT: 10
// CHECK-NEXT: 20
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

T boundedProductSum<T : IBuiltinScalarNumeric>(T left, T right, T upperBound)
{
    T result = left * right + left;
    T bounded = min(max(result, T.zero()), upperBound);
    return clamp(bounded, T.zero(), upperBound);
}

vector<T, N> boundedVectorProductSum<T : IBuiltinScalarNumeric, let N : int>(
    vector<T, N> left,
    vector<T, N> right,
    vector<T, N> upperBound)
{
    vector<T, N> result;
    for (int index = 0; index < N; ++index)
        result[index] = boundedProductSum(left[index], right[index], upperBound[index]);
    return result;
}

void requireBuiltinScalarNumeric<T : IBuiltinScalarNumeric>()
{}

void requireNumericExtrema<T : INumericExtrema>()
{}

void proveBuiltinScalarSignedNumericRefinement<T : IBuiltinScalarSignedNumeric>()
{
    requireBuiltinScalarNumeric<T>();
    requireNumericExtrema<T>();
}

void proveBuiltinScalarIntegerTypeRefinement<T : IBuiltinScalarIntegerType>()
{
    requireBuiltinScalarNumeric<T>();
    requireNumericExtrema<T>();
}

void proveBuiltinScalarUnsignedIntegerTypeRefinement<T : IBuiltinScalarUnsignedIntegerType>()
{
    requireBuiltinScalarNumeric<T>();
    requireNumericExtrema<T>();
}

void proveBuiltinScalarSignedIntegerTypeRefinement<T : IBuiltinScalarSignedIntegerType>()
{
    requireBuiltinScalarNumeric<T>();
    requireNumericExtrema<T>();
}

void proveBuiltinScalarFractionalRefinement<T : IBuiltinScalarFractional>()
{
    requireBuiltinScalarNumeric<T>();
    requireNumericExtrema<T>();
}

void proveBuiltinScalarFloatingPointTypeRefinement<T : IBuiltinScalarFloatingPointType>()
{
    requireBuiltinScalarNumeric<T>();
    requireNumericExtrema<T>();
}

void proveBuiltinScalarRealRefinement<T : IBuiltinScalarReal>()
{
    requireBuiltinScalarNumeric<T>();
    requireNumericExtrema<T>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Compile the complete intended builtin scalar domain. The operation-bearing cases below
    // exercise one representative from each major numeric family.
    requireBuiltinScalarNumeric<int8_t>();
    requireBuiltinScalarNumeric<int16_t>();
    requireBuiltinScalarNumeric<int>();
    requireBuiltinScalarNumeric<int64_t>();
    requireBuiltinScalarNumeric<intptr_t>();
    requireBuiltinScalarNumeric<uint8_t>();
    requireBuiltinScalarNumeric<uint16_t>();
    requireBuiltinScalarNumeric<uint>();
    requireBuiltinScalarNumeric<uint64_t>();
    requireBuiltinScalarNumeric<uintptr_t>();
    requireBuiltinScalarNumeric<half>();
    requireBuiltinScalarNumeric<float>();
    requireBuiltinScalarNumeric<double>();

    proveBuiltinScalarSignedNumericRefinement<float>();
    proveBuiltinScalarIntegerTypeRefinement<int>();
    proveBuiltinScalarUnsignedIntegerTypeRefinement<uint>();
    proveBuiltinScalarSignedIntegerTypeRefinement<int>();
    proveBuiltinScalarFractionalRefinement<float>();
    proveBuiltinScalarFloatingPointTypeRefinement<float>();
    proveBuiltinScalarRealRefinement<float>();

    outputBuffer[0] = boundedProductSum(3, 4, 10);
    outputBuffer[1] = int(boundedProductSum(5u, 6u, 20u));
    outputBuffer[2] = int(boundedProductSum(0.75f, 4.0f, 2.5f));

    int2 vectorResult = boundedVectorProductSum(int2(2, 5), int2(4, 3), int2(7, 12));
    outputBuffer[3] = vectorResult.x;
    outputBuffer[4] = vectorResult.y;

    // These calls also verify that the C++ and CUDA preludes provide the helper names emitted for
    // extrema over the small builtin integer types.
    outputBuffer[5] = int(boundedProductSum(int8_t(3), int8_t(4), int8_t(10)));
    outputBuffer[6] = int(boundedProductSum(int16_t(5), int16_t(6), int16_t(20)));
    outputBuffer[7] = int(boundedProductSum(uint8_t(4), uint8_t(2), uint8_t(7)));
    outputBuffer[8] = int(boundedProductSum(uint16_t(2), uint16_t(3), uint16_t(12)));
}
