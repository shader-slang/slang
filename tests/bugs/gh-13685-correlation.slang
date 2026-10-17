// Regression test for https://github.com/shader-slang/slang/issues/13685
//
// A merge of a never-written value and a defined one is folded to the defined value. When the
// never-written value is also read elsewhere, it is refined to the same defined value first, so the
// two reads still agree. Here `t` is `x` or the never-written `t`, and `before` reads the
// never-written `t`, so `t == before` must hold whenever `c` is false.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-41035
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-41035

//TEST_INPUT: set inputBuffer = ubuffer(data=[0 1 7], stride=4)
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

int mergeAndCompare(bool c, int x)
{
    int t;
    int before = t;
    if (c)
        t = x;
    return (c ? t == x : t == before) ? 1 : 0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = inputBuffer[2];
    // CHECK: type: int32_t
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[0] = mergeAndCompare(inputBuffer[0] != 0, x);
    outputBuffer[1] = mergeAndCompare(inputBuffer[1] != 0, x);
}
