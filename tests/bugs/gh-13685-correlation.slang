// Regression test for https://github.com/shader-slang/slang/issues/13685
//
// A merge of a never-written value and a defined one is folded to the defined value. When the
// never-written value is also read elsewhere, it is refined to the same defined value first, so the
// two reads still agree. Refinement only targets a value that cannot be poison, such as a constant.
// Here `x` becomes the constant 7 only when the back end inlines `mergeAndCompare`, so the front end
// keeps the merge and the back end refines the never-written `t` to 7. Since `before` reads that
// same `t`, `t == before` must hold whenever `c` is false.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-41035
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-41035 -xslang -g
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-41035
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-41035 -xslang -g

//TEST_INPUT: set inputBuffer = ubuffer(data=[0 1], stride=4)
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[ForceInline]
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
    // CHECK: type: int32_t
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[0] = mergeAndCompare(inputBuffer[0] != 0, 7);
    outputBuffer[1] = mergeAndCompare(inputBuffer[1] != 0, 7);
}
