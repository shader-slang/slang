//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Regression test for shader-slang/slang#11075: a generic `min`/`max` constrained by `IFloat` and
// specialized on a matrix reaches the concrete matrix overload without recursing back into the
// generic overload, for both the default and the column-major layout.

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 a2 = float2x2(1, 6, 3, 8), b2 = float2x2(5, 2, 7, 4);
    float2x2 mn2 = genMin(a2, b2);

    float3x3 a3 = float3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    float3x3 b3 = float3x3(9, 8, 7, 6, 5, 4, 3, 2, 1);
    float3x3 mx3 = genMax(a3, b3);

    float2x3 a23 = float2x3(1, 8, 3, 7, 5, 9), b23 = float2x3(4, 2, 6, 3, 6, 1);
    float2x3 mn23 = genMin(a23, b23);

    typealias ColumnMajor2x3 = matrix<float, 2, 3, MatrixLayoutMode.ColumnMajor>;
    ColumnMajor2x3 ca = ColumnMajor2x3(1, 8, 3, 7, 5, 9), cb = ColumnMajor2x3(4, 2, 6, 3, 6, 1);
    ColumnMajor2x3 cmn = genMin(ca, cb);
    ColumnMajor2x3 cmx = genMax(ca, cb);

    int i = 0;
    outputBuffer[i++] = mn2[0][0]; // CHECK: 1.0
    outputBuffer[i++] = mn2[0][1]; // CHECK: 2.0
    outputBuffer[i++] = mn2[1][0]; // CHECK: 3.0
    outputBuffer[i++] = mn2[1][1]; // CHECK: 4.0

    outputBuffer[i++] = mx3[0][0]; // CHECK: 9.0
    outputBuffer[i++] = mx3[0][1]; // CHECK: 8.0
    outputBuffer[i++] = mx3[0][2]; // CHECK: 7.0
    outputBuffer[i++] = mx3[1][0]; // CHECK: 6.0
    outputBuffer[i++] = mx3[1][1]; // CHECK: 5.0
    outputBuffer[i++] = mx3[1][2]; // CHECK: 6.0
    outputBuffer[i++] = mx3[2][0]; // CHECK: 7.0
    outputBuffer[i++] = mx3[2][1]; // CHECK: 8.0
    outputBuffer[i++] = mx3[2][2]; // CHECK: 9.0

    outputBuffer[i++] = mn23[0][0]; // CHECK: 1.0
    outputBuffer[i++] = mn23[0][1]; // CHECK: 2.0
    outputBuffer[i++] = mn23[0][2]; // CHECK: 3.0
    outputBuffer[i++] = mn23[1][0]; // CHECK: 3.0
    outputBuffer[i++] = mn23[1][1]; // CHECK: 5.0
    outputBuffer[i++] = mn23[1][2]; // CHECK: 1.0

    outputBuffer[i++] = cmn[0][0]; // CHECK: 1.0
    outputBuffer[i++] = cmn[0][1]; // CHECK: 2.0
    outputBuffer[i++] = cmn[0][2]; // CHECK: 3.0
    outputBuffer[i++] = cmn[1][0]; // CHECK: 3.0
    outputBuffer[i++] = cmn[1][1]; // CHECK: 5.0
    outputBuffer[i++] = cmn[1][2]; // CHECK: 1.0

    outputBuffer[i++] = cmx[0][0]; // CHECK: 4.0
    outputBuffer[i++] = cmx[0][1]; // CHECK: 8.0
    outputBuffer[i++] = cmx[0][2]; // CHECK: 6.0
    outputBuffer[i++] = cmx[1][0]; // CHECK: 7.0
    outputBuffer[i++] = cmx[1][1]; // CHECK: 6.0
    outputBuffer[i++] = cmx[1][2]; // CHECK: 9.0
}
