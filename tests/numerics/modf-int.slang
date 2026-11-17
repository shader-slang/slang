//TEST:SIMPLE(filecheck=NATIVE): -target hlsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=NATIVE): -target spirv -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=NATIVE): -target glsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=NATIVE): -target metal -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=NATIVE): -target wgsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=NATIVE): -target cuda -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=NATIVE): -target cpp -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=NATIVE): -target spirv -entry computeMain -stage compute -experimental-feature -DTEST_FLOAT=half
//TEST:SIMPLE(filecheck=NATIVE): -target spirv -entry computeMain -stage compute -experimental-feature -DTEST_FLOAT=double -DTEST_DOUBLE_PRECISION
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type -Xslang -experimental-feature -Xslang -DTEST_FLOAT=double -Xslang -DTEST_DOUBLE_PRECISION
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -output-using-type -Xslang -experimental-feature

// We check integer outputs through both representation-independent interfaces and the public
// builtin aliases. Non-square matrices distinguish their dimensions, explicit layouts check both
// associated output types, and negative inputs distinguish truncation toward zero from flooring.
// NATIVE: {{[Mm]odf}}
// CHECK: type: int32_t
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1$}}

#ifndef TEST_FLOAT
#define TEST_FLOAT float
#endif

import slang.numerics;

typealias TestScalar = TEST_FLOAT;
typealias TestVector = vector<TestScalar, 3>;
typealias TestMatrix = matrix<TestScalar, 2, 3>;
typealias RowMajorMatrix = matrix<TestScalar, 2, 3, MatrixLayoutMode.RowMajor>;
typealias ColumnMajorMatrix = matrix<TestScalar, 2, 3, MatrixLayoutMode.ColumnMajor>;

//TEST_INPUT:ubuffer(data=[-2.75 2.75 -0.5 0.5 -3.0 0.0], stride=4):name=inputBuffer
StructuredBuffer<float> inputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

/// Split an opaque numeric shape using its associated integer-output type.
T splitShape<T : IFloatingPointWithIntegerPart>(T value, out T.IntegerPart wholePart)
{
    return modf(value, wholePart);
}

/// Split an opaque scalar using its associated scalar integer output.
T splitScalar<T : IScalarFloatingPointWithIntegerPart>(T value, out T.IntegerPart wholePart)
{
    return modf(value, wholePart);
}

/// Split a floating-point value through the floating-output `IFloatingPoint` requirement.
T splitFloating<T : IFloatingPoint>(T value, out T wholePart)
{
    return modf(value, wholePart);
}

/// Split a builtin scalar through its public numerics alias.
T splitBuiltinScalar<T : IBuiltinScalarFloatingPointType>(T value, out int wholePart)
{
    return modf(value, wholePart);
}

/// Split a builtin vector whose element type uses the public numerics alias.
vector<T, N> splitBuiltinVector<T : IBuiltinScalarFloatingPointType, let N : int>(
    vector<T, N> value,
    out vector<int, N> wholePart)
{
    return modf(value, wholePart);
}

/// Split a builtin matrix whose element type uses the public numerics alias.
matrix<T, R, C, L> splitBuiltinMatrix<
    T : IBuiltinScalarFloatingPointType,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(matrix<T, R, C, L> value, out matrix<int, R, C, L> wholePart)
{
    return modf(value, wholePart);
}

/// Fill the associated integer-output shape with a scalar value.
T.IntegerPart integerPartFromScalar<T : IFloatingPointWithIntegerPart>(int value)
{
    return T.IntegerPart.fromScalar(T.IntegerPart.Scalar(value));
}

/// Require the associated integer output to be exactly the expected type, including layout.
void requireIntegerPartType<T : IFloatingPointWithIntegerPart, TInteger>()
    where T.IntegerPart == TInteger
{
}

/// Return whether all logical integer matrix elements match the expected values.
bool integerMatrixEquals<let R : int, let C : int, let L : MatrixLayoutMode>(
    matrix<int, R, C, L> value,
    matrix<int, R, C> expected)
{
    // The matrix conformance supplies shape only, so we compare through its integer rows.
    for (int row = 0; row < R; ++row)
    {
        if (!value[row].isEqualTo(expected[row]).all())
            return false;
    }
    return true;
}

/// Record independent checks of whole parts, fractions, and native fractional zero signs.
[numthreads(1, 1, 1)]
void computeMain()
{
    TestMatrix input = TestMatrix(
        TestScalar(inputBuffer[0]), TestScalar(inputBuffer[1]), TestScalar(inputBuffer[2]),
        TestScalar(inputBuffer[3]), TestScalar(inputBuffer[4]), TestScalar(inputBuffer[5]));
    TestMatrix expectedFractions = TestMatrix(-0.75, 0.75, -0.5, 0.5, 0.0, 0.0);
    int2x3 expectedWholeParts = int2x3(-2, 2, 0, 0, -3, 0);
#ifdef TEST_DOUBLE_PRECISION
    // We form +/-16777217.5 in double precision from runtime inputs. Narrowing before the split
    // would lose both the fractional part and the low integer bit in every tested shape.
    input[0][0] = TestScalar(inputBuffer[2]) - TestScalar(16777217);
    input[0][1] = TestScalar(inputBuffer[3]) + TestScalar(16777217);
    expectedFractions[0][0] = -0.5;
    expectedFractions[0][1] = 0.5;
    expectedWholeParts[0][0] = -16777217;
    expectedWholeParts[0][1] = 16777217;
#endif

    int scalarWhole;
    TestScalar scalarFraction = splitScalar(input[0][0], scalarWhole);
    outputBuffer[0] = int(scalarFraction == expectedFractions[0][0]);
    outputBuffer[1] = int(scalarWhole == expectedWholeParts[0][0]);
    int3 vectorWhole;
    TestVector vectorFraction = splitShape(input[0], vectorWhole);
    outputBuffer[2] = int(vectorFraction.isEqualTo(expectedFractions[0]).all());
    outputBuffer[3] = int(vectorWhole.isEqualTo(expectedWholeParts[0]).all());

    // Type-equality constraints establish the associated integer matrix layout without relying
    // on assignments, which would permit an implicit conversion between layouts.
    requireIntegerPartType<RowMajorMatrix, row_major int2x3>();
    requireIntegerPartType<ColumnMajorMatrix, column_major int2x3>();
    RowMajorMatrix rowMajorInput = input;
    row_major int2x3 rowMajorWhole;
    RowMajorMatrix rowMajorFraction = splitShape(rowMajorInput, rowMajorWhole);
    outputBuffer[4] = int(rowMajorFraction.isEqualTo(RowMajorMatrix(expectedFractions)).all());
    outputBuffer[5] = int(integerMatrixEquals(rowMajorWhole, expectedWholeParts));
    ColumnMajorMatrix columnMajorInput = input;
    column_major int2x3 columnMajorWhole;
    ColumnMajorMatrix columnMajorFraction = splitShape(columnMajorInput, columnMajorWhole);
    outputBuffer[6] = int(
        columnMajorFraction.isEqualTo(ColumnMajorMatrix(expectedFractions)).all());
    outputBuffer[7] = int(integerMatrixEquals(columnMajorWhole, expectedWholeParts));

    // The builtin-alias constraints also keep direct integer-output core calls available.
    scalarFraction = splitBuiltinScalar(input[0][0], scalarWhole);
    outputBuffer[8] = int(scalarFraction == expectedFractions[0][0]);
    outputBuffer[9] = int(scalarWhole == expectedWholeParts[0][0]);
    vectorFraction = splitBuiltinVector(input[0], vectorWhole);
    outputBuffer[10] = int(vectorFraction.isEqualTo(expectedFractions[0]).all());
    outputBuffer[11] = int(vectorWhole.isEqualTo(expectedWholeParts[0]).all());
    columnMajorFraction = splitBuiltinMatrix(columnMajorInput, columnMajorWhole);
    outputBuffer[12] = int(
        columnMajorFraction.isEqualTo(ColumnMajorMatrix(expectedFractions)).all());
    outputBuffer[13] = int(integerMatrixEquals(columnMajorWhole, expectedWholeParts));

    // Numeric zero comparisons above ignore its sign. We compare bits with the
    // floating-output operation so each target's existing negative-zero behavior is preserved.
    TestScalar negativeZero = -TestScalar(inputBuffer[5]);
    TestScalar floatingScalarWhole;
    TestScalar floatingScalarFraction = splitFloating(negativeZero, floatingScalarWhole);
    scalarFraction = splitScalar(negativeZero, scalarWhole);
    outputBuffer[14] = int(
        asuint(float(scalarFraction)) == asuint(float(floatingScalarFraction)));
    TestVector floatingVectorWhole;
    TestVector floatingVectorFraction = splitFloating(
        TestVector(negativeZero), floatingVectorWhole);
    vectorFraction = splitShape(TestVector(negativeZero), vectorWhole);
    outputBuffer[15] = int(
        asuint(float(vectorFraction[0])) == asuint(float(floatingVectorFraction[0])));
    RowMajorMatrix floatingRowWhole;
    RowMajorMatrix floatingRowFraction = splitFloating(
        RowMajorMatrix(negativeZero), floatingRowWhole);
    rowMajorFraction = splitShape(RowMajorMatrix(negativeZero), rowMajorWhole);
    outputBuffer[16] = int(
        asuint(float(rowMajorFraction[0][0])) == asuint(float(floatingRowFraction[0][0])));
    ColumnMajorMatrix floatingColumnWhole;
    ColumnMajorMatrix floatingColumnFraction = splitFloating(
        ColumnMajorMatrix(negativeZero), floatingColumnWhole);
    columnMajorFraction = splitShape(ColumnMajorMatrix(negativeZero), columnMajorWhole);
    outputBuffer[17] = int(
        asuint(float(columnMajorFraction[0][0])) == asuint(float(floatingColumnFraction[0][0])));

    // Both matrix layouts expose the associated shape's scalar-broadcast operation.
    rowMajorWhole = integerPartFromScalar<RowMajorMatrix>(7);
    outputBuffer[18] = int(integerMatrixEquals(rowMajorWhole, int2x3(7)));
    columnMajorWhole = integerPartFromScalar<ColumnMajorMatrix>(7);
    outputBuffer[19] = int(integerMatrixEquals(columnMajorWhole, int2x3(7)));
}
