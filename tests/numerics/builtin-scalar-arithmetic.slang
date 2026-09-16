// Verify that the broad builtin scalar arithmetic constraint supports real operations across
// signed-integer, unsigned-integer, and floating-point domains.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o builtin-scalar-arithmetic.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o builtin-scalar-arithmetic.spv -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 10
// CHECK-NEXT: 20
// CHECK-NEXT: 2
// CHECK-NEXT: 7
// CHECK-NEXT: 12
// CHECK-NEXT: 4
// CHECK-NEXT: 3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

T boundedProductSum<T : IBuiltinScalarArithmetic>(T left, T right, T upperBound)
{
    T result = left * right + left;
    return min(max(result, T.zero()), upperBound);
}

T builtinQuotient<T : IBuiltinScalarArithmetic>(T numerator, T denominator)
{
    return numerator / denominator;
}

vector<T, N> boundedVectorProductSum<T : IBuiltinScalarArithmetic, let N : int>(
    vector<T, N> left,
    vector<T, N> right,
    vector<T, N> upperBound)
{
    vector<T, N> result;
    for (int index = 0; index < N; ++index)
        result[index] = boundedProductSum(left[index], right[index], upperBound[index]);
    return result;
}

void requireBuiltinScalarArithmetic<T : IBuiltinScalarArithmetic>()
{}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Compile the complete intended builtin scalar domain. The operation-bearing cases below
    // exercise one representative from each major arithmetic family.
    requireBuiltinScalarArithmetic<int8_t>();
    requireBuiltinScalarArithmetic<int16_t>();
    requireBuiltinScalarArithmetic<int>();
    requireBuiltinScalarArithmetic<int64_t>();
    requireBuiltinScalarArithmetic<intptr_t>();
    requireBuiltinScalarArithmetic<uint8_t>();
    requireBuiltinScalarArithmetic<uint16_t>();
    requireBuiltinScalarArithmetic<uint>();
    requireBuiltinScalarArithmetic<uint64_t>();
    requireBuiltinScalarArithmetic<uintptr_t>();
    requireBuiltinScalarArithmetic<half>();
    requireBuiltinScalarArithmetic<float>();
    requireBuiltinScalarArithmetic<double>();

    outputBuffer[0] = boundedProductSum(3, 4, 10);
    outputBuffer[1] = int(boundedProductSum(5u, 6u, 20u));
    outputBuffer[2] = int(boundedProductSum(0.75f, 4.0f, 2.5f));

    int2 vectorResult = boundedVectorProductSum(int2(2, 5), int2(4, 3), int2(7, 12));
    outputBuffer[3] = vectorResult.x;
    outputBuffer[4] = vectorResult.y;
    outputBuffer[5] = builtinQuotient(9, 2);
    outputBuffer[6] = int(builtinQuotient(6.0f, 2.0f));
}
