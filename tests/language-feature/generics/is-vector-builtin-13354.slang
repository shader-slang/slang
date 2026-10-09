//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// `__isVector<T>()` asks about `T` itself: true for `float3`, and false for `float` and `float3x3`,
// even though a matrix is made of the same `float` elements as a vector. `isVectorAfterSpecialization`
// keeps `T` generic, so its query folds only once `T` is known. `isFloatVectorOfCount` leaves only
// `N` generic; its query folds to true in the unspecialized body, since the type is a vector for
// every `N`. Each true answer sets one bit: 2, 8 and 64, for an expected total of 74.

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

bool isVectorAfterSpecialization<T>()
{
    return __isVector<T>();
}

bool isFloatVectorOfCount<let N : int>()
{
    return __isVector<vector<float, N>>();
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
    if (isVectorAfterSpecialization<float3>())
        bits |= 8;
    if (isVectorAfterSpecialization<float>())
        bits |= 16;
    if (isVectorAfterSpecialization<float3x3>())
        bits |= 32;
    if (isFloatVectorOfCount<4>())
        bits |= 64;
    outputBuffer[0] = bits;
}

// CHECK: type: int32_t
// CHECK-NEXT: 74
