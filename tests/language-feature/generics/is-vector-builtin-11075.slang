//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for the `__isVector<T>()` fold kept with shader-slang/slang#11075. The fold asks
// about the operand type itself, so it is true for `float3` and false for `float` and `float3x3`,
// which share its element type.

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

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
    outputBuffer[0] = bits;
}

// CHECK: type: int32_t
// CHECK-NEXT: 2
