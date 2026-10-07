//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature -Xslang -floating-point-mode -Xslang precise
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-features half -output-using-type -Xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-features half -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -render-features half -output-using-type -Xslang -experimental-feature

// Verify numeric-operand selection through the public extrema witness, including half/float/double
// scalars, vectors, and an explicit column-major matrix. The input NaN comes from a buffer so
// constant folding cannot replace the operations before witness specialization.

// CHECK: type: int32_t
// CHECK-NEXT: 3
// CHECK-NEXT: 7
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 5
// CHECK-NEXT: 9
// CHECK-NEXT: 3
// CHECK-NEXT: 5
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 3
// CHECK-NEXT: 7
// CHECK-NEXT: 5

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
//TEST_INPUT:ubuffer(data=[0x7fc00000], stride=4):name=inputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;
StructuredBuffer<uint> inputBuffer;

T numericMinimum<T : INumericExtrema>(T left, T right)
{
    return min(left, right);
}

T numericMaximum<T : INumericExtrema>(T left, T right)
{
    return max(left, right);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float nanValue = asfloat(inputBuffer[0]);
    outputBuffer[0] = int(numericMinimum(3.0, nanValue));
    outputBuffer[1] = int(numericMaximum(nanValue, 7.0));
    outputBuffer[2] = int(isnan(numericMinimum(nanValue, nanValue)));
    outputBuffer[3] = int(isnan(numericMaximum(nanValue, nanValue)));
    double doubleNaN = double(nanValue);
    outputBuffer[4] = int(numericMinimum(double(5.0), doubleNaN));
    outputBuffer[5] = int(numericMaximum(doubleNaN, double(9.0)));
    float2 vectorMinimum = numericMinimum(float2(nanValue, 5.0), float2(3.0, nanValue));
    outputBuffer[6] = int(vectorMinimum.x);
    outputBuffer[7] = int(vectorMinimum.y);
    typealias Matrix = matrix<float, 2, 2, MatrixLayoutMode.ColumnMajor>;
    Matrix matrixMaximum = numericMaximum(
        Matrix(nanValue, -5.0, 7.0, nanValue), Matrix(3.0, nanValue, nanValue, 8.0));
    outputBuffer[8] = int(matrixMaximum[1][0]);
    outputBuffer[9] = int(matrixMaximum[1][1]);
    half halfNaN = half(nanValue);
    outputBuffer[10] = int(numericMinimum(half(3.0), halfNaN));
    outputBuffer[11] = int(numericMaximum(halfNaN, half(7.0)));
    half2 halfVector = numericMinimum(half2(halfNaN, 5.0h), half2(3.0h, halfNaN));
    outputBuffer[12] = int(halfVector.y);
}
