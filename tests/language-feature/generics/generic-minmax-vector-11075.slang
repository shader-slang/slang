//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type

// Regression test for shader-slang/slang#11075.
//
// A generic `min`/`max` (constrained by `IFloat`/`IComparable`) specialized on
// a vector must produce the same componentwise result on every backend. The
// shared CHECK buffer across cpu/cuda/vk/mtl is the cross-target parity gate.

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

T genMinC<T : IComparable>(T a, T b)
{
    return min(a, b);
}
T genMaxC<T : IComparable>(T a, T b)
{
    return max(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 a2 = float2(1, 5), b2 = float2(4, 2);
    float3 a3 = float3(1, 5, 3), b3 = float3(4, 2, 6);
    float4 a4 = float4(8, 5, 3, 1), b4 = float4(7, 2, 6, 4);

    float2 mn2 = genMin(a2, b2);
    float3 mx3 = genMax(a3, b3);
    float4 mn4 = genMin(a4, b4);
    float4 mx4 = genMax(a4, b4);

    // Exercise additional element prefixes so a defect in one of these prelude rows
    // (wrong CUDA base name / accessor / missing helper) fails here, not only in the field:
    // U32 (unsigned prefix) and U64 (uint64 prefix), through both paths. F64 (double) is not a
    // Metal type, so its prefix row is covered by generic-minmax-double-vector-11075.slang, which
    // omits `-mtl`.
    int3 ai = int3(9, 2, 5), bi = int3(3, 8, 5);
    int3 mniC = genMinC(ai, bi);
    int3 mxiC = genMaxC(ai, bi);

    uint4 au = uint4(9, 2, 5, 1), bu = uint4(3, 8, 5, 7);
    uint4 mxu = genMaxC(au, bu);

    uint64_t2 al = uint64_t2(9, 2), bl = uint64_t2(3, 8);
    uint64_t2 mnl = genMinC(al, bl);

    int i = 0;
    outputBuffer[i++] = mn2.x; // CHECK: 1.0
    outputBuffer[i++] = mn2.y; // CHECK: 2.0

    outputBuffer[i++] = mx3.x; // CHECK: 4.0
    outputBuffer[i++] = mx3.y; // CHECK: 5.0
    outputBuffer[i++] = mx3.z; // CHECK: 6.0

    outputBuffer[i++] = mn4.x; // CHECK: 7.0
    outputBuffer[i++] = mn4.y; // CHECK: 2.0
    outputBuffer[i++] = mn4.z; // CHECK: 3.0
    outputBuffer[i++] = mn4.w; // CHECK: 1.0

    outputBuffer[i++] = mx4.x; // CHECK: 8.0
    outputBuffer[i++] = mx4.y; // CHECK: 5.0
    outputBuffer[i++] = mx4.z; // CHECK: 6.0
    outputBuffer[i++] = mx4.w; // CHECK: 4.0

    outputBuffer[i++] = float(mniC.x); // CHECK: 3.0
    outputBuffer[i++] = float(mniC.y); // CHECK: 2.0
    outputBuffer[i++] = float(mniC.z); // CHECK: 5.0

    outputBuffer[i++] = float(mxiC.x); // CHECK: 9.0
    outputBuffer[i++] = float(mxiC.y); // CHECK: 8.0
    outputBuffer[i++] = float(mxiC.z); // CHECK: 5.0

    outputBuffer[i++] = float(mxu.x); // CHECK: 9.0
    outputBuffer[i++] = float(mxu.y); // CHECK: 8.0
    outputBuffer[i++] = float(mxu.z); // CHECK: 5.0
    outputBuffer[i++] = float(mxu.w); // CHECK: 7.0

    outputBuffer[i++] = float(mnl.x); // CHECK: 3.0
    outputBuffer[i++] = float(mnl.y); // CHECK: 2.0

    // vector → true; scalar and matrix → false.
    outputBuffer[i++] = __isVector<float3>() ? 1.0 : 0.0;   // CHECK: 1.0
    outputBuffer[i++] = __isVector<float>() ? 1.0 : 0.0;    // CHECK: 0.0
    outputBuffer[i++] = __isVector<float3x3>() ? 1.0 : 0.0; // CHECK: 0.0
}
