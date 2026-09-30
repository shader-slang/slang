//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Regression test for shader-slang/slang#11075: `double` vectors and matrices.
//
// This is the `double` counterpart of generic-minmax-vector-11075.slang, split into its own file
// because Metal has no `double` type.

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    double2 ad = double2(1.5, 9.0), bd = double2(4.0, 2.5);
    double2 mnd = genMin(ad, bd);

    double3 ce = double3(7.5, 1.0, 4.0), de = double3(2.0, 6.0, 4.0);
    double3 mxe = genMax(ce, de);

    double2x2 m1 = double2x2(1.5, 9.0, 4.0, 0.5), m2 = double2x2(4.0, 2.5, 3.0, 8.0);
    double2x2 mnm = genMin(m1, m2);
    double2x2 mxm = genMax(m1, m2);

    int i = 0;
    outputBuffer[i++] = float(mnd.x); // CHECK: 1.5
    outputBuffer[i++] = float(mnd.y); // CHECK: 2.5

    outputBuffer[i++] = float(mxe.x); // CHECK: 7.5
    outputBuffer[i++] = float(mxe.y); // CHECK: 6.0
    outputBuffer[i++] = float(mxe.z); // CHECK: 4.0

    outputBuffer[i++] = float(mnm[0][0]); // CHECK: 1.5
    outputBuffer[i++] = float(mnm[0][1]); // CHECK: 2.5
    outputBuffer[i++] = float(mnm[1][0]); // CHECK: 3.0
    outputBuffer[i++] = float(mnm[1][1]); // CHECK: 0.5
    outputBuffer[i++] = float(mxm[0][0]); // CHECK: 4.0
    outputBuffer[i++] = float(mxm[0][1]); // CHECK: 9.0
    outputBuffer[i++] = float(mxm[1][0]); // CHECK: 4.0
    outputBuffer[i++] = float(mxm[1][1]); // CHECK: 8.0
}
