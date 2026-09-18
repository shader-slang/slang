//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Regression test for shader-slang/slang#11075, F64 (double) element prefix.
//
// This is the double (F64) counterpart of generic-minmax-vector-11075.slang, split into its own
// file because Metal has no `double` type: a `double` vector cannot appear in the shared
// cross-target test that also targets `-mtl`. The backends that do support `double` (cpp/cuda/vk)
// exercise the F64 vector prelude row here.

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMaxC<T : IComparable>(T a, T b)
{
    return max(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    double2 ad = double2(1.5, 9.0), bd = double2(4.0, 2.5);
    double2 mnd = genMin(ad, bd);

    // Arity 3 exercises the double3 / Vector<double,3> helper through the IComparable path too.
    double3 ce = double3(7.5, 1.0, 4.0), de = double3(2.0, 6.0, 4.0);
    double3 mxe = genMaxC(ce, de);

    int i = 0;
    outputBuffer[i++] = float(mnd.x); // CHECK: 1.5
    outputBuffer[i++] = float(mnd.y); // CHECK: 2.5

    outputBuffer[i++] = float(mxe.x); // CHECK: 7.5
    outputBuffer[i++] = float(mxe.y); // CHECK: 6.0
    outputBuffer[i++] = float(mxe.z); // CHECK: 4.0
}
