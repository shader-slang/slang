//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias RowMajorFloat2x2 = matrix<float, 2, 2, MatrixLayoutMode.RowMajor>;
typealias ColumnMajorFloat2x3 = matrix<float, 2, 3, MatrixLayoutMode.ColumnMajor>;

[Differentiable]
matrix<T, R, C, L> genericMatrixMinimum<
    T : __BuiltinFloatingPointType,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(matrix<T, R, C, L> left, matrix<T, R, C, L> right)
{
    return min(left, right);
}

[Differentiable]
matrix<T, R, C, L> genericMatrixMaximum<
    T : __BuiltinFloatingPointType,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(matrix<T, R, C, L> left, matrix<T, R, C, L> right)
{
    return max(left, right);
}

[Differentiable]
vector<T, N> genericVectorMinimum<T : __BuiltinFloatingPointType, let N : int>(
    vector<T, N> left,
    vector<T, N> right)
{
    return min(left, right);
}

[Differentiable]
vector<T, N> genericVectorMaximum<T : __BuiltinFloatingPointType, let N : int>(
    vector<T, N> left,
    vector<T, N> right)
{
    return max(left, right);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    RowMajorFloat2x2 left = RowMajorFloat2x2(1.0, 4.0, 5.0, 2.0);
    RowMajorFloat2x2 right = RowMajorFloat2x2(3.0, 2.0, 4.0, 2.0);
    RowMajorFloat2x2 leftTangent = RowMajorFloat2x2(10.0, 20.0, 30.0, 40.0);
    RowMajorFloat2x2 rightTangent = RowMajorFloat2x2(1.0, 2.0, 3.0, 4.0);

    let minimumForward = fwd_diff(
        genericMatrixMinimum<float, 2, 2, MatrixLayoutMode.RowMajor>)(
        diffPair(left, leftTangent), diffPair(right, rightTangent));
    outputBuffer[0] = minimumForward.p[0][0]; // CHECK: 1.0
    outputBuffer[1] = minimumForward.p[0][1]; // CHECK: 2.0
    outputBuffer[2] = minimumForward.p[1][0]; // CHECK: 4.0
    outputBuffer[3] = minimumForward.p[1][1]; // CHECK: 2.0
    outputBuffer[4] = minimumForward.d[0][0]; // CHECK: 10.0
    outputBuffer[5] = minimumForward.d[0][1]; // CHECK: 2.0
    outputBuffer[6] = minimumForward.d[1][0]; // CHECK: 3.0
    outputBuffer[7] = minimumForward.d[1][1]; // CHECK: 22.0

    let maximumForward = fwd_diff(
        genericMatrixMaximum<float, 2, 2, MatrixLayoutMode.RowMajor>)(
        diffPair(left, leftTangent), diffPair(right, rightTangent));
    outputBuffer[8] = maximumForward.p[0][0];   // CHECK: 3.0
    outputBuffer[9] = maximumForward.p[0][1];   // CHECK: 4.0
    outputBuffer[10] = maximumForward.p[1][0];  // CHECK: 5.0
    outputBuffer[11] = maximumForward.p[1][1];  // CHECK: 2.0
    outputBuffer[12] = maximumForward.d[0][0];  // CHECK: 1.0
    outputBuffer[13] = maximumForward.d[0][1];  // CHECK: 20.0
    outputBuffer[14] = maximumForward.d[1][0];  // CHECK: 30.0
    outputBuffer[15] = maximumForward.d[1][1];  // CHECK: 22.0

    var minimumLeft = diffPair(left, RowMajorFloat2x2(0.0));
    var minimumRight = diffPair(right, RowMajorFloat2x2(0.0));
    bwd_diff(genericMatrixMinimum<float, 2, 2, MatrixLayoutMode.RowMajor>)(
        minimumLeft, minimumRight, RowMajorFloat2x2(1.0));
    outputBuffer[16] = minimumLeft.d[0][0];   // CHECK: 1.0
    outputBuffer[17] = minimumLeft.d[0][1];   // CHECK: 0.0
    outputBuffer[18] = minimumLeft.d[1][0];   // CHECK: 0.0
    outputBuffer[19] = minimumLeft.d[1][1];   // CHECK: 0.5
    outputBuffer[20] = minimumRight.d[0][0];  // CHECK: 0.0
    outputBuffer[21] = minimumRight.d[0][1];  // CHECK: 1.0
    outputBuffer[22] = minimumRight.d[1][0];  // CHECK: 1.0
    outputBuffer[23] = minimumRight.d[1][1];  // CHECK: 0.5

    var maximumLeft = diffPair(left, RowMajorFloat2x2(0.0));
    var maximumRight = diffPair(right, RowMajorFloat2x2(0.0));
    bwd_diff(genericMatrixMaximum<float, 2, 2, MatrixLayoutMode.RowMajor>)(
        maximumLeft, maximumRight, RowMajorFloat2x2(1.0));
    outputBuffer[24] = maximumLeft.d[0][0];   // CHECK: 0.0
    outputBuffer[25] = maximumLeft.d[0][1];   // CHECK: 1.0
    outputBuffer[26] = maximumLeft.d[1][0];   // CHECK: 1.0
    outputBuffer[27] = maximumLeft.d[1][1];   // CHECK: 0.5
    outputBuffer[28] = maximumRight.d[0][0];  // CHECK: 1.0
    outputBuffer[29] = maximumRight.d[0][1];  // CHECK: 0.0
    outputBuffer[30] = maximumRight.d[1][0];  // CHECK: 0.0
    outputBuffer[31] = maximumRight.d[1][1];  // CHECK: 0.5

    var vectorMinimumLeft = diffPair(float2(1.0, 4.0), float2(0.0));
    var vectorMinimumRight = diffPair(float2(3.0, 2.0), float2(0.0));
    bwd_diff(genericVectorMinimum<float, 2>)(
        vectorMinimumLeft, vectorMinimumRight, float2(1.0));
    outputBuffer[32] = vectorMinimumLeft.d.x;   // CHECK: 1.0
    outputBuffer[33] = vectorMinimumLeft.d.y;   // CHECK: 0.0
    outputBuffer[34] = vectorMinimumRight.d.x;  // CHECK: 0.0
    outputBuffer[35] = vectorMinimumRight.d.y;  // CHECK: 1.0

    var vectorMaximumLeft = diffPair(float2(1.0, 4.0), float2(0.0));
    var vectorMaximumRight = diffPair(float2(3.0, 2.0), float2(0.0));
    bwd_diff(genericVectorMaximum<float, 2>)(
        vectorMaximumLeft, vectorMaximumRight, float2(1.0));
    outputBuffer[36] = vectorMaximumLeft.d.x;   // CHECK: 0.0
    outputBuffer[37] = vectorMaximumLeft.d.y;   // CHECK: 1.0
    outputBuffer[38] = vectorMaximumRight.d.x;  // CHECK: 1.0
    outputBuffer[39] = vectorMaximumRight.d.y;  // CHECK: 0.0

    ColumnMajorFloat2x3 columnLeft = ColumnMajorFloat2x3(1.0, 4.0, 5.0, 8.0, 3.0, 6.0);
    ColumnMajorFloat2x3 columnRight = ColumnMajorFloat2x3(2.0, 3.0, 7.0, 6.0, 9.0, 0.0);
    ColumnMajorFloat2x3 columnLeftTangent =
        ColumnMajorFloat2x3(10.0, 20.0, 30.0, 40.0, 50.0, 60.0);
    ColumnMajorFloat2x3 columnRightTangent =
        ColumnMajorFloat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);

    let columnMinimumForward = fwd_diff(
        genericMatrixMinimum<float, 2, 3, MatrixLayoutMode.ColumnMajor>)(
        diffPair(columnLeft, columnLeftTangent),
        diffPair(columnRight, columnRightTangent));
    outputBuffer[40] = columnMinimumForward.p[0][0];  // CHECK: 1.0
    outputBuffer[41] = columnMinimumForward.p[0][1];  // CHECK: 3.0
    outputBuffer[42] = columnMinimumForward.p[0][2];  // CHECK: 5.0
    outputBuffer[43] = columnMinimumForward.p[1][0];  // CHECK: 6.0
    outputBuffer[44] = columnMinimumForward.p[1][1];  // CHECK: 3.0
    outputBuffer[45] = columnMinimumForward.p[1][2];  // CHECK: 0.0
    outputBuffer[46] = columnMinimumForward.d[0][0];  // CHECK: 10.0
    outputBuffer[47] = columnMinimumForward.d[0][1];  // CHECK: 2.0
    outputBuffer[48] = columnMinimumForward.d[0][2];  // CHECK: 30.0
    outputBuffer[49] = columnMinimumForward.d[1][0];  // CHECK: 4.0
    outputBuffer[50] = columnMinimumForward.d[1][1];  // CHECK: 50.0
    outputBuffer[51] = columnMinimumForward.d[1][2];  // CHECK: 6.0

    var columnMaximumLeft = diffPair(columnLeft, ColumnMajorFloat2x3(0.0));
    var columnMaximumRight = diffPair(columnRight, ColumnMajorFloat2x3(0.0));
    bwd_diff(genericMatrixMaximum<float, 2, 3, MatrixLayoutMode.ColumnMajor>)(
        columnMaximumLeft, columnMaximumRight, ColumnMajorFloat2x3(1.0));
    outputBuffer[52] = columnMaximumLeft.d[0][0];   // CHECK: 0.0
    outputBuffer[53] = columnMaximumLeft.d[0][1];   // CHECK: 1.0
    outputBuffer[54] = columnMaximumLeft.d[0][2];   // CHECK: 0.0
    outputBuffer[55] = columnMaximumLeft.d[1][0];   // CHECK: 1.0
    outputBuffer[56] = columnMaximumLeft.d[1][1];   // CHECK: 0.0
    outputBuffer[57] = columnMaximumLeft.d[1][2];   // CHECK: 1.0
    outputBuffer[58] = columnMaximumRight.d[0][0];  // CHECK: 1.0
    outputBuffer[59] = columnMaximumRight.d[0][1];  // CHECK: 0.0
    outputBuffer[60] = columnMaximumRight.d[0][2];  // CHECK: 1.0
    outputBuffer[61] = columnMaximumRight.d[1][0];  // CHECK: 0.0
    outputBuffer[62] = columnMaximumRight.d[1][1];  // CHECK: 1.0
    outputBuffer[63] = columnMaximumRight.d[1][2];  // CHECK: 0.0
}
