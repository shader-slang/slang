//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// `__isVector<T>()` asks about `T` itself: true for `float3`, and false for `float` and `float3x3`,
// whose element type is also `float`. The generic cases fold after specialization.

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

bool isVectorOf<T>()
{
    return __isVector<T>();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int bits = 0;
    if (__isVector<float>())
        bits |= 1;
    if (__isVector<float3>())
        bits |= 2;
    if (__isVector<float3x3>())
        bits |= 4;
    if (isVectorOf<float3>())
        bits |= 8;
    if (isVectorOf<float>())
        bits |= 16;
    outputBuffer[0] = bits;
}

// CHECK: type: int32_t
// CHECK-NEXT: 10
